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


    void Component::Bind(parameter & p, std::string n) // FIXME: Do the fancy recurrency here as well ****
    {
        p = kernel().parameters.at(name_+"."+n);
    };


    void Component::Bind(matrix & m, std::string n) // Bind input, output or parameter
    {
        Kernel & k = kernel();
        std::string name = name_+"."+n;
        if(k.inputs.count(name))
            m = k.inputs[name];
        else if(k.outputs.count(name))
            m = k.outputs[name];
        else if(k.parameters.count(name))
            m = (matrix &)(k.parameters[name]); // FIXME: Check if matrix parameter // FIXME: Do the fancy recurrency here as well **** maybe
        else
            throw exception("Input or output named "+name+" does not exist");
    }

    void Component::AddInput(dictionary parameters)
    {
        std::string input_name = std::string(info_["name"])+"."+validate_identifier(parameters["name"]);
        kernel().AddInput(input_name, parameters);
    };

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
      };

    void Component::SetParameter(std::string name, std::string value)
    {
        if(name=="name")
            return;
        if(name=="class")
            return;  
        std::string parameter_name = std::string(info_["name"])+"."+validate_identifier(name);
        kernel().SetParameter(parameter_name, value);
      };

    std::string Component::Lookup(std::string name) // Call kernel.lookup() later ************
    {
        name = name_+"."+name;
        if(!kernel().parameters.count(name))
            throw exception("Variable \""+name+"\" does not exist");

        return kernel().parameters[name];
    }

  Component::Component()
    {
        info_ = kernel().current_component_info;
        name_ = info_["name"];

        std::cout << "COMPONENT CREATOR: " << name_ << std::endl;


        for(auto p: info_["parameters"])
            AddParameter(p["attributes"]);

        for(auto p: dictionary(info_["attributes"]))
            SetParameter(p.first, p.second);

        for(auto & input: info_["inputs"])
            AddInput(input["attributes"]);

        for(auto & output: info_["outputs"])
            AddOutput(output["attributes"]);

    //    for(auto p: dictionary(info_["class"]["attributes"])) // Add ad-hoc attributes/parameters // TODO: remove class
    //        SetParameter(p.first, p.second);
    //    for(auto p: info_["class"]["parameters"]) // TODO: remove class
    //         AddParameter(p["attributes"]);
    
    }

  Module::Module()
    {
        auto & minfo = kernel().current_module_info;
        for(auto p: dictionary(minfo["attributes"]))
            SetParameter(p.first, p.second);
    }


// The following lines will create the kernel the first time it is accessed by one of the components

    Kernel& kernel()
    {
        static Kernel * kernelInstance = new Kernel();
        return *kernelInstance;
    }
};


