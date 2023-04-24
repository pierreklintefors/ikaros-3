// options.h   (c) Christian Balkenius 2023

#ifndef OPTIONS
#define OPTIONS

namespace ikaros {

class options
{
public:
    std::vector<std::string> filenames;
    std::map<std::string, std::string> d;

    options() {};

    void set_default(std::string a, std::string v)
    {
        d[a] = v;
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
                d[s.substr(1, 1)] = s.substr(2);  // option with parameter
            else if(s.size()>1 && s.front() =='-')
                d[s.substr(1, 1)] = "";  // option without parameter
            else
            {
                if (!std::filesystem::exists(s))
                        throw std::runtime_error(std::string(s) + ": File not found");
                filenames.push_back(s);
            }
        }
        for(auto & x : d)
            std::cout << x.first << " = " << x.second << std::endl;
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
    };
};

};
#endif
