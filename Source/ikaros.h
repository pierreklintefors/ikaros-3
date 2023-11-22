// Ikaros 3.0

#ifndef IKAROS
#define IKAROS

#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <filesystem>

#define INSTALL_CLASS(class_name)  static InitClass init_##class_name(#class_name, []() { return new class_name(); });

#include "Kernel/utilities.h"
#include "Kernel/dictionary.h"
#include "Kernel/options.h"
#include "Kernel/range.h"
#include "Kernel/expression.h"
#include "Kernel/matrix.h"
#include "Kernel/socket.h"
#include "Kernel/timing.h"
#include "Kernel/socket.h"


#include "Kernel/deprecated.h"

namespace ikaros {

const int run_mode_stop = 0;
const int run_mode_pause = 1;
const int run_mode_step = 2;    // Not used
const int run_mode_play = 3;
const int run_mode_realtime = 4;




using tick_count = long long int;

std::string  validate_identifier(std::string s);

class exception : public std::exception
{
public:
    std::string message;
    exception(std::string msg): message(msg) {};
    const char * what () const throw () { return message.c_str(); }
};

  class fatal_error : public exception 
  {
  public:
        fatal_error(std::string msg) : exception(msg) {}
  };

class Component;
class Module;
class Connection;
class Kernel;

Kernel& kernel();

//
// CIRCULAR BUFFER
//

class CircularBuffer
{
    public:
    std::vector<matrix> buffer_;
    int                 index_;


    CircularBuffer()
    {

    }

    CircularBuffer(matrix &  m,  int size):
        index_(0),
        buffer_(std::vector<matrix>(size))
    {
        for(int i=0; i<size;i++)
        {
            buffer_[i].copy(m);
            buffer_[i].reset(); // FIXME: use other function
        }
    }

    void rotate(matrix &  m)
    {
        buffer_[index_].copy(m);
        index_ = ++index_ % buffer_.size();
        /*
        for(int i=1; i<=buffer_.size(); i++)
            std::cout << i << " " << get(i); 
        std::cout << std::endl;
        */
    }


    matrix & get(int i) // Get output with delay i
    {
        return buffer_[(buffer_.size()+index_-i) % buffer_.size()];
    }

};


//
// PARAMETERS
//

// TODO: add bracket notation to set any type p(x,y) etc


enum parameter_type { no_type=0, int_type, bool_type, float_type, string_type, matrix_type, options_type, rate_type };
static std::vector<std::string> parameter_strings = {"none", "int", "bool", "float", "string", "matrix", "options", "rate"}; // TODO: Add list of strings?

class parameter // FIXME: check all types everywhere; send base rate to WebUI not current value
{
private:
public:
    parameter_type                  type;
    float                           timebase;
    std::string                     default_value;
    std::shared_ptr<int>            int_value;
    std::shared_ptr<float>          float_value;
    std::shared_ptr<matrix>         matrix_value;
    std::shared_ptr<std::string>    string_value;
    std::vector<std::string>        options;

    parameter(): type(no_type)
    {}

    parameter(std::string type_string, std::string default_val, std::string options_string): type(no_type), timebase(1)
    {
        default_value = default_val;
        auto type_index = std::find(parameter_strings.begin(), parameter_strings.end(), type_string);
        if(type_index == parameter_strings.end())
            throw exception("Unkown parameter type: "+type_string+".");
        type = parameter_type(std::distance(parameter_strings.begin(), type_index));
        // Inits shared pointer
        switch(type)
        {
            case int_type: int_value = std::make_shared<int>(0); break;
            case bool_type: int_value = std::make_shared<int>(0); break;
            case float_type: float_value = std::make_shared<float>(0); break;
            case string_type: string_value = std::make_shared<std::string>(""); break;
            case matrix_type: matrix_value = std::make_shared<matrix>(); break;
       case rate_type: float_value = std::make_shared<float>(0); break;

            case options_type:
                int_value = std::make_shared<int>(0);
                options = split(options_string,",");
                break;

            // TODO: bool
            default: break;
        } 
    }



    void operator=(parameter & p) // this shares data with p
    {
        type = p.type;
        default_value = p.default_value;
        int_value = p.int_value;
        float_value = p.float_value;
        matrix_value = p.matrix_value;
        string_value = p.string_value;
        options = p.options;
    }

    int operator=(int v)
    {
        switch(type)
        {
            case int_type: if(int_value) *int_value = v; break;
            case float_type: if(float_value) *float_value = float(v); break;
            case rate_type: if(float_value) *float_value = float(v); break;
            case string_type: if(string_value) *string_value = std::to_string(v); break;
            // TODO: options and bool
            default: break;
        }

        return v;
    }

    float operator=(float v)
    {
        switch(type)
        {
            case int_type: if(int_value) *int_value = int(v); break;
            case float_type: if(float_value) *float_value = v; break;
            case rate_type: if(float_value) *float_value = v; break;
            case string_type: if(string_value) *string_value = std::to_string(v); break;
            case bool_type: if(int_value) *int_value = int(v); break;
            case options_type: if(int_value) *int_value = int(v); break; // FIXME: check range
            default: break;
        }
        //is_set = true;
        return v;
    }

    float operator=(double v) { return operator=(float(v)); };


    std::string operator=(std::string v) // FIXME: Handle exceptions higher up *******  // FIXME: ADD ALL TYPE HERE!!! *** STEP 1 *** 
    {
        switch(type)
        {
            case no_type: type=string_type;  string_value = std::make_shared<std::string>(v); break; 
        
            case int_type: 
                if(int_value) 
                    *int_value = std::stoi(v); 
                else
                    int_value = std::make_shared<int>(std::stoi(v));
                break;

            case bool_type:
                if(int_value)
                {
                    if(is_true(v))
                        *int_value = 1;
                    else
                        *int_value = 0;
                }
                break;

            case options_type:
            {
                auto ix = find(options.begin(), options.end(), v);
                if (ix==options.end())
                    throw exception("option \""+v+"\" not defined."); // FIXME: Should be able to recover if the value came from WEBUI
                int_value = std::make_shared<int>(ix - options.begin());
            }
            break;

            case float_type:
            case rate_type:
                if(float_value) 
                    *float_value = std::stof(v);
                else
                    float_value = std::make_shared<float>(std::stof(v));
                break;


            case string_type:
                if(string_value)
                    *string_value = v;
                else
                    string_value = std::make_shared<std::string>(v);
                    break;
            case matrix_type: 
                if(matrix_value) 
                    *matrix_value = matrix(v);
                else
                    matrix_value = std::make_shared<matrix>(matrix(v));
                break;

            default: 
                break;
        }
        return v;
    }


    operator matrix & ()
    {
        if(type==matrix_type && matrix_value) 
            return *matrix_value;

        throw exception("Not a matrix value.");
    }

    operator std::string()
    {
        switch(type)
        {
            case no_type: throw exception("Uninitialized parameter."); // return "uninitialized_parameter";
            case int_type: if(int_value) return std::to_string(*int_value);            
            // FIXME: get options string and bool
            case float_type: if(float_value) return std::to_string(*float_value);
            case rate_type: if(float_value) return std::to_string(*float_value);
            case bool_type: if(int_value) return std::to_string(*int_value==1);
            case string_type: if(string_value) return *string_value;

            //case matrix_type: return "MATRIX-FIX-ME"; //FIXME: Get matrix string - add to matrix class to also be used by print *****
            default: ;
        }
        throw exception("Type conversion error for parameter.");
    }

   operator int()
    {
        switch(type)
        {
            case no_type: throw exception("Uninitialized_parameter.");
            case int_type: if(int_value) return *int_value;
            case options_type: if(int_value) return *int_value;
            case float_type: if(float_value) return *float_value;
            case rate_type: if(float_value) return *float_value;    // FIXME: Take care of time base
            case string_type: if(string_value) return stof(*string_value); // FIXME: Check that it is a number
            case matrix_type: throw exception("Could not convert matrix to float"); // FIXME check 1x1 matrix
            default: ;
        }
        throw exception("Type conversion error for  parameter");
    }

   operator float()
    {
        switch(type)
        {
            case no_type: throw exception("uninitialized_parameter.");
            case int_type: if(int_value) return *int_value;
            case options_type: if(int_value) return *int_value;
            case float_type: if(float_value) return *float_value;
            case rate_type: if(float_value) return *float_value;    // FIXME: Take care of time base here *******************
            case string_type: if(string_value) return stof(*string_value); // FIXME: Check that it is a number
            //case matrix_type: throw exception("Could not convert matrix to float"); // FIXME check 1x1 matrix
            default: ;
        }
    throw exception("Type conversion error for  parameter.");
    }

    std::string json()
    {
        switch(type)
        {
            case int_type:      if(int_value)       return "[["+std::to_string(*int_value)+"]]";
            case options_type:  if(int_value)       return "[["+std::to_string(*int_value)+"]]";
            case float_type:    if(float_value)     return "[["+std::to_string(*float_value)+"]]";
            case rate_type:     if(float_value)     return "[["+std::to_string(*float_value)+"]]";  // FIXME: print timebase as well??? Probably not here.
            case bool_type:     if(int_value)       return (int_value ? "true" : "false");
            case string_type:   if(string_value)    return "\""+*string_value+"\"";
            case matrix_type:   if(matrix_value)    return matrix_value->json();
            default:            return "";
        }
    }

    friend std::ostream& operator<<(std::ostream& os, parameter p)
    {
        switch(p.type)
        {
            case int_type:      if(p.int_value) os << *p.int_value; break;
            case options_type:  if(p.int_value) os << *p.int_value; break;
            case float_type:    if(p.float_value) os <<  *p.float_value; break;
            case rate_type:    if(p.float_value) os <<  *p.float_value; break;  // FIXME: print timebase as well???
            case bool_type:     if(p.int_value) os <<  (*p.int_value==1? "true" : "false"); break;
            case string_type:   if(p.string_value) os <<  *p.string_value; break;
            case matrix_type:   if(p.matrix_value) os <<  *p.matrix_value; break;
            default:            os << "unkown-parameter-type"; break;
        }

        return os;
    }
};

//
// COMPONENT
//

class Component
{
public:
    Component *     parent_;
    dictionary      info_;      // FIXME: Make shared
    std::string     name_;

    Component();

    virtual ~Component() {};

    void print()
    {
        std::cout << "Component: " << info_["name"]  << '\n';
    }

    void AddInput(dictionary parameters);
    void AddOutput(dictionary parameters);
    void AddParameter(dictionary parameters);
    void SetParameter(std::string name, std::string value);


    bool BindParameter(parameter & p,  std::string & name) // Handle parameter sharing
    {
        std::string bind_to = GetValue(name+".bind");
        if(bind_to.empty())
            return false;
        else
            return LookupParameter(p, bind_to);
    }



    bool ResolveParameter(parameter & p,  std::string & name,   std::string & default_value)
    {
        // Look for binding
        std::string bind_to = GetBind(name);
        if(!bind_to.empty())
        {
          if(LookupParameter(p, bind_to))
                return true;
        }

        // Lookup normal value in current component-context
        std::string v = GetValue(name);
        if(!v.empty())
        {
            SetParameter(name, Evaluate(v));
            return true;
        }

        SetParameter(name, Evaluate(default_value));
        return true; // IF refault value ******
    }



    void    Bind(parameter & p, std::string n);   // Bind to parameter in global parameter table
    void    Bind(matrix & m, std::string n); // Bind to input or output in global parameter table, or matrix parameter

    virtual void Tick() {}
    virtual void Init() {}

    std::string JSONString();

    std::string GetValue(const std::string & name)    // Get value of a attribute/variable in the context of this component
    {        
        if(dictionary(info_["attributes"]).contains(name))
            return Evaluate(info_["attributes"][name]);
        if(parent_)
            return parent_->GetValue(name);
        return "";
    }


  std::string GetBind(const std::string & name)
    {
        //std::cout << "Getting binding: " << name << std::endl;
        if(dictionary(info_["attributes"]).contains(name))
            return ""; // Value set in attribute - do not bind
        if(dictionary(info_["attributes"]).contains(name+".bind"))
            return info_["attributes"][name+".bind"];
        if(parent_)
            return parent_->GetBind(name);
        return "";
    }

    std::string SubstituteVariables(const std::string & s)
    {
        std::string var; 
        std::string sep;
        for(auto c : split(s, "."))
        {
            if(c[0] == '@')
                var += sep + GetValue(c.substr(1));
            else
                var += sep + c;
            sep = ".";
        }
        return var;
    }

    Component * GetComponent(const std::string & s); // Get component; sensitive to variables and indirection
 

    std::string Evaluate(const std::string & s)     // Evaluate an expression in the current context - as string?
    {
        if(s.empty())

    if(!expression::is_expression(s))
    {
        if(s[0]=='@') // Handle indirection (unless expresson)
        {
            if(s.find('.')==std::string::npos)
                return GetValue(s.substr(1));
            
            std::string component_path = rhead(s.substr(1), '.');
            std::string variable_name = SubstituteVariables(rtail(s, '.'));
            return GetComponent(component_path)->GetValue(variable_name);
        }
        else
            return s;
    }

        // Handle mathematical expression

        if(!expression::is_expression(s))
            return s;

         expression e = expression(s);
            std::map<std::string, std::string> vars;
            for(auto v : e.variables())
            {
                std::string value = Evaluate(v); // was GetValue
                if(value.empty())
                    throw exception("Variable \""+v+"\" not defined.");
                vars[v] = value;
        }
        return std::to_string(expression(s).evaluate(vars));
    }


    bool LookupParameter(parameter & p, const std::string & name);



    //std::string Lookup(const std::string & name) const;
    
    int EvaluateIntExpression(std::string & s)
    {
        expression e = expression(s);
        std::map<std::string, std::string> vars;
        //for(auto v : e.variables())
        //    vars[v] = Lookup(v); // FIXME: Use Evaluate() instead later **************
        return expression(s).evaluate(vars);
    }

    std::vector<int> EvaluateSizeList(std::string & s) // return list of size from size list in string
    {
        std::vector<int> shape;
        for(std::string e : split(s, ","))
        shape.push_back(EvaluateIntExpression(e));
        return shape;
//        throw exception("Could not parse size list string");
    }

    std::vector<int> EvaluateSize(std::string & s) // Evaluate size/shape string
    {
         if(ends_with(s, ".size")) // special case: use shape function on input
        {
            auto & x = rsplit(s, ".", 1); // FIXME: rhead ???
            matrix m;
            Bind(m, x.at(0));
            return m.shape(); 
        }

        return EvaluateSizeList(s); // default case: parse size list
    }


bool InputsReady(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections);

void SetSourceRanges(const std::string & name, std::vector<Connection *> & ingoing_connections);
void SetInputSize_Flat(const std::string & name,  std::vector<Connection *> & ingoing_connections, bool add_labels);
void SetInputSize_Index(const std::string & name, std::vector<Connection *> & ingoing_connections);
void SetInputSize(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections);

    virtual int SetSizes(std::map<std::string,std::vector<Connection *>> & ingoing_connections);
};

typedef std::function<Module *()> ModuleCreator;


class Group : public Component
{
};


//
// MODULE
//


class Module : public Component
{
public:
    Module();
    ~Module() {}    
};

class Connection
{
    public:
    std::string source;             // FIXME: Add undescore to names ****
    range       source_range;
    std::string target;
    range       target_range;
    range       delay_range_;
    std::string alias_;
    bool        flatten_;

    Connection(std::string s, std::string t, range & delay_range, std::string alias="")
    {
        source = head(s, '[');
        source_range = range(tail(s, '[', true)); // FIXME: CHECK NEW TAIL FUNCTION WITHOUT SEPARATOR
        target = head(t, '[');
        target_range = range(tail(t, '[', true));
        delay_range_ = delay_range;
        flatten_ = false;
        alias_ = alias;
    }


    void
    Print()
    {
        std::cout << "\t" << source <<  delay_range_.curly() <<  std::string(source_range) << " => " << target  << std::string(target_range);
        if(!alias_.empty())
            std::cout << " \"" << alias_ << "\"";
        std::cout << '\n'; 
    }
};


//
// CLASS
//


class Class
{
public:
    ModuleCreator   module_creator;
    std::string     name;
    std::string     path;
    std::map<std::string, std::string>  parameters;

    Class() {};
    Class(std::string n, std::string p) : module_creator(nullptr), name(n), path(p)
    {
    };

    void print()
    {
        std::cout << name << ": " << path  << '\n';
    }
};


//
// KERNEL
//

class Kernel
{
public:

    std::string                             webui_dir;

    std::map<std::string, Class>            classes;
    std::map<std::string, Component *>      components;
    std::vector<Connection>                 connections;
    //std::map<std::string, matrix>           buffers;         // Use IO-structure later: or only matrix?
    //std::map<std::string, matrix>           buffers;        // Use IO-structure later: Output
    std::map<std::string, matrix>           buffers;        // IO-structure    
    std::map<std::string, int>              max_delays;     // Maximum delay needed for each output
    std::map<std::string, CircularBuffer>   circular_buffers;        // Circular circular_buffers for delayed buffers
    std::map<std::string, parameter>        parameters;

    bool                first_request;
    long                master_id;
    bool                is_running;
    std::atomic<bool>   tick_is_running;
    std::atomic<bool>   sending_ui_data;
    std::atomic<bool>   handling_request;
    int                 run_mode;

    dictionary                          current_component_info; // From class or group
    dictionary                          current_module_info;

    float                               idle_time;              // TODO: change to duration<float>             
    float                               time_usage;             // TODO: change to duration<float>

    int                                 cpu_cores;
    double                              cpu_usage;
    double                              last_cpu;
    float                               last_cpu_time;

    Timer                               timer;                  // Main timer
    
    bool                                start;          // Start automatically                   

    // Timing parameters and functions

    //bool              real_time;
    double              tick_duration;  // Desired actual or simulated duration for each tick

    tick_count          tick;

    tick_count          stop_after;
    double              lag;            // Lag of a tick in real-time mode
    double              lag_min;     // Largest negative lag
    double              lag_max;     // Largest positive lag
    double              lag_sum;     // Sum |lag|

    ServerSocket *      socket;
    long                session_id;

    tick_count GetTick() { return tick; }
    double GetTickDuration() { return tick_duration; } // Time for each tick in seconds (s)
    double GetTime() { return (run_mode == run_mode_realtime) ? GetRealTime() : static_cast<double>(tick)*tick_duration; }   // Time since start (in real time or simulated (tick) time dending on mode)
    double GetRealTime() { return (run_mode == run_mode_realtime) ? timer.GetTime() : static_cast<double>(tick)*tick_duration; } 
    double GetLag() { return (run_mode == run_mode_realtime) ? static_cast<double>(tick)*tick_duration - timer.GetTime() : 0; }

    std::vector<std::string>            log;

    bool Terminate()
    {
        return stop_after!= 0 &&  tick >= stop_after;
    }

    void ScanClasses(std::string path)
    {
        int i=0;
        for(auto& p: std::filesystem::recursive_directory_iterator(path))
            if(std::string(p.path().extension())==".ikc")
            {
                std::string name = p.path().stem();
                classes[name] = Class(name, p.path());
            }
    }


    void ListClasses()
    {
        std::cout << "\nClasses:" << std::endl;
        for(auto & c : classes)
            c.second.print();
    }


    void ResolveParameter(parameter & p,  std::string & name,  std::string & default_value)
    {
        std::size_t i = name.rfind(".");
        if(i == std::string::npos)
            return;

         auto c = components.at(name.substr(0, i));
        std::string parameter_name = name.substr(i+1, name.size());
        c->ResolveParameter(p, parameter_name, default_value);
    }


    void ResolveParameters() // Find and evaluate value or default
    {
        for (auto p=parameters.rbegin(); p!=parameters.rend(); p++) // Reverse order equals outside in in groups
        {
            std::size_t i = p->first.rfind(".");
            // Find component and parameter_name and resolve
            i = p->first.rfind(".");
            if(i != std::string::npos)
            {
                auto c = components.at(p->first.substr(0, i));
                std::string parameter_name = p->first.substr(i+1, p->first.size());
                //std::cout << "Resolve: " << parameter_name << std::endl;
                c->ResolveParameter(p->second, parameter_name, p->second.default_value);
            }
        }
    }


    void CalculateSizes()
    {
        // Copy the table of all components
        std::map<std::string, Component *> init_components = components;

        // Build input table

        std::map<std::string,std::vector<Connection *>> ingoing_connections; 
    for(auto & c : connections)
        ingoing_connections[c.target].push_back(&c);

        // Propagate output size to buffers as long as at least one module sets one of it output sizes

        int iteration_counter = 0;
        bool progress = true;
        while(progress)
        {
     
            //progress = false; // FIXME: turn on again when progress reporting is implemented

            // Try to set input and output sizes and remove from list if complete

            auto it = init_components.begin();
            while( it!=init_components.end())
                if(it->second->SetSizes(ingoing_connections) == 1) // 1 = completed
                {
                    it = init_components.erase(it);
                    //progress = true;
                }
                else
                    it++;
            if(iteration_counter++ > 4) // FIXME: REAL CONDITION LATER
                return;
        }
    }


    void CalculateDelays()
    {
        for(auto & c : connections)
        {
            if(!max_delays.count(c.source))
                max_delays[c.source] = 0;
            if(c.delay_range_.extent()[0] > max_delays[c.source])
            {
                //int xxx = c.delay_range_.extent()[0];
                max_delays[c.source] = c.delay_range_.extent()[0];
            }
        }

        std::cout << "\nDelays:" << std::endl;
        for(auto & o : buffers)
            std::cout  << "\t" << o.first <<": " << max_delays[o.first] << std::endl;
    }


    void InitCircularBuffers()
    {
        std::cout << "\nInitCircularBuffers:" << std::endl;
        for(auto it : max_delays)
        {
            if(it.second <= 1)
                continue;
            std::cout << "\t" << it.first << std::endl;
            circular_buffers.emplace(it.first, CircularBuffer(buffers[it.first], it.second)); // FIXME: Change to initialization list in C++20
        }

        /*
        std::cout << "\nBuffers:" << std::endl;
        for(auto it : circular_buffers)
            std::cout << "\t" << it.first << "  " << it.second.buffer_.size() <<  std::endl;
        */
    }


    void RotateBuffers()
    {
        //std::cout << "Rotate circular_buffers"   << std::endl;
        for(auto & it : circular_buffers)
            it.second.rotate(buffers[it.first]);
    }


    void ListComponents()
    {
        std::cout << "\nComponents:" << std::endl;
        for(auto & m : components)
            m.second->print();
    }


    void ListConnections()
    {
        std::cout << "\nConnections:" << std::endl;
        for(auto & c : connections)
            c.Print();
    }


    void ListInputs()
    {
        std::cout << "\nInputs:" << std::endl;
        for(auto & i : buffers)
            std::cout << "\t" << i.first <<  i.second.shape() << std::endl;
    }


   void ListOutputs()
    {
        std::cout << "\nOutputs:" << std::endl;
        for(auto & o : buffers)
            std::cout  << "\t" << o.first << o.second.shape() << std::endl;
    }


    void ListBuffers()
    {
        std::cout << "\nBuffers:" << std::endl;
        for(auto & i : buffers)
            std::cout << "\t" << i.first <<  i.second.shape() << std::endl;
    }


 void ListCircularBuffers()
    {
        std::cout << "\nCircularBuffers:" << std::endl;
        for(auto & i : circular_buffers)
            std::cout << "\t" << i.first <<  " " << i.second.buffer_.size() << std::endl;
    }


   void ListParameters()
    {
        std::cout << "\nParameters:" << std::endl;
        for(auto & p : parameters)
            std::cout  << "\t" << p.first << ": " << p.second << std::endl;
    }


    void PrintLog()
    {
        for(auto & s : log)
            std::cout << "IKAROS: " << s << std::endl;
        log.clear();
    }


    Kernel():
        tick(0),
        is_running(false),
        tick_is_running(false),
        time_usage(0),
        idle_time(0),
        stop_after(0),
        tick_duration(0.01), // 10 ms
        webui_dir("Source/WebUI/") // FIXME: get from somewhere else
    {

            ScanClasses("Source/Modules");
    }

    // Functions for creating the network

    void AddInput(std::string name, dictionary parameters=dictionary())
    {
        buffers[name] = matrix();
    }

    void AddOutput(std::string name, dictionary parameters=dictionary())
    {
        buffers[name] = matrix();
      }

    void AddParameter(std::string name, dictionary params=dictionary())
    {
        std::string type_string = params["type"];
        std::string default_value = params["default"];
        std::string options = params["options"];
        parameter p(type_string, default_value, options);
        parameters.emplace(name, p);
    }

    void SetParameter(std::string name, std::string value)
    {
        if(parameters.count(name))
        {
            parameters[name] = value;
        }
    }


    void AddGroup(std::string name, dictionary info=dictionary())
    {
        if(components.count(name)> 0)
            throw exception("Module or group with this name already exists.");


        current_component_info = info;
        current_component_info["name"] = name;
        current_component_info["is_group"] = "true";
        components[name] = new Group(); // Implicit argument passing as for components
    }


    void AddModule(std::string name, dictionary info=dictionary())
    {
        if(components.count(name)> 0)
            throw exception("Module or group with this name already exists.");
                info["is_group"] = "false"; // FIXME: Allow boolean Value
        std::string classname = info["attributes"]["class"];
        current_component_info = dictionary(classes[classname].path, true);
        current_component_info["name"] = name;
        current_module_info = info;
        current_module_info["name"] = name;
        if(!classes.count(classname))
            throw exception("Class \""+classname+"\" does not exist.");
        if(classes[classname].module_creator == nullptr)
            throw exception("Class \""+classname+"\" has no installed code. Check that it is included in CMakeLists.txt."); // TODO: Check that this works for classes that are allowed to have no code
        components[name] = classes[classname].module_creator(); // throws bad function call if not defined
    }


    void AddConnection(std::string souce, std::string target, std::string delay_range, std::string alias)
    {
        if(delay_range.empty())
            delay_range = "[1]";
        else if(delay_range[0] != '[')
            delay_range = "["+delay_range+"]";
        range r(delay_range);
        connections.push_back(Connection(souce, target, r, alias));
    }

    // Functions for reading from file

    void ParseGroupFromXML(XMLElement * xml, std::string path="")
    {
        std::string name = validate_identifier((*xml)["name"]);
        if(!path.empty())
            name = path+"."+name;
        AddGroup(name, dictionary(xml, true)); // FIME: Should only build dictionary once for whole ikg-file

        for (XMLElement * xml_node = xml->GetContentElement(); xml_node != nullptr; xml_node = xml_node->GetNextElement())
        {
            if(xml_node->IsElement("module"))
            {
                AddModule(name+"."+validate_identifier((*xml_node)["name"]), dictionary(xml_node)); 
            }
            else if(xml_node->IsElement("group"))
            {
                ParseGroupFromXML(xml_node, name);
            }
            else if(xml_node->IsElement("connection"))
            {
                AddConnection(name+"."+(*xml_node)["source"], name+"."+(*xml_node)["target"], (*xml_node)["delay"], (*xml_node)["alias"]);
            }
        }
    }
 

    void AllocateInputs()
    {
        // Allocate memory for all buffers and delay lines 
    }


    void InitComponents()
    {
        // Call Init for all modules (after CalcalateSizes and Allocate)
        for(auto & c : components)
            try
            {
                c.second->Init();
            }
            catch(const fatal_error& e)
            {
                log.push_back("Fatal error. Init failed for \""+c.second->name_+"\": "+std::string(e.what())); // FIXME: set end of execution
            }
            catch(const std::exception& e)
            {
                log.push_back("Init failed for \""+c.second->name_+"\": "+std::string(e.what()));
            }
    }

    // Start up the kernel for the first time

    void Start(std::vector<std::string> files, options & opts) 
    {
        LoadFiles(files, opts);
        long port = opts.get_long("webui_port");  // FIXME: Port cannot be set in IKG file; should it?
        InitSocket(port);
    }
        //k.ListClasses();
        //k.ListBuffers();
        //k.ListCircularBuffers();
        //k.ListInputs();
        //k.ListOutputs();

    void LoadFiles(std::vector<std::string> files, options & opts)
    {
            for(auto & filename: files)
                try
                {
                    XMLDocument * xmlDoc = new XMLDocument(filename.c_str());
                    if(xmlDoc->xml == nullptr)
                        throw exception("File \""+filename+"\" not found");

                    if(xmlDoc->xml->name != std::string("group"))
                         throw exception("Group element not found in \""+filename+"\"."); // FIXME: request exit

                    // Add command line options to top element of XML before parsing // FIXME: Clean up

                    for(auto & x : opts.d)
                        if(!xmlDoc->xml->GetAttribute(x.first.c_str())) 
                            ((XMLElement *)(xmlDoc->xml))->attributes = new XMLAttribute(create_string(x.first.c_str()), create_string(x.second.c_str()),0,((XMLElement *)(xmlDoc->xml))->attributes);          
                    ParseGroupFromXML(xmlDoc->xml);

                    // Set basic parameters from loaded file

                      dictionary d = components.begin()->second->info_["attributes"]; // Get top group


                    start = is_true(d["start"]);
                    stop_after = d["stop"];
                    tick_duration = d["tick_duration"];
                    if(is_true(d["real_time"]))
                        run_mode = run_mode_realtime;
                    session_id = std::time(nullptr);

                }
                catch(const std::exception& e)
                {
                    log.push_back(e.what());
                }
    }


    void SortNetwork()
    {
        // Resolve paths
        // Sort Components and Connections => Thread Groups and Partially Ordered Events (Tick & Propagate)
    }


    void Propagate()
    {
         for(auto & c : connections)
        {
            if(c.delay_range_.empty() || c.delay_range_.is_delay_0())
            {
                // Do not handle here yet // FIXME: !!!!
            }

            else if(c.delay_range_.empty() || c.delay_range_.is_delay_1())
                buffers[c.target].copy(buffers[c.source], c.target_range, c.source_range);

            else if(c.flatten_) // Copy flattened delayed values
            {
                matrix target = buffers[c.target];
                int target_offset = c.target_range.a_[0];
                for(int i=c.delay_range_.a_[0]; i<c.delay_range_.b_[0]; i++)  // FIXME: assuming continous range (inc==1)
                {   
                    matrix s = circular_buffers[c.source].get(i);

                    for(auto ix=c.source_range; ix.more(); ix++)
                    {
                        int source_index = s.compute_index(ix.index());
                        target[target_offset++] = (*(s.data_))[source_index];
                    }
                }
            }

            else if(c.delay_range_.a_[0]+1 == c.delay_range_.b_[0]) // Copy indexed delayed value with single delay
            {
                matrix s = circular_buffers[c.source].get(c.delay_range_.a_[0]);
                buffers[c.target].copy(s, c.target_range, c.source_range);
            }

            else // Copy indexed delayed values with more than one element
            {
                for(int i=c.delay_range_.a_[0]; i<c.delay_range_.b_[0]; i++)  // FIXME: assuming continous range (inc==1)
                {   
                    matrix s = circular_buffers[c.source].get(i);
                    int target_ix = i - c.delay_range_.a_[0]; // trim!
                    range tr = c.target_range.tail();
                    matrix t = buffers[c.target][target_ix];
                    t.copy(s, tr, c.source_range);

                    std::cout << std::endl;
                }
            }
        }
    }


    std::string JSONString();

    void InitSocket(int port)
{
    try
    {
        
        socket =  new ServerSocket(port);
    }
    catch (const SocketException& e)
    {
        throw fatal_error("Ikaros is unable to start a webserver on port "+std::to_string(port)+". Make sure no other ikaros process is running and try again.");
    }
}

    void Pause();

    void DoStop(std::string uri, std::string args);
    void DoPause(std::string uri, std::string args);
    void DoStep(std::string uri, std::string args);
    void DoPlay(std::string uri, std::string args);
    void DoRealtime(std::string uri, std::string args);
    void DoCommand(std::string uri, std::string args);
    void DoControl(std::string uri, std::string args);
    void DoSendNetwork(std::string uri, std::string args);
    void DoSendData(std::string uri, std::string args);
    void DoUpdate(std::string uri, std::string args);
    void DoGetLog(std::string uri, std::string args);
    void DoSendClasses(std::string uri, std::string args);
    void DoSendFile(std::string file);
    void DoSendError();
 

    void HandleHTTPRequest();
    void HandleHTTPThread();
    
    std::thread *   httpThread;
    static void *   StartHTTPThread(Kernel * k);
    
    void Tick();
    void Run();
};



//
// INITIALIZATION CLASS
//

class InitClass
{
public:
    InitClass(const char * name, ModuleCreator mc)
    {
        kernel().classes.at(name).module_creator = mc;
    }
};

}; // namespace ikaros
#endif

