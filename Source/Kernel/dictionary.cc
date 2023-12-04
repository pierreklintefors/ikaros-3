
// dictionary.h  (c) Christian Balkenius 2023

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
list::xml()
{
    std::string s; //  = "\n<list>\n"; // FIXME: Look at tag
    std::string sep = "";
    for(auto & v : list_)
    {
        s += sep + v.xml();
        sep = "\n";
    }
    //s += "\n</list>\n";
    return s;
}

std::string  
dictionary::xml()
{
    std::string s = "<";
    s += std::string(dict_["tag"]) + " ";
    for(auto & a : dictionary(dict_["attributes"]))
        if(a.first != "tag")
            s += a.first + "=" + a.second.xml() + " ";

    if(dict_.size() == 1)
    {
        s += "/>\n";
        return s;
    }
    else
        s += ">\n";

    for(auto & e : dict_)
        if(e.first != "tag" && e.first != "attributes" && !e.second.is_null())
        {
            s += e.second.xml();
        }
    s += "</"+std::string(dict_["tag"])+">";
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

dictionary::dictionary(XMLElement * xml_node, bool merge)
{
    if(std::string(xml_node->name) == "view")
        merge = false;

    dict_["tag"] = std::string(xml_node->name);
    for(XMLAttribute * a = xml_node->attributes; a!=nullptr; a=(XMLAttribute *)(a->next))
        dict_["attributes"][std::string(a->name)] = a->value;

    for (XMLElement * xml_element = xml_node->GetContentElement(); xml_element != nullptr; xml_element = xml_element->GetNextElement())
        if(merge)
            dict_[std::string(xml_element->name)+"s"].push_back(dictionary(xml_element, merge));
        else
            dict_["elements"].push_back(dictionary(xml_element, merge));
}

dictionary::dictionary(std::string filename, bool merge)
{
    XMLDocument doc(filename.c_str());
    dict_ = dictionary(doc.xml, merge).dict_;
}
