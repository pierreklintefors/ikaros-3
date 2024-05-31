
// dictionary.h  (c) Christian Balkenius 2023-2024

#include "dictionary.h"


#include "json.cc"  // TEMPORARY EXTERNAL JSON FUNCTIONS


using namespace ikaros;
namespace ikaros 
{

        static void skip_whitespace(const std::string& s, size_t& pos)
        {
            while (pos<s.length() && std::isspace(s[pos]))
                ++pos;
        }

    // null

    null::operator std::string () const
    {
        return "";  // FIXME: Or "null" 
    }

    std::string 
    null::json()
    {
        return "null";
    }

    std::string 
    null::xml(std::string name, int depth, std::string exclude)
    {
        return tab(depth)+"<null/>\n";
    }

    std::ostream&
    operator<<(std::ostream& os, const null & v)
    {
        os << "null";
        return os;
    }


    // list

    list::list():
        list_(std::make_shared<std::vector<value>>())
    {
    }

    list::operator std::string () const
    {
        std::string s = "[";
        std::string sep = "";
        for(auto & v : *list_)
        {
            s += sep + std::string(v);
            sep = ", ";
        }
        s += "]";
        return s;
    }


        list & 
        list::erase(int index)  
        {
                list_->erase(list_->begin()+index); return *this;
        }

    std::string 
    list::json()
    {
        std::string s = "[";
        std::string sep = "";
        for(auto & v : *list_)
        {
            s += sep + v.json();
            sep = ", ";
        }
        s += "]";
        return s;
    }



       value & 
       list::operator[] (int i)
        {
            if( list_->size() < i+1)
                 list_->resize(i+1);
            return  list_->at(i);
        }

        std::ostream& 
        operator<<(std::ostream& os, const list & v)
        {
            os << std::string(v);
            return os;
        }

// dictionary


    value & 
    dictionary::operator[](std::string s)
    {
        return (*dict_)[s];
    }

    int 
    dictionary::get_int(std::string s)
    {
        return (*dict_)[s].as_int();
    }

    bool 
    dictionary::contains(std::string s)
        {
            return dict_->count(s);
        }


        size_t 
        dictionary::count(std::string s)
        {
            return dict_->count(s);
        }
/*
        dictionary::dictionary(const dictionary & d)
        {
            //std::cout << "COPY CONSTRUCTOR" << std::endl;
            dict_ = d.dict_;
        }
*/
        dictionary::dictionary():   
            dict_(std::make_shared<std::map<std::string, value>>())
        {};


        void dictionary::merge(const dictionary & source, bool overwrite) // shallow merge: copy from source to this
        {
            for(auto p : *(source.dict_))
                if(!dict_->count(p.first) || overwrite)
                    (*dict_)[p.first] = p.second;
        }


    dictionary::operator std::string () const
    {
        std::string s = "{";
        std::string sep = "";
        for(auto & v : *dict_)
        {
            s += sep + "" + v.first + ": " + std::string(v.second);
            sep = ", ";
        }
        s += "}";
        return s;
    }



    std::string  
    dictionary::json()
    {
        std::string s = "{";
        std::string sep = "";
        for(auto & v : *dict_)
        {
            s += sep + "\"" + v.first + "\": " + v.second.json();
            sep = ", ";
        }
        s += "}";
        return s;
    }


    // XML

    std::string 
    list::xml(std::string name, int depth, std::string exclude)
    {
        std::string s;
        for(auto & v : *list_)
        {
            s += v.xml(name, depth+1);
        }
        return s;
    }

    // FIXME: Use _tag for elememt name

    std::string  
    dictionary::xml(std::string name, int depth, std::string exclude)
    {
        std::string s = tab(depth)+"<"+name;
        for(auto & a : *dict_)
            if(!a.second.is_list())
                if(!a.second.is_null()) // Do not include null attributes - but include empty strings
                    if(a.first != "_tag") // Do not include tag attributes since the are used as element name
                        s += " "+a.first + "=\"" +std::string(a.second)+"\"";

        std::string sep = ">\n";
        for(auto & e : *dict_)
            if(e.second.is_list()) //  && e.first!="inputs" && e.first!="outputs" && e.first!="parameters"
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


    dictionary::dictionary(XMLElement * xml_node):
        dictionary()
    {
        (*dict_)["_tag"] = xml_node->name;
        for(XMLAttribute * a = xml_node->attributes; a!=nullptr; a=(XMLAttribute *)(a->next))
            (*dict_)[std::string(a->name)] = a->value;

        for (XMLElement * xml_element = xml_node->GetContentElement(); xml_element != nullptr; xml_element = xml_element->GetNextElement())
            //if(merge.empty())
                (*dict_)[std::string(xml_element->name)+"s"].push_back(dictionary(xml_element));
            //else
            //    (*dict_)["elements"].push_back(dictionary(xml_element));
    }

    dictionary::dictionary(std::string filename):
        dictionary(XMLDocument(filename.c_str()).xml)
    {
    }


    void 
    dictionary::parse_url(std::string s)
    {
        if(s.empty())
            return;

        for(auto p : split(s, "&"))
        {   
            std::string a = head(p,"=");
            (*this)[a] = p;
        }
    }


    static void 
    copy_from_json(dictionary & d, const std::unordered_map<std::string, JSON>& json);
    
    static void 
    copy_from_json(list & l, const std::vector<JSON> & json)
    {
        for (auto& value : json)
        {
            if(value.isString())
            {
                //std::cout << value.asString() << std::endl;
                l.push_back(value.asString());
            }
            else if(value.isNull())
            {
                //std::cout << "NULL" << std::endl;
                l.push_back(null());
            }
            else if(value.isBool())
            {
                //std::cout << "BOOL" << std::endl;
                if(value.asBool())
                    l.push_back("true");
                else
                    l.push_back("false");
            }
            else if(value.isNumber())
            {
                //std::cout << "NUMBER" << std::endl;
                l.push_back(std::to_string(value.asNumber()));
            }
            else if(value.isArray())
            {
                //std::cout << "ARRAY" << std::endl;
                list ll;
                copy_from_json(ll, value.asArray());
                l.push_back(ll);
            }
            else if(value.isObject())
            {
                //std::cout << "OBJECT" << std::endl;
                dictionary dd;
                copy_from_json(dd, value.asObject());
                l.push_back(dd);
            }
        }
    }
    
    static void 
    copy_from_json(dictionary & d, const std::unordered_map<std::string, JSON>& json)
    {
        for (auto& [key, value] : json)
        {
            if(value.isString())
            {
                //std::cout << key << ": " << value.asString() << std::endl;
                d[key] = value.asString();
            }
            else if(value.isNull())
            {
                //std::cout << key << ": " << "NULL" << std::endl;
                d[key] = null();
            }
            else if(value.isBool())
            {
                //std::cout << key << ": " << "BOOL" << std::endl;
                if(value.asBool())
                    d[key] ="true";
                else
                    d[key] = "false";
            }
            else if(value.isNumber())
            {
                //std::cout << key << ": " << "NUMBER" << std::endl;
                d[key] = std::to_string(value.asNumber());
            }
            else if(value.isArray())
            {
                //std::cout << key << ": " << "ARRAY" << std::endl;
                list l;
                copy_from_json(l, value.asArray());
                d[key] = l;
            }
            else if(value.isObject())
            {
                //std::cout << key << ": " << "OBJECT" << std::endl;
                dictionary dd;
                copy_from_json(dd, value.asObject());
                d[key] = dd;
            }
        }
    }


    void
    dictionary::parse_json(std::string s)
    {
        JSON json = JSON::parse(s);
       // std::cout << "Parsed JSON successfully." << std::endl;

        // 

        copy_from_json(*this, json.asObject());
    }








        std::ostream& operator<<(std::ostream& os, const dictionary & v)
        {
            os << std::string(v);
            return os;
        }
// value

    value & 
    value::operator[] (const char * s) // Captures literals as argument
        {
            return (*this)[std::string(s)];
        }

        value & 
        value::operator[] (const std::string & s)
        {
            if(!std::holds_alternative<dictionary>(value_))
            value_ = dictionary();
            return std::get<dictionary>(value_)[s];
        }

        std::ostream& operator<<(std::ostream& os, const value & v)
        {
            os << std::string(v);
            return os;
        }

        value &  
        value::push_back(const value & v)
        {
            if(!std::holds_alternative<list>(value_))
                value_ = list();
                std::get<list>(value_). list_->push_back(v);
                return *this;
        }

        int  
        value::size()
        {
            if(std::holds_alternative<list>(value_))
                return std::get<list>(value_). list_->size();
            else if(std::holds_alternative<dictionary>(value_))
                return std::get<dictionary>(value_).dict_->size();
            else if(std::holds_alternative<null>(value_))
                return 0;
            else
                return 1;
        }


        std::vector<value>::iterator value::begin()
        {
            if(std::holds_alternative<list>(value_))
                return std::get<list>(value_). list_->begin();
            else
                return empty.begin();
        }


        std::vector<value>::iterator value::end()
        {
            if(std::holds_alternative<list>(value_))
                return std::get<list>(value_). list_->end();
            else
                return empty.end();
        }


        value &   
        value::operator[] (int i)
        {
            if(!std::holds_alternative<list>(value_))
                value_ = list();
            if(std::get<list>(value_). list_->size() < i+1)
                std::get<list>(value_). list_->resize(i+1);
            return std::get<list>(value_). list_->at(i);
        }

        value::operator std::string () const
        {
            if(std::holds_alternative<bool>(value_))
                return std::get<bool>(value_) ? "true" : "false";
            if(std::holds_alternative<int>(value_))
                return std::to_string(std::get<int>(value_));
            if(std::holds_alternative<float>(value_))
                return std::to_string(std::get<float>(value_));
            if(std::holds_alternative<std::string>(value_))
                return std::get<std::string>(value_);
            else if(std::holds_alternative<list>(value_))
                return std::string(std::get<list>(value_));
            else if(std::holds_alternative<dictionary>(value_))
                return std::string(std::get<dictionary>(value_));
            else if(std::holds_alternative<null>(value_))
                return "null";                                              // FIXME: return rempty string???
            else
                return "*";
        }

        std::string  
        value::json()
        {
            if(std::holds_alternative<std::string>(value_))
                return "\""+std::get<std::string>(value_)+"\"";
            else if(std::holds_alternative<list>(value_))
                return std::get<list>(value_).json();
            else if(std::holds_alternative<dictionary>(value_))
                return std::get<dictionary>(value_).json();
            else if(std::holds_alternative<null>(value_))
                return "null";
            else
                return std::string(*this);
        }

        std::string  
        value::xml(std::string name, int depth, std::string exclude)
        {
            if(std::holds_alternative<std::string>(value_))
                return tab(depth)+"<string>"+std::get<std::string>(value_)+"</string>\n"; // FIXME: <'type' value="v" />    ???
            else if(std::holds_alternative<list>(value_))
                return std::get<list>(value_).xml(name, depth);
            else if(std::holds_alternative<dictionary>(value_))
                return std::get<dictionary>(value_).xml(name, depth);
            else if(std::holds_alternative<null>(value_))
                return tab(depth)+"<null/>\n";
            else
                return std::string(*this);                              // FIXME: <'type' value="v" />    ???
        }

          
        value::operator double ()                                          // FIXME: Add other types - both from and to
        { 
            if(std::holds_alternative<std::string>(value_))
                return std::stod(std::get<std::string>(value_));
            else
                return 0; // FIXME: throw?
        }

          
        value::operator list ()
        {
            return std::get<list>(value_);
        }

  
        value::operator dictionary ()
        {
            if(std::holds_alternative<null>(value_))
                return dictionary();
            else
                return std::get<dictionary>(value_);
        }


/*

        void  
        value::print()
        {
            std::cout << "*";
        }
*/

};

