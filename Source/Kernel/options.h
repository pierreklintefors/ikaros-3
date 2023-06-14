// options.h   (c) Christian Balkenius 2023

#ifndef OPTIONS
#define OPTIONS

namespace ikaros {

class options
{
public:
    std::vector<std::string> filenames;
    std::map<std::string, std::string> d;
    std::map<std::string, std::string> full;
    std::map<std::string, std::string> description;

    options() {};

    void add_option(std::string short_name, std::string full_name, std::string desc, std::string default_value="")
    {
        full[short_name] = full_name;
        description[full_name] = desc;
        if(!default_value.empty())
            d[full_name] = default_value;
    }


    void parse_args(int argc, char *argv[])
    {
        if (argc > 64)
            throw std::runtime_error("Too many input parameters");
        if (argc < 1)
            throw std::runtime_error("Too few input parameters");
        const std::vector<std::string> args(argv+1, argv+argc);
        for(auto & s: args)
        {
            int pos = s.find('=');
            if(pos != std::string::npos)
            {
                if(pos < 1)
                    throw std::runtime_error("Assignment without variable");
                if(s.size() >3 && s.at(pos+1)== '"' && s.back()== '"')
                    d[s.substr(0, pos)] = s.substr(pos+2, s.size()-pos-3);
                else
                    d[s.substr(0, pos)] = s.substr(pos+1, s.size()-pos);
            }
            else if(s.size()>2 && s.front() =='-')
            {
                std::string attr = s.substr(1, 1);
                if(!full.count(attr))
                    throw std::runtime_error("\"-"+attr + "\" is not a valid command line option");
                d[full[attr]] = s.substr(2);  // option with parameter
            }
            else if(s.size()>1 && s.front() =='-')
            {
                std::string attr = s.substr(1, 1);
                if(!full.count(attr))
                    throw std::runtime_error("\"-"+attr + "\" is not a valid command line option");
                d[full[attr]] = "";  // option without parameter
            }
            else
            {
                if (!std::filesystem::exists(s))
                        throw std::runtime_error(std::string(s) + ": File not found");
                filenames.push_back(s);
            }
        }
    }

    bool is_set(std::string o)
    {
        return d.count(o)>0;
    }

    std::string get(std::string o)
    { 
        if(d.count(o)>0)
             return d.at(o);
        else
            return "";
    }

    long get_long(std::string o)
    { 
        if(d.count(o)>0)
             return std::stol(d.at(o));
        else
            return 0;
    }
};

};
#endif
