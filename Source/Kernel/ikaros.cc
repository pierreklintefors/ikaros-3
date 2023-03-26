// Ikaros 3.0


#include "ikaros.h"

using namespace ikaros;

namespace ikaros
{
std::string  validate_identifier(std::string s)
{
    static std::string legal = "abcdefghijklmnopqrstuvxyzABCDEFGHIJKLMNOPQRSTUVX_1234567890";
    if(s.empty())
        throw exception("Identifier cannot be empty string");
    if('0' <= s[0] && s[0] <= '9')
        throw exception("Identifier cannot start with a number: "+s);
    for(auto c : s)
        if(legal.find(c) == std::string::npos)
            throw exception("Illegal character in identifier: "+s);
    return s;
}


    void Module::Bind(parameter & p, std::string n) // FIXME: Do the fancy recurrency here as well
    {
        Kernel & k = kernel();
        p = k.parameters.at(name_+"."+n);
    };


    void Module::Bind(matrix & m, std::string n)
    {
        Kernel & k = kernel();
        std::string name = name_+"."+n;
        if(k.inputs.count(name))
            m = k.inputs[name];
        else if(k.outputs.count(name))
            m = k.outputs[name];
        else if(k.parameters.count(name))
            m = matrix(42); // FIXME: Get actual matrix later IF matrix parameter
        else
            throw exception("Input or output named "+name+" does not exist");
    }


    void Module::AddInput(dictionary parameters)
    {
        std::string input_name = std::string(info_["name"])+"."+validate_identifier(parameters["name"]);
        kernel().AddInput(input_name, parameters);
    };

    void Module::AddOutput(dictionary parameters)
    {
        std::string output_name = std::string(info_["name"])+"."+validate_identifier(parameters["name"]);
        kernel().AddOutput(output_name, parameters);
      };

    void Module::AddParameter(dictionary parameters)
    {
        std::string pn = parameters["name"];
        if(pn=="name")
            return;
        if(pn=="class")
            return;            
        std::string parameter_name = std::string(info_["name"])+"."+validate_identifier(pn);
        kernel().AddParameter(parameter_name, parameters);
      };

    void Module::SetParameter(std::string name, std::string value)
    {
        if(name=="name")
            return;
        if(name=="class")
            return;  
        std::string parameter_name = std::string(info_["name"])+"."+validate_identifier(name);
        kernel().SetParameter(parameter_name, value);
      };

  Module::Module()
    {
        info_ = kernel().current_module_info;
        name_ = info_["name"];
        for(auto & input: info_["class"]["inputs"])
            AddInput(input["attributes"]);
        for(auto & output: info_["class"]["outputs"])
            AddOutput(output["attributes"]);
        for(auto p: info_["class"]["parameters"])
            AddParameter(p["attributes"]);
        for(auto p: dictionary(info_["parameters"]["attributes"]))
            SetParameter(p.first, p.second);
    }

// The following lines will create the kernel the first time it is accessed by one of the modules

    Kernel& kernel()
    {
        static Kernel * kernelInstance = new Kernel();
        return *kernelInstance;
    }

};