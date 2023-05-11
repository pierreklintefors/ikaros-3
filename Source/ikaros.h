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

class Component;
class Module;
class Kernel;

Kernel& kernel();

enum parameter_type { no_type=0, int_type, float_type, string_type, matrix_type, options_type };
static std::vector<std::string> parameter_strings = {"none", "int", "float", "string", "matrix", "options"};

class parameter // FIXME: check all types everywhere
{
private:
    parameter_type  type;
    bool            live;
    std::shared_ptr<int>            int_value;
    std::shared_ptr<float>          float_value;
    std::shared_ptr<matrix>         matrix_value;
    std::shared_ptr<std::string>    string_value;
    std::shared_ptr<std::string>    default_value;
    std::vector<std::string>        options;
public:
    parameter(): type(no_type) {}

    parameter(std::string type_string, std::string default_val, std::string options_string)
    {
        default_value = std::make_shared<std::string>(default_val);
        auto type_index = std::find(parameter_strings.begin(), parameter_strings.end(), type_string);
        if(type_index == parameter_strings.end())
            throw exception("unkown parameter type: "+type_string);
        type = parameter_type(std::distance(parameter_strings.begin(), type_index));

        return;
/*
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
        else if(type_string == "options")
        {
            type = options_type;
            options = split(options_string, ",");
            auto ix = find(options.begin(), options.end(), default_value);
            if (ix==options.end())
                throw exception("option not defined");
            int_value = std::make_shared<int>(ix - options.begin());
        }
        else
            throw exception("unkown parameter type");
*/
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

    float operator=(double v) { return operator=(float(v)); }; // FIXME: ADD ALL TYPE HERE!!! *** STEP 1 ***


    std::string operator=(std::string v) // FIXME: Handle exceptions higher up *******  // FIXME: ADD ALL TYPE HERE!!! *** STEP 1 ***
    {
        switch(type)
        {
            case no_type: type=string_type;  string_value = std::make_shared<std::string>(v); break;          // FIXME: ADD THIS TO THE OTHER TYPES AS WELL
        
            case int_type: 
                if(int_value) 
                    *int_value = std::stoi(v); 
                else
                    int_value = std::make_shared<int>(std::stoi(v));
                break;
            case options_type:
            {
                auto ix = find(options.begin(), options.end(), v);
                if (ix==options.end())
                    throw exception("option \""+v+"\" not defined"); // FIXME: Should be able to recover if the value came from WEBUI
                int_value = std::make_shared<int>(ix - options.begin());
            }
            break;

            case float_type: 
                if(float_value) 
                    *float_value = std::stof(v);
                else
                    float_value = std::make_shared<float>(std::stof(v));
                break;


            case string_type: if(string_value) *string_value = v; break;
            case matrix_type: 
                if(matrix_value) 
                    matrix_value = std::make_shared<matrix>(matrix(v)); // FIXME: Copy???
                else
                    matrix_value = std::make_shared<matrix>(matrix(v));
                break;
            default: break;
        }
        return v;
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
            // FIXME: get options string
            case float_type: if(float_value) return std::to_string(*float_value);
            case string_type: if(string_value) return *string_value;
            case matrix_type: return "MATRIX-FIX-ME"; //FIXME: Get matrix string - add to matrix class to also be used by print
            default: return "type-conversion-error";
        }
        return "type-conversion-error";
    }

   operator int()
    {
        switch(type)
        {
            case no_type: throw exception("uninitialized_parameter");
            case int_type: if(int_value) return *int_value;
            case options_type: if(int_value) return *int_value;
            case float_type: if(float_value) return *float_value;
            case string_type: if(string_value) return stof(*string_value); // FIXME: Check that it is a number
            case matrix_type: throw exception("Could not convert matrix to float"); // FIXME check 1x1 matrix
            default: throw exception("type-conversion-error");  // FIXME: ADD NAME
        }
        throw exception("type-conversion-error");
    }

   operator float()
    {
        switch(type)
        {
            case no_type: throw exception("uninitialized_parameter");
            case int_type: if(int_value) return *int_value;
            case options_type: if(int_value) return *int_value;
            case float_type: if(float_value) return *float_value;
            case string_type: if(string_value) return stof(*string_value); // FIXME: Check that it is a number
            case matrix_type: throw exception("Could not convert matrix to float"); // FIXME check 1x1 matrix
            default: throw exception("type-conversion-error");  // FIXME: ADD NAME
        }
        throw exception("type-conversion-error");
    }

    friend std::ostream& operator<<(std::ostream& os, parameter p)
    {
        switch(p.type)
        {
            case int_type:      if(p.int_value) os << *p.int_value; break;
            case options_type:  if(p.int_value) os << *p.int_value; break;
            case float_type:    if(p.float_value) os <<  *p.float_value; break;
            case string_type:   if(p.string_value) os <<  *p.string_value; break;
            case matrix_type:   if(p.matrix_value) os <<  *p.matrix_value; break;
            default:            os << "unkown-parameter-type"; break;
        }

        return os;
    }
};

class Component
{
public:
    std::string     name_;
    dictionary      info_;

    Component();

    virtual ~Component() {};

    void print()
    {
        std::cout << "Component: " << info_["name"]  << '\n';
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

        for(auto & d : info_["outputs"])
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


class Group : public Component
{
    
};


class Module : public Component
{
public:
    Module();
    ~Module() {}    
};

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
    std::map<std::string, Component *>  components;
    std::vector<Connection>             connections;
    std::map<std::string, matrix>       inputs;         // Use IO-structure later: or only matrix?
    std::map<std::string, matrix>       outputs;        // Use IO-structure later: Output
    std::map<std::string, parameter>    parameters;

    dictionary                          current_component_info; // From class or group
    dictionary                          current_module_info;

    long tick;
    float tick_duration; // actual or simulated time for each tick

    long GetTick() { return tick; }
    double GetTickDuration() { return tick_duration; } // Time for each tick in seconds (s)
    double GetTime() { return GetTickTime(); }   // Time since start (in real time or simulated (tick) time depnding on mode)
    double GetRealTime() { return GetTickTime(); }    // Time since start in real time  (s) - is equal to GetTickTime when not in real-time mode // FIXME: Handle real-time mode
    double GetTickTime() { return float(GetTick())*GetTickDuration(); }    // Nominal time since start for current tick (s)
    double GetLag() { return GetTickTime() - GetRealTime(); }


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
        std::map<std::string, Component *> init_components = components;     // Copy the table of all components

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

            auto it = init_components.begin();
            while( it!=init_components.end())
                if(it->second->SetSizes() == 1) // 1 = completed
                {
                    it = init_components.erase(it);
                    //progress = true;
                }
                else
                    it++;
            // If we reached the maximum number of iterations, exit.

            iteration_counter++;
            if(iteration_counter > connections.size())
                break;
        }

        // List remaining components

        if(init_components.size() > 0)
        {
            std::cout << "--- Components with unset input sizes ---" << std::endl;
            for(auto & m : init_components)
                std::cout << m.first << std::endl;
            std::cout << "--------------------------------------" << std::endl;
        }
    }



    void ListComponents()
    {
        std::cout << "\nComponents:" << std::endl;
        for(auto & m : components)
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
            std::cout << "\t" << i.first <<  ": (rank:" << i.second.rank() << ", shape: " << i.second.shape() << ")" << std::endl;
    }


   void ListOutputs()
    {
        std::cout << "\nOutputs:" << std::endl;
        for(auto & o : outputs)
            std::cout  << "\t" << o.first << ": (rank:" << o.second.rank() << ", shape: " << o.second.shape() << ")" << std::endl;
    }


   void ListParameters()
    {
        std::cout << "\nParameters:" << std::endl;
        for(auto & p : parameters)
            std::cout  << "\t" << p.first << ": " << p.second << std::endl;
    }


    Kernel()
    {
            tick = 0;
            tick_duration = 0.01; // 10 ms

            ScanClasses("Source/Modules");
    }

    // Functions for creating the network

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
        std::cout << "ADDING PARAMETER: " << name << ": " <<  std::endl;
        std::string type_string = params["type"];
        std::string default_value = params["default"];
        std::string options = params["options"];
        parameter p(type_string, default_value, options);
        parameters[name] =  p;
    }

    void SetParameter(std::string name, std::string value)
    {
        std::cout << "SETTING PARAMETER: " << name << ": " << value << std::endl;
        if(parameters.count(name))
            parameters[name] = value;
        else
            parameters[name] = value; // TEST
           // throw exception("ATTEMPTING TO SET NON-EXISTING PARAMETER: "+name);
    }

    void AddGroup(std::string name, dictionary info=dictionary()) // WILL BE USED FOR ADD-MODULE AS WELL LATER ON ***
    {
        if(components.count(name)> 0)
            throw exception("Module or group with this name already exists");

        current_component_info = info;
        current_component_info["name"] = name;
        components[name] = new Group(); // Implicit argument passing as for components
    }

    void AddModule(std::string name, dictionary info=dictionary())
    {
        if(components.count(name)> 0)
            throw exception("Module or group with this name already exists");
        std::string classname = info["attributes"]["class"];
        current_component_info = dictionary(classes[classname].path, true);
        current_component_info["name"] = name;
        current_module_info = info;
        current_module_info["name"] = name;
        components[name] = classes[classname].module_creator(); // throws bad function call if not defined
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
        AddGroup(name, dictionary(xml, true));

        for (XMLElement * xml_node = xml->GetContentElement(); xml_node != nullptr; xml_node = xml_node->GetNextElement())
        {
            if(xml_node->IsElement("module"))
            {
                AddModule(name+"."+validate_identifier((*xml_node)["name"]), dictionary(xml_node)); // (*xml_node)["class"], 
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


    void InitComponents()
    {
        // Call Init for all modules (after CalcalateSizes and Allocate)
        for(auto & m : components)
            m.second->Init(); 
    }

    void LoadFiles(std::vector<std::string> files, options & opts)
    {
            for(auto & filename: files)
            {
                //std::cout << "READING FILE: " << filename << std::endl;
                XMLDocument * xmlDoc = new XMLDocument(filename.c_str());
                if(xmlDoc->xml == nullptr)
                    std::cout << "File not found" << std::endl;

                if(xmlDoc->xml->name != std::string("group"))
                    exit(1);

                // Add command line options to top element of XML before parsing // FIXME: Clean up

                for(auto & x : opts.d)
                    if(!xmlDoc->xml->GetAttribute(x.first.c_str())) 
                        ((XMLElement *)(xmlDoc->xml))->attributes = new XMLAttribute(create_string(x.first.c_str()), create_string(x.second.c_str()),0,((XMLElement *)(xmlDoc->xml))->attributes);          


                ParseGroupFromXML(xmlDoc->xml);
            }
    }


    void SortNetwork()
    {
        // Resolve paths
        // Sort Components and Connections => Thread Groups and Partially Ordered Events (Tick & Propagate)
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
        for(auto & m : components)
            m.second->Tick();
        tick++;
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

