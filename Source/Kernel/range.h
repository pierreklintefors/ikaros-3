// range.h - ranges for iteration

// TODO: Add enumerated ranges: c,b,c
// TODO: trim= at appropriate places

#ifndef RANGE
#define RANGE

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <iterator>

#include "utilities.h"

namespace ikaros {

class range
{
public:
    std::vector<int>    inc_;
    std::vector<int>    a_;
    std::vector<int>    b_;
    std::vector<int>    index_;

    range() {};
    range(int a, int b, int inc=1):
        inc_(std::vector<int>{inc}),
        a_(std::vector<int>{a}),
        b_(std::vector<int>{b}),
        index_(std::vector<int>{inc>0 ? a : a+inc*((b-a-1)/inc)})
     {};
     range(int a) : range(a, a+1, 1) {};

     range & push(int a, int b, int inc=1)
     {
        a_.push_back(a);
        b_.push_back(b);
        inc_.push_back(inc);
        if(inc == 0)
            index_.push_back(0);
        else
            index_.push_back(inc>0 ? a : a+inc*((b-a)/inc));
        return *this;
     };

     range & push(int a)
     {
        return push(a, a+1, 1);
     };

     range & push()
     {
        return push(0, 0, 0);
     };


     range(std::string s) // parse range string: "[][7][1:2][10:20:-1]..."
    {                       // MEW: parse range string: ";7;1:2;10:20:-1"      ";" == two dimensions
        if(s.empty())
            return;
        if(s[0] != '[')
            throw std::invalid_argument("Malformed range string");
        if(s[s.size()-1] != ']')
            throw std::invalid_argument("Malformed range string");

        std::vector<std::string> ranges = split(s.substr(1, s.size()-2), "][");

        for(auto & ss : ranges)
        {
            auto & r = split(ss, ":");
            if(r.size()==1)
            {
                if(r[0].empty())
                    push(0, 0, 0);
                else
                    push(std::stoi(r[0]), std::stoi(r[0])+1, 1); // single index
        }
            else if(r.size()==2)
            {
                int a = r[0].empty() ? 0 : std::stoi(r[0]);
                int b = r[1].empty() ? 0 : std::stoi(r[1]);
                if(a== 0 && b==0)
                    push(0, 0, 0);
                else
                    push(a, b, 1);
            }
            else if(r.size()==3)
            {
                int a = r[0].empty() ? 0 : std::stoi(r[0]);
                int b = r[1].empty() ? 0 : std::stoi(r[1]);
                int i = r[2].empty() ? (a==0 && b==0 ? 0 : 1) : std::stoi(r[2]);
                push(a, b, i);
            }
            else
               throw std::invalid_argument("Malformed range string");
        }
    }

    std::vector<int>  & index() { return index_; };


    std::vector<int>  operator++(int)
    {
        for(int d=index_.size()-1; d>0; d--)
        {
            index_[d]+=inc_[d];
            if(more(d))
                return index();
            reset(d);
        }
        index_[0]+=inc_[0];
        return index();
    };

    void reset(int d=0)
    {
        if(d>=index_.size())
            return;
        if(inc_[d] == 0)
            return;
        index_[d] = inc_[d]>0 ? a_[d] : a_[d]+inc_[d]*((b_[d]-a_[d]-1)/inc_[d]);
    };

    bool more(int d=0)
    {
        return (inc_[d] > 0 && index_[d] < b_[d]) || (inc_[d] < 0 && index_[d] >= a_[d]);
    };

    operator std::vector<int> &() { return index(); };

    range & set(int d, int a, int b, int inc)
    {
        a_.at(d) = a;
        b_.at(d) = b;
        inc_.at(d) = inc;
        index_.at(d) = inc_[d]>0 ? a_[d] : a_[d]+inc_[d]*((b_[d]-a_[d]-1)/inc_[d]);
        return *this;
    };


    friend void operator|=(range & r, range & s)
    {
        for(int d=0; d<r.index_.size(); d++)
        {
            if(s.a_[d] < r.a_[d])
                r.a_[d] = s.a_[d];
            if(s.b_[d] > r.b_[d])
                r.b_[d] = s.b_[d];
            r.inc_[d] = 1;
            r.index_[d] = r.inc_[d]>0 ? r.a_[d] : r.a_[d]+r.inc_[d]*((r.b_[d]-r.a_[d]-1)/r.inc_[d]);
        }
    };


    void print_index()
    {
        for(auto x: index_)
            std::cout << x << " ";
        std::cout << std::endl;
    };


    operator std::string()
    {
        std::string s;
        std::string sep;
        s += "[";
        for(int d=0; d< index_.size(); d++)
        {
            s +=  sep;

            if(inc_[d] == 0) // empty/full range
                ;

            else if(a_[d] == b_[d]-1 && inc_[d]==1) // single index
                s+=std::to_string(a_[d]);

            else
            {
                if(a_[d] != 0)
                    s+= std::to_string(a_[d]); // always print start of range

                if(b_[d] == 0)
                    s += ":";
                else 
                    s += ":"+std::to_string(b_[d]);

                if(inc_[d] != 1)
                    s += ":"+std::to_string(inc_[d]);
            }
            sep = "][";
        }
        s +=  "]";
        return s;   
    };

    friend bool operator==(range & a, range & b)
    {
        return a.index_==b.index_ && a.a_==b.a_ && a.b_==b.b_ && a.inc_==b.inc_;
    }
    
    friend bool operator!=(range & a, range & b)
    {
        return !(a==b);
    }

    friend bool operator<(range & a, range & b)
    {
        return false;
    }

    int size()
    {
        if(index_.size() == 0)
            return 0;

        int s=1;
        for(int d=0; d< index_.size(); d++)
            s *= (b_[d]-a_[d]);
      return s;
    }

    friend std::ostream& operator<<(std::ostream& os, const range & x);
    
    int rank()
    {
        return index_.size();
    }


    //rank(matrix m) {} // or conversion operator  probably better
};




}; // namespace ikaros

#endif

