// Ikaros 3.0

#ifndef IKAROS
#define IKAROS

#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <filesystem>

#define INSTALL_CLASS(class_name)  static InitClass init_##class_name(#class_name, []() { return new class_name(); });

#include "Kernel/utilities.h"
#include "Kernel/dictionary.h"
#include "Kernel/options.h"
#include "Kernel/range.h"
#include "Kernel/expression.h"
#include "Kernel/matrix.h"
#include "Kernel/socket.h"

namespace ikaros {

std::string  validate_identifier(std::string s);

struct exception : public std::exception
{
    std::string message;
    exception(std::string msg): message(msg) {};

   const char * what () const throw () { return message.c_str(); }
};




class Kernel;

Kernel& kernel();

enum parameter_type { no_type, int_type, float_type, string_type, matrix_type, list_type };

class parameter // FIXME: check all types everywhere
{
private:
    parameter_type  type;
    bool            live;
    std::shared_ptr<int>            int_value;
    std::shared_ptr<float>          float_value;
    std::shared_ptr<matrix>         matrix_value;
    std::shared_ptr<std::string>    string_value;
    std::vector<std::string>        list_alternatives;
public:
    parameter(): type(no_type) {}

    parameter(std::string type_string, std::string default_value)
    {
        if(type_string == "int")
        {
            type = int_type;
            int_value = std::make_shared<int>(std::stoi(default_value));
        }
        else if(type_string == "float")
        {
            type = float_type;
            float_value = std::make_shared<float>(std::stof(default_value));
        }
        else if(type_string == "string")
        {
            type = string_type;
            string_value = std::make_shared<std::string>(default_value);
        }
        else if(type_string == "matrix")
        {
            type = matrix_type;
            matrix_value = std::make_shared<matrix>(default_value);
        }
        else
            throw exception("unkown parameter type");
    }


    int operator=(int v)
    {
        switch(type)
        {
            case int_type: if(int_value) *int_value = v; break;
            case float_type: if(float_value) *float_value = float(v); break;
            case string_type: if(string_value) *string_value = std::to_string(v); break;
            default: break;
        }
        return v;
    }

    float operator=(float v)
    {
        switch(type)
        {
            case int_type: if(int_value) *int_value = int(v); break;
            case float_type: if(float_value) *float_value = v; break;
            case string_type: if(string_value) *string_value = std::to_string(v); break;
            default: break;
        }
        return v;
    }

    float operator=(double v) { return operator=(float(v)); };


    std::string operator=(std::string v) // FIXME: Handle exceptions higher up ******* FOR STRING CHECK CONVERSION POSSIBLY
    {
        try
        {
            switch(type)
            {
                case no_type: type=string_type;  string_value = std::make_shared<std::string>(v); break;          // FIXME: ADD THIS TO THE OTHER TYPES AS WELL
                case int_type: if(int_value) *int_value = std::stoi(v); break;
                case float_type: if(float_value) *float_value = std::stof(v); break;
                case string_type: if(string_value) *string_value = v; break;
                case matrix_type: if(matrix_value) *matrix_value = v; break; // ERRR ****
                default: break;
            }
            return v;
        }
        catch(const std::exception& e)
        {
            throw exception("String \""+v+"\" cannot be converted to float");
        }
        
    }


    operator matrix & ()
    {
        if(type==matrix_type && matrix_value) 
            return *matrix_value;

        throw exception("not a matrix parameter");
    }

    operator std::string()
    {
        switch(type)
        {
            case no_type: return "uninitialized_parameter"; // FIXME: throw
            case int_type: if(int_value) return std::to_string(*int_value);
            case float_type: if(float_value) return std::to_string(*float_value);
            case string_type: if(string_value) return *string_value;
            case matrix_type: return "MATRIX-FIX-ME";
            default: return "type-conversion-error";
        }
        return "type-conversion-error";
    }


    friend std::ostream& operator<<(std::ostream& os, parameter p)
    {
        switch(p.type)
        {
            case int_type:      os << *p.int_value; break;
            case float_type:    os <<  *p.float_value; break;
            case string_type:   os <<  *p.string_value; break;
            default:            os << "unkown-parameter-type"; break;
        }

        return os;
    }
};

class Module
{
public:
    std::string     name_;
    dictionary      info_;

    Module();

    virtual ~Module() {};

    void print()
    {
        std::cout << "MODULE: " << info_["name"]  << '\n';
    }

    void AddInput(dictionary parameters);
    void AddOutput(dictionary parameters);
    void AddParameter(dictionary parameters);
    void SetParameter(std::string name, std::string value);

    void    Bind(parameter & p, std::string n);   // Bind to parameter in global parameter table
    void    Bind(matrix & m, std::string n); // Bind to input or output in global tables, or matrix parameter

    virtual void Tick() {}
    virtual void Init() {}

    std::string Lookup(std::string name);

    int EvaluateIntExpression(std::string & s)
    {
        expression e = expression(s);
        std::map<std::string, std::string> vars;
        for(auto v : e.variables())
            vars[v] = Lookup(v);
        return expression(s).evaluate(vars);
    }

    std::vector<int> EvaluateSizeList(std::string & s) // return list of size from size list in string
    {
        std::vector<int> shape;
        for(std::string e : split(s, ","))
        shape.push_back(EvaluateIntExpression(e));
        return shape;
//        throw exception("Could not parse size list string");
    }

    std::vector<int> EvaluateSize(std::string & s) // Evaluate size/shape string
    {
         if(ends_with(s, ".size")) // special case: use shape function on input
        {
            auto & x = rsplit(s, ".", 1); // FIXME: rhead ???
            matrix m;
            Bind(m, x.at(0));
            return m.shape(); 
        }

        return EvaluateSizeList(s); // default case: parse size list
    }



    virtual int SetSizes() // FIXME: add more exception handling
    {
        int outputs_with_size = 0;

        for(auto & d : info_["class"]["outputs"])
        {
            std::string n = d["attributes"]["name"];
            std::cout << "SET SIZES " << name_ << "." << n << std::endl;
            std::string s = d["attributes"]["size"];
            std::vector<int> shape = EvaluateSize(s);
            matrix o;
            Bind(o, n);
            if(o.rank() != 0)
                outputs_with_size++; // Count number of inputs that are set
            
            o.realloc(shape);
            outputs_with_size++;
        }

        if(outputs_with_size == info_["class"]["outputs"].size())
            return 0; // all outputs have been set
        else
            return 0; // needs to be called again - FIXME: Report progress later on
    }
};

typedef std::function<Module *()> ModuleCreator;

class Connection
{
    public:
    std::string source;
    range       source_range;
    std::string target;
    range       target_range;

    Connection(std::string s, std::string t)
    {
        source = head(s, '[');
        source_range = range(tail(s, '['));
        target = head(t, '[');
        target_range = range(tail(t, '['));
    };


    void
    Print()
    {
        std::cout << "CONNECTION: " << source  << " => " << target << '\n'; 
    }
};


class Class
{
public:
    ModuleCreator   module_creator;
    std::string     name;
    std::string     path;
    std::map<std::string, std::string>  parameters;

    Class() {};
    Class(std::string n, std::string p) : module_creator(nullptr), name(n), path(p)
    {
    };

    void print()
    {
        std::cout << name << ": " << path  << '\n';
    }
};


class Kernel
{
public:
    std::map<std::string, Class>        classes;
    std::map<std::string, Module *>     modules;
    std::vector<Connection>             connections;
    std::map<std::string, matrix>       inputs;         // Use IO-structure later: or only matrix?
    std::map<std::string, matrix>       outputs;        // Use IO-structure later: Output
    std::map<std::string, parameter>    parameters;

    dictionary                      current_module_info;


    void ScanClasses(std::string path)
    {
        int i=0;
        for(auto& p: std::filesystem::recursive_directory_iterator(path))
            if(p.path().extension()==".ikc")
            {
                std::string name = p.path().stem();
                classes[name] = Class(name, p.path());
            }
    }


    void ListClasses()
    {
        std::cout << "\nClasses:" << std::endl;
        for(auto & c : classes)
            c.second.print();
    }

    /*
        EXPLAIN ALGORITHM HERE 
    */

    void CalculateSizes()
    {
        std::map<std::string, Module *> init_modules = modules;     // Copy the table of all modules

        // Propagate output size to inputs as long as at least one module sets one of it output sizes

        int iteration_counter = 0;
        bool progress = true;
        while(progress)
        {
     
            //progress = false; // FIXME: turn on again when progress reporting is implemented

            // STEP 1: Calculate input sizes

            std::set<std::string> waiting; // Put inputs that cannot be set on the waiting list, remove when set
            for(auto & c : connections)
            {
                if (waiting.count(c.target)) // skip waiting inputs
                    continue;

                if(outputs[c.source].rank() == 0)
                {
                    waiting.insert(c.target);
                    inputs[c.target].realloc(); // clear input size if set
                    continue;
                }

                inputs[c.target].realloc(outputs[c.source].shape());   // FIXME: Only works for 1-1 connection: type = copy!!!, update target size for different alternatives
            }

            // STEP 2: Try to set output sizes and remove from list if complete

            auto it = init_modules.begin();
            while( it!=init_modules.end())
                if(it->second->SetSizes() == 1) // 1 = completed
                {
                    it = init_modules.erase(it);
                    //progress = true;
                }
                else
                    it++;
            // If we reached the maximum number of iterations, exit.

            iteration_counter++;
            if(iteration_counter > connections.size())
                break;
        }

        // List remaining modules

        if(init_modules.size() > 0)
        {
            std::cout << "--- Modules with unset input sizes ---" << std::endl;
            for(auto & m : init_modules)
                std::cout << m.first << std::endl;
            std::cout << "--------------------------------------" << std::endl;
        }
    }



    void ListModules()
    {
        std::cout << "\nModules:" << std::endl;
        for(auto & m : modules)
            m.second->print();
    }


    void ListConnections()
    {
        std::cout << "\nConnections:" << std::endl;
        for(auto & c : connections)
            c.Print();
    }


    void ListInputs()
    {
        std::cout << "\nInputs:" << std::endl;
        for(auto & i : inputs)
            std::cout << i.first <<  ": (rank:" << i.second.rank() << ", shape: " << i.second.shape() << ")" << std::endl;
    }


   void ListOutputs()
    {
        std::cout << "\nOutputs:" << std::endl;
        for(auto & o : outputs)
            std::cout << o.first << ": (rank:" << o.second.rank() << ", shape: " << o.second.shape() << ")" << std::endl;
    }


   void ListParameters()
    {
        std::cout << "\nParameters:" << std::endl;
        for(auto & p : parameters)
            std::cout << p.first << ": " << p.second << std::endl;
    }


    Kernel()
    {
            ScanClasses("Source/Modules");
    }

    // Functions for creating the network

    void AddGroup(std::string name)
    {
        //std::cout << "ADD GROUP: " << name  << std::endl;
    }

    void AddInput(std::string name, dictionary parameters=dictionary())
    {
        inputs[name] = matrix();
    }

    void AddOutput(std::string name, dictionary parameters=dictionary())
    {
        outputs[name] = matrix();
      }

    void AddParameter(std::string name, dictionary params=dictionary())
    {
        //std::cout << "ADDING PARAMETER: " << name << ": " <<  std::endl;
        std::string type_string = params["type"];
        std::string default_value = params["default"];
        std::string options = params["options"];
        parameter p(type_string, default_value);
        parameters[name] =  p;
    }

    void SetParameter(std::string name, std::string value)
    {
        std::cout << "SETTING PARAMETER: " << name << ": " << value << std::endl;
        if(parameters.count(name))
            parameters[name] = value;
        else
           throw exception("ATTEMPTING TO SET NON-EXISTING PARAMETER: "+name);
    }

    void AddModule(std::string name, std::string classname, dictionary parameters=dictionary())
    {
            if(modules.count(name)> 0)
                throw exception("Module with this name already exists");

            current_module_info["class"] = dictionary(classes[classname].path, true);
            current_module_info["parameters"] = parameters;
            current_module_info["name"] = name;
            modules[name] = classes[classname].module_creator(); // throws bad function call if not defined
    }


    void AddConnection(std::string souce, std::string target) // TODO: Add ranges later
    {
        //std::cout << "ADD CONNECTION: " << souce << "=>" << target <<  std::endl;
        connections.push_back(Connection(souce, target));
    }

    // Functions for reading from file

    void ParseGroupFromXML(XMLElement * xml, std::string path="")
    {
        std::string name = validate_identifier((*xml)["name"]);
        if(!path.empty())
            name = path+"."+name;

        //std::cout << "PARSE GROUP: " << name << std::endl;
        AddGroup(name);


        for (XMLElement * xml_node = xml->GetContentElement(); xml_node != nullptr; xml_node = xml_node->GetNextElement())
        {
            if(xml_node->IsElement("module"))
            {
                AddModule(name+"."+validate_identifier((*xml_node)["name"]), (*xml_node)["class"], dictionary(xml_node));
            }
            else if(xml_node->IsElement("group"))
            {
                ParseGroupFromXML(xml_node, name);
            }
            else if(xml_node->IsElement("connection"))
            {
                AddConnection(name+"."+(*xml_node)["source"], name+"."+(*xml_node)["target"]);
            }
        }
    }
 


    void AllocateInputs()
    {
        // Allocate memory for all inputs and delay lines 
    }


    void InitModules()
    {
        // Call Init for all modules (after CalcalateSizes and Allocate)
        for(auto & m : modules)
            m.second->Init(); 
    }

    void LoadFiles(std::vector<std::string> files)
    {
            for(auto & filename: files)
            {
                //std::cout << "READING FILE: " << filename << std::endl;
                XMLDocument * xmlDoc = new XMLDocument(filename.c_str());
                if(xmlDoc->xml == nullptr)
                    std::cout << "File not found" << std::endl;

                if(xmlDoc->xml->name != std::string("group"))
                    exit(1);

                ParseGroupFromXML(xmlDoc->xml); // TODO: check that main element is of type 'group'
            }
    }


    void SortNetwork()
    {
        // Resolve paths
        // Sort Modules and Connections => Thread Groups and Partially Ordered Events (Tick & Propagate)
    }


    void Propagate()
    {
         for(auto & c : connections)
        {
            //std::cout << "Propagating: " << c.source << " => " << c.target << std::endl;
            inputs[c.target].copy(outputs[c.source]);
        }
    }


    void Tick()
    {
        for(auto & m : modules)
            m.second->Tick();
    }
};

// Initialization class

class InitClass
{
public:
    InitClass(const char * name, ModuleCreator mc)
    {
        // std::cout << "INSTALLING: " << name << std::endl;
        kernel().classes.at(name).module_creator = mc;
    }
};

}; // namespace ikaros
#endif

