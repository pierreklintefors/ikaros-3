
// dictionary.h  (c) Christian Balkenius 2023-2024

#include "dictionary.h"


using namespace ikaros;

list::operator std::string ()
{
    std::string s = "[";
    std::string sep = "";
    for(auto & v : list_)
    {
        s += sep + std::string(v);
        sep = ", ";
    }
    s += "]";
    return s;
}

dictionary::operator std::string ()
{
    std::string s = "{";
    std::string sep = "";
    for(auto & v : dict_)
    {
        s += sep + "" + v.first + ": " + std::string(v.second);
        sep = ", ";
    }
    s += "}";
    return s;
}

// JSON

std::string 
list::json()
{
    std::string s = "[";
    std::string sep = "";
    for(auto & v : list_)
    {
        s += sep + v.json();
        sep = ", ";
    }
    s += "]";
    return s;
}

std::string  
dictionary::json()
{
    std::string s = "{";
    std::string sep = "";
    for(auto & v : dict_)
    {
        s += sep + "\"" + v.first + "\": " + v.second.json();
        sep = ", ";
    }
    s += "}";
    return s;
}

// XML

std::string 
list::xml(std::string name, int depth)
{
    std::string s;
    for(auto & v : list_)
    {
        s += v.xml(name, depth+1);
    }
    return s;
}

std::string  
dictionary::xml(std::string name, int depth)
{
    std::string s = tab(depth)+"<"+name;
    for(auto & a : dict_)
        if(!a.second.is_list())
        s += " "+a.first + "=\"" +std::string(a.second)+"\"";

    std::string sep = ">\n";
    for(auto & e : dict_)
        if(e.second.is_list()) 
        {
            s += sep + e.second.xml(e.first.substr(0, e.first.size()-1), depth);
            sep = "";
        }
    if(sep.empty())
        s += tab(depth)+"</"+name+">\n";
    else
        s +="/>\n";
    return s;
}


// Print

void dictionary::print()
{
    for(auto & v : dict_)
    {
        std::cout << std::string(*this) << std::endl;
    }
}

dictionary::dictionary(XMLElement * xml_node)
{
    for(XMLAttribute * a = xml_node->attributes; a!=nullptr; a=(XMLAttribute *)(a->next))
        dict_[std::string(a->name)] = a->value;

    for (XMLElement * xml_element = xml_node->GetContentElement(); xml_element != nullptr; xml_element = xml_element->GetNextElement())
        //if(merge.empty())
            dict_[std::string(xml_element->name)+"s"].push_back(dictionary(xml_element));
        //else
        //    dict_["elements"].push_back(dictionary(xml_element));
}

dictionary::dictionary(std::string filename):
    dictionary(XMLDocument(filename.c_str()).xml)
{
}
