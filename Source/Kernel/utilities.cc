// utilities.cc

#include "utilities.h"

namespace ikaros
{

std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

const std::vector<std::string>
split(const std::string & s, const std::string & sep, int maxsplit)
{
    std::vector<std::string> r;
    std::string::size_type i=0, j=0;
    std::string::size_type len = s.size();
    std::string::size_type n = sep.size();

    if (n == 0)
    {
        while(i<len)
        {
            while (i < len && ::isspace(s[i]))
                i++;
            j = i;
            while (i < len && !::isspace(s[i]))
                i++;

            if(j < i)
            {
                if(maxsplit != -1 && maxsplit-- <= 0)
                    break;
                r.push_back(trim(s.substr(j, i-j)));
                while(i < len && ::isspace(s[i]))
                    i++;
                j = i;
            }
        }
        if (j < len)
            r.push_back(trim(s.substr(j, len - j)));

        return r;
    }

    i = j = 0;
    while (i+n <= len)
    {
        if (s[i] == sep[0] && s.substr(i, n) == sep)
        {
            if(maxsplit != -1 && maxsplit-- <= 0)
                break;

            r.push_back(trim(s.substr(j, i - j)));
            i = j = i + n;
        }
        else
            i++;
    }

    r.push_back(trim(s.substr(j, len-j)));
    return r;
}


const std::vector<std::string>
rsplit(const std::string & str, const std::string & sep, int maxsplit)
{
    if (maxsplit < 0)   // FIXME: Is this necessary?? WHY???
        return split(str, sep, maxsplit);

    std::vector<std::string> r;
    std::string::size_type i=str.size();
    std::string::size_type j=str.size();
    std::string::size_type n=sep.size();

    if(n == 0)
    {
        while(i > 0)
        {
            while(i > 0 && ::isspace(str[i-1]))
                i--;
            j = i;
            while(i > 0 && !::isspace(str[i-1]))
                i--;

            if (j > i)
            {
                if(maxsplit != -1 && maxsplit-- <= 0)
                    break;
                r.push_back(str.substr(i, j-i));
                while(i > 0 && ::isspace(str[i-1]))
                    i--;
                j = i;
            }
        }
        if (j > 0)
            r.push_back( str.substr(0, j));
    }
    else
    {
        while(i >= n)
        {
            if(str[i-1] == sep[n-1] && str.substr(i-n, n) == sep)
            {
                if(maxsplit != -1 && maxsplit-- <= 0)
                    break;
                r.push_back(str.substr(i, j-i));
                i = j = i-n;
            }
            else
                i--;
        }
        r.push_back(str.substr(0, j));
    }
    
    std::reverse(r.begin(), r.end());
    return r;
}


std::string
join(const std::string & separator, const std::vector<std::string> & v, bool reverse)
{
    std::string s;
    std::string sep;
    if(reverse)
        for (auto & e : v)
        {
            s = e + sep + s;
            sep = separator;
        }    
    else
        for (auto & e : v)
        {
            s += sep + e;
            sep = separator;
        }
    return s;
}



bool
starts_with(const std::string & s, const std::string & start) // waiting for C++20
{
    return start.length() <= s.length() && equal(start.begin(), start.end(), s.begin()); // more efficient than find
}

bool ends_with(const std::string & s, const std::string & end) // waiting for C++20
{
    return  s.size() >= end.size() && s.compare(s.size() - end.size(), end.size(), end) == 0;
}



const std::string head(std::string s, char token) // without token
{
    int p = s.find(token);
    if(p==std::string::npos)
        return s;
    else
        return s.substr(0, p);
}

const std::string tail(std::string s, char token, bool include_token) // including token
{
    int p = s.find(token);
    if(p==std::string::npos)
        return "";
    else if(include_token)
        return s.substr(p, s.size()-p);
    else
        return s.substr(p+1, s.size()-p-1);
}


const std::string rhead(std::string s, char token) // without token
{
    int p = s.rfind(token);
    if(p==std::string::npos)
        return s;
    else
        return s.substr(0, p);
}

const std::string rtail(std::string s, char token) // without token
{
    int p = s.rfind(token);
    if(p==std::string::npos)
        return s; // was ""
    else
        return s.substr(p+1, s.size()-p);
}



bool contains(std::string & s, std::string n)
{
    return s.find(n) != std::string::npos;
}



bool is_integer(std::string & s)
    {
        for( char c: s)
            if(c<'0' || c>'9')
            return false;
        return true;
    }


bool is_true(const std::string & s)
{
    if(s.empty())
        return false;
    if(s[0] == 'T')
        return true;
     if(s[0] == 't')
        return true;
    if(s[0] == 'Y')
        return true;
    if(s[0] == 'y')
        return true;
    if(s=="1")
        return true;
    return false;
}

std::ostream& operator<<(std::ostream& os, const std::vector<int> & v)
{
    std::string sep;
    std::cout << "(";
    for(auto i : v)
    {
        std::cout << sep << i;
        sep = ", ";
    }
    std::cout << ")";
    return os;
}



// Utility functions

    // auto tab = [](int d){ return std::string(3*d, ' ');};

    void 
    print_attribute_value(const std::string & name, int value, int indent)
    {
        std::cout << name << " = " << value <<  std::endl;
    }

    void 
    print_attribute_value(const std::string & name, const std::string & value, int indent)
    {
        std::cout << name << " = " << value <<  std::endl;
    }

    void 
    print_attribute_value(const std::string & name, const std::vector<int> & values, int indent, int max_items)
    {
                std::cout << name << " = ";
        int s = values.size();
        if(max_items>0 && s>max_items)
            s = max_items;

        for(int i=0; i<s; i++)
            std::cout << values.at(i) << " ";
        if(values.size() >= max_items && max_items>0)
            std::cout << "..." << std::endl;
        std::cout << std::endl;
    }
   
    void 
    print_attribute_value(const std::string name, std::vector<float> & values, int indent, int max_items)
    {
                std::cout << name << " = ";
        int s = values.size();
        if(max_items>0 && s>max_items && max_items>0)
            s = max_items;

        for(int i=0; i<s; i++)
            std::cout << values.at(i) << " ";
        if(values.size() >= max_items)
            std::cout << "..." << std::endl;
        std::cout << std::endl;
    }

    void
    print_attribute_value(const std::string & name, const std::vector<std::vector<std::string>> &  values, int indent, int max_items)
    {
        std::cout << name << " = " << std::endl;
        for(auto d : values)
        {
            std::cout << tab(1);
            if(d.empty())
                std::cout << "none" << std::endl;
            else
                for(auto s : d)
                    std::cout << s << " ";
        }
        std::cout << std::endl;
    }

};
