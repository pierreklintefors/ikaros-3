
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

void dictionary::print()
{
    for(auto & v : dict_)
    {
        std::cout << std::string(*this) << std::endl;
    }
}


dictionary::dictionary(XMLElement * xml_node, bool merge)
{
    dict_["tag"] = std::string(xml_node->name);
    for(XMLAttribute * a = xml_node->attributes; a!=nullptr; a=(XMLAttribute *)(a->next))
        dict_["attributes"][a->name] = a->value;

    for (XMLElement * xml_element = xml_node->GetContentElement(); xml_element != nullptr; xml_element = xml_element->GetNextElement())
        if(merge)
            dict_[std::string(xml_element->name)+"s"].push_back(dictionary(xml_element));
        else
            dict_["elements"].push_back(dictionary(xml_element));
}


dictionary::dictionary(std::string filename, bool merge)
{
    XMLDocument doc(filename.c_str());
    dict_ = dictionary(doc.xml, merge).dict_;
}
