// Ikaros 3.0

#include "ikaros.h"

using namespace ikaros;

namespace ikaros
{
std::string  validate_identifier(std::string s)
{
    static std::string legal = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_1234567890";
    if(s.empty())
        throw exception("Identifier cannot be empty string");
    if('0' <= s[0] && s[0] <= '9')
        throw exception("Identifier cannot start with a number: "+s);
    for(auto c : s)
        if(legal.find(c) == std::string::npos)
            throw exception("Illegal character in identifier: "+s);
    return s;
}


    void Component::Bind(parameter & p, std::string name)
    {
        Kernel & k = kernel();
        std::string pname = name_+"."+name;
        if(k.parameters.count(pname))
            p = kernel().parameters[pname];
        else
            throw exception("Cannot bind to \""+name+"\"");
    };


    void Component::Bind(matrix & m, std::string n) // Bind input, output or parameter
    {
        std::string name = name_+"."+n;
        try
        {
            Kernel & k = kernel();
            if(k.inputs.count(name))
                m = k.inputs[name];
            else if(k.outputs.count(name))
                m = k.outputs[name];
            else if(k.parameters.count(name))
                m = (matrix &)(k.parameters[name]);
            else if(k.parameters.count(name))
                throw exception("Cannot bind to attribute \""+name+"\". Define it as a parameter!");
            else
                throw exception("Input or output named \""+name+"\" does not exist");
        }
        catch(exception e)
        {
            throw exception("Bind:\""+name+"\" failed. "+e.message);
        }
    }

    void Component::AddInput(dictionary parameters)
    {
        std::string input_name = std::string(info_["name"])+"."+validate_identifier(parameters["name"]);
        kernel().AddInput(input_name, parameters);
    }

    void Component::AddOutput(dictionary parameters)
    {
        std::string output_name = std::string(info_["name"])+"."+validate_identifier(parameters["name"]);
        kernel().AddOutput(output_name, parameters);
      };

    void Component::AddParameter(dictionary parameters)
    {
        std::string pn = parameters["name"];
        if(pn=="name")
            return;
        if(pn=="class")
            return;            
        std::string parameter_name = std::string(info_["name"])+"."+validate_identifier(pn);
        kernel().AddParameter(parameter_name, parameters);
    }


    void Component::SetParameter(std::string name, std::string value)
    {
        if(name=="name")
            return;
        if(name=="class")
            return;  
        std::string parameter_name = std::string(info_["name"])+"."+validate_identifier(name);
        kernel().SetParameter(parameter_name, value);
      }


    bool Component::LookupParameter(parameter & p, const std::string & name)
    {
        Kernel & k = kernel();
        if(k.parameters.count(name_+"."+name))
        {
            p = k.parameters[name_+"."+name];
            return true;
        }
        else if(parent_)
            return parent_->LookupParameter(p, name);
        else
            return false;
    }


/*
std::string Component::Lookup(const std::string & name) const // FIXME: Should this funciton throw????
{
    std::string component_name = name_;
    while (true)
    {
        auto it = kernel().parameters.find(component_name + "." + name);
        if (it != kernel().parameters.end() && it->second.is_set)
            return it->second;
        int p = component_name.rfind('.');
        if(p == std::string::npos)
            return std::string();
        component_name = component_name.substr(0, p);
    }
}


// Recursive

std::string Component::Lookup(const std::string & name) const
{
    auto it = kernel().parameters.find(name_ + "." + name);
    if (it != kernel().parameters.end() && it->second.is_set)
        return it->second;
    else if(parent)
        return parent->Lookup(name)
    else 
        return "";
    }
}
*/


    Component * Component::GetComponent(const std::string & s) // Get component; sensitive to variables and indirection
    {
        std::string path = SubstituteVariables(s);
        try
        {        
            std::string aaa = rhead(name_,'.');
            std::string xxx = rtail(rhead(name_,'.'),'.');

            if(path.empty()) // this
                return this;
            if(path[0]=='.') // global
                return kernel().components.at(path.substr(1));
            if(kernel().components.count(name_+"."+head(path,'.'))) // inside
                return kernel().components[name_+"."+head(path,'.')]->GetComponent(tail(path,'.'));
            if(rtail(rhead(name_,'.'),'.') == head(path,'.') && parent_) // parent
                return parent_->GetComponent(tail(path,'.'));
            throw exception("Component does not exist.");
        }
        catch(const std::exception& e)
        {
            throw exception("Component \""+path+"\" does not exist.");
        }
    }



  Component::Component():
    parent_(nullptr),
    info_(kernel().current_component_info),
    name_(info_["name"])
{
    //std::cout << "COMPONENT CREATOR: " << name_ << std::endl;

    for(auto p: info_["parameters"])
        AddParameter(p["attributes"]);

/*
    for(auto p: dictionary(info_["attributes"]))
        SetParameter(p.first, p.second);
*/
    for(auto & input: info_["inputs"])
        AddInput(input["attributes"]);

    for(auto & output: info_["outputs"])
        AddOutput(output["attributes"]);

    // Set parent

    auto p = name_.rfind('.');
    if(p != std::string::npos)
        parent_ = kernel().components.at(name_.substr(0, p));
}

  Module::Module()
    {
    // Copy module attributes into info structure taht already contains class attributes
        for(auto p: dictionary(kernel().current_module_info["attributes"]))
            info_["attributes"][p.first] = p.second;

    }


// The following lines will create the kernel the first time it is accessed by one of the components

    Kernel& kernel()
    {
        static Kernel * kernelInstance = new Kernel();
        return *kernelInstance;
    }
}; // namespace ikaros


bool
Component::InputsReady(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections) // FIXME: Handle optional inputs
{
    Kernel& k = kernel();

    std::cout << "CHECKING INPUT  " << name_ << "." << d["attributes"]["name"] << std::endl;
    std::string n = d["attributes"]["name"];
    // Check that all connections to this input has a size
    for(auto & c : ingoing_connections[name_+'.'+n])
    {
        std::cout << "\tCHECKING CONNECTION FROM " << c->source << std::endl;
        int r = k.outputs.at(c->source).rank();
        std::cout << "\tRANK=" << r << std::endl;
        if(r==0)
            return false;
    }
    return true;
}



void Component::SetInputSize_Copy(const std::string & name, std::map<std::string,std::vector<Connection *>> & ingoing_connections)
{
    if(ingoing_connections[name].size() != 1)
        throw exception("Input with type copy only accepts a single ingoing connection.");
    std::vector<int> shape = kernel().outputs.at(ingoing_connections[name][0]->source).shape();
    kernel().inputs[name].realloc(shape);

    // TODO: SET CONNECTION RANGES *****************
}



void Component::SetInputSize_Flat(const std::string & name, std::map<std::string,std::vector<Connection *>> & ingoing_connections)
{
    int flattened_input_size = 0;
    for(auto & c : ingoing_connections[name])
        flattened_input_size += kernel().outputs.at(ingoing_connections[name][0]->source).size();
    kernel().inputs[name].realloc(flattened_input_size);

    // TODO: SET CONNECTION RANGES
}


void Component::SetInputSize_Stack(const std::string & name, std::map<std::string,std::vector<Connection *>> & ingoing_connections)
{
}


void Component::SetInputSize_Index(const std::string & name, std::map<std::string,std::vector<Connection *>> & ingoing_connections)
{
}


void Component::SetInputSize(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections)
{
        Kernel& k = kernel();

        std::cout << ">>>***** SETTING INPUT SIZE  " << name_ << "." << d["attributes"]["name"] << " as " << d["attributes"]["type"] <<  std::endl;
        std::string input_name = name_ + "." + std::string(d["attributes"]["name"]);
        std::string type = d["attributes"]["type"];

        if(type == "copy")
            SetInputSize_Copy(input_name, ingoing_connections);
        else if(type == "flat")
            SetInputSize_Flat(input_name, ingoing_connections);
        else if(type == "stack")
            SetInputSize_Stack(input_name, ingoing_connections);
        else if(type == "index")
            SetInputSize_Index(input_name, ingoing_connections);
        else // keep_or_flatten [keep if one - flatten if several]
            {
                // Call one of two functions keep/flatten
            }

/*
    Kernel& k = kernel();

    std::cout << "CHECKING INPUT  " << name_ << "." << d["attributes"]["name"] << std::endl;
    std::string n = d["attributes"]["name"];
    // Check that all connections to this input has a size
    for(auto & c : ingoing_connections[name_+'.'+n])
    {
        std::cout << "\tCHECKING CONNECTION FROM " << c->source << std::endl;
        int r = k.outputs.at(c->source).rank();
        std::cout << "\tRANK=" << r << std::endl;
        if(r==0)
            return false;
    }
*/
    return;
}


// Attempt to set sizes for a single component

int
Component::SetSizes(std::map<std::string,std::vector<Connection *>> & ingoing_connections) // FIXME: add more exception handling
{
    Kernel& k = kernel();

    // Set input sizes (if possible)

    for(auto & d : info_["inputs"])
        if(InputsReady(dictionary(d), ingoing_connections))
            SetInputSize(dictionary(d), ingoing_connections);

    // Set output sizes // FIXME: Move to separate function

    int outputs_with_size = 0;
    for(auto & d : info_["outputs"])
    {
        std::string n = d["attributes"]["name"];
        std::cout << "SET SIZES " << name_ << "." << n << std::endl;
        std::string s = d["attributes"]["size"];
        std::vector<int> shape = EvaluateSize(s);
        matrix o;
        Bind(o, n); // FIXME: Get directly?
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
