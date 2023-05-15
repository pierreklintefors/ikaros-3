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
/*
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

  Component::Component(): 
    parent(nullptr),
    info_(kernel().current_component_info),
    name_(info_["name"])
{
    std::cout << "COMPONENT CREATOR: " << name_ << std::endl;

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
        parent = kernel().components.at(name_.substr(0, p));
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


