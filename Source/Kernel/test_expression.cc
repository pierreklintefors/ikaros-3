#include <iostream>
#include <string>
#include <vector>

#include "utilities.h"

// https://stackoverflow.com/questions/16425571/unary-minus-in-shunting-yard-expression-parser

/*
When parsing the operators, take into account the cases when the operator is - and it is 
preceded by another operator, or preceded by a left parenthesis, or when it is the first 
character of the input (these cases mean that it is a unary minus rather than binary). 
In this case, you change it to another character, say u (this was my case), and make its 
precedence the same as that of ^.
*/

// binary operators: +, -, *, /
// unary operators: +, - [first in string or after (, * or / or before (]
// unary indirection operator: @ (treated as pasrt if variable name)
// values: variable_name, integer, float
// variable_name: including @, . and _

// value type: int, float, string
// list separated by , is handled at a higher level: list == expr, expr, ..., expr

void
erase_whitespace(std::string & s)
{
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char x){ return std::isspace(x); }), s.end());
}

// split string at token except withing parantheses or after a chacter in not_after-list (for unary minus mainly)

bool split_expression(std::string & head, std::string & tail, std::string & s, char token, bool unary=false)
{
    if(s.empty())
        return false;

    int p_count = 0;
    for(int i=s.size()-1; i>=0 ; i--)
    {
        if(s[i]=='(')
            p_count++;
        if(s[i]==')')
            p_count--;
        if(p_count==0 && s[i]==token)
        {
            bool match = true;
            std::string not_after = "+(*/";
            if(unary)
                for(int j=0; j<not_after.size(); j++)
                    if(i>0 && s[i-1]==not_after[j])
                    {
                        match = false; // a goto here is sooooo tempting...if only C++ had a decent continue statement...or internal functions
                        break;
                    }
            
            if(match)
            {    
                int start=0;
                while(isspace(s[start]))
                    start++;
                    
                    head = s.substr(start, i);
                    tail = s.substr(i+1, s.size()-i);
                return true;
            }
        }
    }
    return false;
}


class expression
{
    public:
        bool split(std::string s, char bin_op, bool unary=false)
        {
            std::string left_string, right_string;
            if(!split_expression(left_string, right_string, s, bin_op, unary))
                return false;
    
        op = bin_op;
        left = std::make_unique<expression>(left_string);
        right = std::make_unique<expression>(right_string);
        return true;
        }

        expression(std::string s)
        {
            op = ' ';
            erase_whitespace(s);
            str = s; 

            if(s.empty())
                return;

            if(s.size()>1 && s.front()=='(' && s.back() == ')')
                str = s = s.substr(1, s.size()-2);

            if(split(s, '+'))
                return;

            if(split(s, '-', true))
                return;

            if(split(s, '/'))
                return;

            if(split(s, '*'))
                return;

            if(split(s, '-')) // unary minus
                return;

            // terminal - compute values of str
            std::cout << "COMPUTE:" << str << std::endl;
        }

        float evaluate()
        {
            try
            {
                switch(op)
                {
                    case ' ':   return str.empty() ? 0 : std::stoi(str); // FIXME: variable lookup
                    case '+':   return left->evaluate() + right->evaluate();
                    case '-':   return left->evaluate() - right->evaluate();
                    case '*':   return left->evaluate() * right->evaluate();
                    case '/':   return left->evaluate() / right->evaluate(); // trow exception if right == 0
                default:
                    return 0;
                }   
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                // FIXME: rethrow *********************** OR NOT
                return 0;
            }
            

        }

        void print(int depth=0)
        {
            if(op==' ')
                std::cout << ::ikaros::tab(depth) << "[" << str << "]" << std::endl;
            else
                std::cout << ::ikaros::tab(depth) << op << std::endl;
            if(left)
                left->print(depth+1);
            if(right)
                right->print(depth+1);
            if(depth == 0)
                std::cout << std::endl;
        }

        char op;
        std::string str;
        std::unique_ptr<expression> left;
        std::unique_ptr<expression> right;
};



int
main()
{   
    auto e2 = expression("-3*7/(9+(1-3)*(5/12))*(1+3+5))");
    e2.print();

    int x = e2.evaluate();  // submit variable function: lookup(variable_name_string) -> value 

    std::cout << e2.str << " = " << x << std::endl;
}


