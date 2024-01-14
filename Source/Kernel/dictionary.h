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

    using valueVariant = std::variant<bool, int, float, null, std::string, list, dictionary>;
    using mapPtr = std::shared_ptr<std::map<std::string, value>>;
    using listPtr = std::shared_ptr<std::vector<value>>;

    static std::vector<value> empty;

    struct null
    {
        operator std::string () const;
        std::string json();
        std::string xml(std::string name, int depth=0, std::string exclude = "");
        friend std::ostream& operator<<(std::ostream& os, const null & v);
    };


    struct dictionary
    {
        mapPtr dict_;

        std::map<std::string, value>::iterator begin()  { return dict_->begin(); };
        std::map<std::string, value>::iterator end()   { return dict_->end(); };

        dictionary();
        dictionary(XMLElement * xml);
        dictionary(std::string filename);
        //dictionary(const dictionary & d);
    


        value & operator[](std::string s);
        bool contains(std::string s);
        size_t count(std::string s);
        void merge(const dictionary & source, bool overwrite=false); // shallow merge: copy from source to this
        operator std::string () const;
        std::string json();
        std::string xml(std::string name, int depth=0, std::string exclude = "");
        friend std::ostream& operator<<(std::ostream& os, const dictionary & v);
        //void print();

        void parse_url(std::string s);
    };


    struct list
    {
        listPtr list_;

        list();
    
        std::vector<value>::iterator begin() { return list_->begin(); };
        std::vector<value>::iterator end()   { return list_->end(); };

        value & operator[] (int i);
        int size() { return list_->size(); };
        list & push_back(const value & v) { list_->push_back(v); return *this; };
        list & erase(int index);
        operator std::string ()  const;
        std::string json();
        std::string xml(std::string name, int depth=0, std::string exclude = "");
        friend std::ostream& operator<<(std::ostream& os, const list & v);
    };


    struct value
    {
        valueVariant    value_;

        value(bool v)                { value_ = v; }
        value(int v)                 { value_ = v; }
        value(float v)               { value_ = v; }
        value(null n=null())         { value_ = null(); }
        value(const char * s)        { value_ = s; }
        value(const std::string & s) { value_ = s; }
        value(const list & v)        { value_ = v; }
        value(const dictionary & d)  { value_ = d; }

        value & operator =(bool v) { value_ = v; return *this; }
        value & operator =(int v) { value_ = v; return *this; }
        //value & operator =(float v) { value_ = v; return *this; }
        value & operator =(double v) { value_ = float(v); return *this; }
        value & operator =(null n) { value_ = null(); return *this; };
        value & operator =(const std::string & s) { value_ = s; return *this; }
        value & operator =(const char * s) { value_ = s; return *this; }
        value & operator =(const list & v) { value_ = v; return *this; }
        value & operator =(const dictionary & d) { value_ = d; return *this; }

        bool is_dictionary()    { return std::holds_alternative<dictionary>(value_); }
        bool is_list()          { return std::holds_alternative<list>(value_); }
        bool is_null()          { return std::holds_alternative<null>(value_); }


        value & operator[] (const char * s); // Captures literals as argument
        value & operator[] (const std::string & s);

        value & push_back(const value & v);

        friend std::ostream& operator<<(std::ostream& os, const value & v);
        int size();

        std::vector<value>::iterator begin();
        std::vector<value>::iterator end();

        value & operator[] (int i);

        operator std::string () const;
        std::string json();

        std::string xml(std::string name, int depth=0, std::string exclude = "");

        operator double ();                                        // FIXME: Add other types - both from and to
        operator list ();
        operator dictionary ();

        //void print();
    };
};
#endif

