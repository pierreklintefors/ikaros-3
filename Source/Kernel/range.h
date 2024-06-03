// range.h - ranges for iteration (c) Christian Balkenius 2023

#ifndef RANGE
#define RANGE

#include <string>
#include <vector>

namespace ikaros
{
    class range
    {
    public:
        std::vector<int>    inc_;   // FIXME: change to vector of individual ranges instead for clearer code
        std::vector<int>    a_;
        std::vector<int>    b_;
        std::vector<int>    index_;

        range();
        range(int a);
        range(int a, int b, int inc=1);
        range(std::string s);                            // parse range string: [a:b:inc]

        range & push(int a, int b, int inc=1);
        range & push(int a);
        range & push();

        range & push_front(int a, int b, int inc=1);

        int rank();
        int size();
        std::vector<int> extent();
        std::vector<int> & index() ;
        std::vector<int> operator++(int);

        void reset(int d=0);
        void clear();
        
        range trim();  // move range to 0..
        range tail();  // drop first dimension // FIXME: pop_front?

        bool is_delay_0(); // FIXME: rename
        bool is_delay_1();

        bool more(int d=0);
        bool empty();

        operator std::vector<int> &();

        range & set(int d, int a, int b, int inc);

        void operator=(const std::vector<int> & v);

        friend void operator|=(range & r, range & s);

        void print();
        void print_index(); // Print the current index position during a loop

        operator std::string();
        std::string curly(); // empty string or range within curly brackets

        friend bool operator==(range & a, range & b);
        friend bool operator!=(range & a, range & b);
        friend bool operator<(range & a, range & b);

        friend std::ostream& operator<<(std::ostream& os, const range & x);
    };
}; // namespace ikaros

#endif

