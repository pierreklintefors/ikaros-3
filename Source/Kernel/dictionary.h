// dictionary.h (c) Christian Balkenius 2023-2024
// Recursive JSON-like dictionary

#ifndef DICTIONARY
#define DICTIONARY


#include <string>
#include <vector>
#include <set>
#include <map>
#include <variant>
#include <iterator>
#include <iostream>

#include "utilities.h"
#include "xml.h"

namespace ikaros 
{
    struct null;
    struct dictionary;
    struct list;
    struct value;

    using valueVariant = std::variant<null, std::string, list, dictionary>;

    static std::vector<value> empty;

    struct null
    {
        friend std::ostream& operator<<(std::ostream& os, null v)
        {
            os << "null";
            return os;
        }

        std::string json()
        {
            return "null";
        }

        std::string xml(std::string name, int depth=0)
        {
            return tab(depth)+"<null/>\n";
        }
    };

    struct dictionary
    {
        std::map<std::string, value> dict_;

        value & operator [](std::string s)
        {
            return dict_[s];
        }

        bool contains(std::string s)
        {
            return dict_.count(s);
        }


        size_t count(std::string s)
        {
            return dict_.count(s);
        }


        operator std::string ();
        std::string json();
        std::string xml(std::string name, int depth=0);

        std::map<std::string, value>::iterator begin()  { return dict_.begin(); };
        std::map<std::string, value>::iterator end()   { return dict_.end(); };

        void print();


        friend std::ostream& operator<<(std::ostream& os, dictionary v)
        {
            os << std::string(v);
            return os;
        }

        dictionary() {};
        dictionary(XMLElement * xml);
        dictionary(std::string filename);
    };

    struct list
    {
        std::vector<value> list_;

        std::vector<value>::iterator begin() { return list_.begin(); };
        std::vector<value>::iterator end()   { return list_.end(); };

        int size() { return list_.size(); };
        list & push_back(const value & v) { list_.push_back(v); return *this; };

        operator std::string ();
        std::string json();
        std::string xml(std::string name, int depth=0);

        value & operator[] (int i)
        {
            if(list_.size() < i+1)
                list_.resize(i+1);
            return list_.at(i);
        }


        friend std::ostream& operator<<(std::ostream& os, list v)
        {
            os << std::string(v);
            return os;
        }
    };

    struct value
    {
        valueVariant    value_;

        value(null n=null())         { value_ = null(); }
        value(const char * s)        { value_ = s; }
        value(const std::string & s) { value_ = s; }
        value(const list & v)        { value_ = v; }
        value(const dictionary & d)  { value_ = d; }

        value & operator =(null n) { value_ = null(); return *this; };
        value & operator =(const std::string & s) { value_ = s; return *this; }
        value & operator =(const char * s) { value_ = s; return *this; }
        value & operator =(const list & v) { value_ = v; return *this; }
        value & operator =(const dictionary & d) { value_ = d; return *this; }

        bool is_list() { return std::holds_alternative<list>(value_); }
        bool is_null() { return std::holds_alternative<null>(value_); }


        value & operator[] (const char * s) // Captures literals as argument
        {
            return (*this)[std::string(s)];
        }

        value & operator[] (const std::string & s)
        {
            if(!std::holds_alternative<dictionary>(value_))
            value_ = dictionary();
            return std::get<dictionary>(value_)[s];
        }

        friend std::ostream& operator<<(std::ostream& os, value v)
        {
            os << std::string(v);
            return os;
        }

        value & push_back(const value & v)
        {
            if(!std::holds_alternative<list>(value_))
                value_ = list();
                std::get<list>(value_).list_.push_back(v);
                return *this;
        }

        int size()
        {
            if(std::holds_alternative<list>(value_))
                return std::get<list>(value_).list_.size();
            else if(std::holds_alternative<dictionary>(value_))
                return std::get<dictionary>(value_).dict_.size();
            else if(std::holds_alternative<null>(value_))
                return 0;
            else
                return 1;
        }


        std::vector<value>::iterator begin()
        {
            if(std::holds_alternative<list>(value_))
                return std::get<list>(value_).list_.begin();
            else
                return empty.begin();
        }


        std::vector<value>::iterator end()
        {
            if(std::holds_alternative<list>(value_))
                return std::get<list>(value_).list_.end();
            else
                return empty.end();
        }


        value & operator[] (int i)
        {
            if(!std::holds_alternative<list>(value_))
                value_ = list();
            if(std::get<list>(value_).list_.size() < i+1)
                std::get<list>(value_).list_.resize(i+1);
            return std::get<list>(value_).list_.at(i);
        }

        operator std::string ()
        {
            if(std::holds_alternative<std::string>(value_))
                return std::get<std::string>(value_);
            else if(std::holds_alternative<list>(value_))
                return std::string(std::get<list>(value_));
            else if(std::holds_alternative<dictionary>(value_))
                return std::string(std::get<dictionary>(value_));
            else if(std::holds_alternative<null>(value_))
                return "null";
            else
                return "*";
        }

        std::string json()
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
                return "*";
        }

        std::string xml(std::string name, int depth=0)
        {
            if(std::holds_alternative<std::string>(value_))
                return tab(depth)+"<string>"+std::get<std::string>(value_)+"</string>\n";
            else if(std::holds_alternative<list>(value_))
                return std::get<list>(value_).xml(name, depth);
            else if(std::holds_alternative<dictionary>(value_))
                return std::get<dictionary>(value_).xml(name, depth);
            else if(std::holds_alternative<null>(value_))
                return tab(depth)+"<null/>\n";
            else
                return "*";
        }

        operator double ()
        { 
            if(std::holds_alternative<std::string>(value_))
                return std::stod(std::get<std::string>(value_));
            else
                return 0; // FIXME: throw?
        }

        operator list ()
        {
            return std::get<list>(value_);
        }

            operator dictionary ()
        {
            if(std::holds_alternative<null>(value_))
                return dictionary();
            else
                return std::get<dictionary>(value_);
        }



        dictionary & dref()
        {
            return std::get<dictionary>(value_); // FIXME: No error checking
        }


        void print()
        {
            std::cout << "*";
        }
    };
};
#endif

