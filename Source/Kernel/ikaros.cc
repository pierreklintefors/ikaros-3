// Ikaros 3.0

#include <iostream> 

#include "ikaros.h"

using namespace ikaros;
using namespace std::chrono;
using namespace std::literals;

namespace ikaros
{
    std::string  validate_identifier(std::string s)
    {
        static std::string legal = "_0123456789aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ";
        if(s.empty())
            throw exception("Identifier cannot be empty string");
        if('0' <= s[0] && s[0] <= '9')
            throw exception("Identifier cannot start with a number: "+s);
        for(auto c : s)
            if(legal.find(c) == std::string::npos)
                throw exception("Illegal character in identifier: "+s);
        return s;
    }

    long new_session_id()
    {
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }


// CircularBuffer

    CircularBuffer::CircularBuffer(matrix &  m,  int size):
        index_(0),
        buffer_(std::vector<matrix>(size))
    {
        for(int i=0; i<size;i++)
        {
            buffer_[i].copy(m);
            buffer_[i].reset(); // FIXME: use other function
        }
    }

    void 
    CircularBuffer::rotate(matrix &  m)
    {
        buffer_[index_].copy(m);
        index_ = ++index_ % buffer_.size();
    }

    matrix & 
    CircularBuffer::get(int i) // Get output with delay i
    {
        return buffer_[(buffer_.size()+index_-i) % buffer_.size()];
    }

// Parameter

    parameter::parameter(dictionary info):
        info_(info), type(no_type)
    {
        std::string type_string = info_["type"];

        if(type_string=="float")
                type_string = "double";
        if(type_string=="int")
            type_string = "double";

        //default_value = std::string(info_["default"]);   // FIXME: use value in dictionary directly later
        //std::string options = info_["options"];

        auto type_index = std::find(parameter_strings.begin(), parameter_strings.end(), type_string);
        if(type_index == parameter_strings.end())
            throw exception("Unkown parameter type: "+type_string+".");

        type = parameter_type(std::distance(parameter_strings.begin(), type_index));

        // Init shared pointers: Use only for matrices in the future *********** check parameter type in info_ dictionary and use these instad of separate shared pointers
        switch(type)
        {
            case double_type: double_value = std::make_shared<double>(0); break;
            case bool_type: bool_value = std::make_shared<bool>(false); break;
            case string_type: string_value = std::make_shared<std::string>(""); break;
            case matrix_type: matrix_value = std::make_shared<matrix>(); break;
            case rate_type: double_value = std::make_shared<double>(0); break;
            case options_type: double_value = std::make_shared<double>(0); break;
            //options = split(info_["options"],","); // FIXME: put in dict later, but consider potential XML saving problems; or split every time it is needed
            
            default: break;
        } 
    }

    void 
    parameter::operator=(parameter & p) // this shares data with p
    {
        info_ = p.info_;
        type = p.type;
        double_value = p.double_value;
        bool_value = p.bool_value;
        matrix_value = p.matrix_value;
        string_value = p.string_value;
    }

    int 
    parameter::operator=(int v)
    {
        if(double_value) 
            *double_value = double(v);
        else if(bool_value) 
            *bool_value = bool(v);
        else if(string_value) 
            *string_value = std::to_string(v);
        return v;
    }

    double 
    parameter::operator=(double v)
    {
       if(double_value) 
            *double_value = v;
        else if(bool_value) 
            *bool_value = bool(v);
        else if(string_value) 
            *string_value = std::to_string(v);
        return v;
    }


    std::string 
    parameter::operator=(std::string v)
    {
        if(type==options_type) // FIXME: DO STUFF
            ;
        else if(double_value) 
            *double_value = stod(v);
        else if(bool_value) 
            *bool_value = is_true(v);
        else if(string_value) 
            *string_value = v;
        else if(matrix_value)
            *matrix_value = v;
        return v;
    }


    parameter::operator matrix & ()
    {
        if(matrix_value) 
            return *matrix_value;
        else
            throw exception("Not a matrix value.");
    }


    parameter::operator std::string()
    {
        switch(type)
        {
            case no_type: throw exception("Uninitialized or unbound parameter.");
            case double_type: if(double_value) return std::to_string(*double_value);
            case bool_type: if(bool_value) return (*bool_value? "true" : "false");
            case rate_type: if(double_value) return std::to_string(*double_value);
            case string_type: if(string_value) return *string_value;
            case matrix_type: return matrix_value->json();
            case options_type: return "**OPTIONS**";
            default:  throw exception("Type conversion error for parameter.");
        }
    }


    parameter::operator double()
    {
        if(double_value) 
            return *double_value;
        else if(bool_value) 
            return *bool_value;
        else if(string_value) 
            return stod(*string_value); // FIXME: may fail ************
        else if(matrix_value)
            return 0;// FIXME check 1x1 matrix ************
        else
            throw exception("Type conversion error. Parameter does not have a type. Bind?");
    }



/*
    parameter::operator bool()
    {
        if(double_value) 
            return (*double_value!= 0);
        else if(bool_value) 
            return *bool_value;
        else if(string_value) 
            return is_true(*string_value);
        else if(matrix_value)
            return !(*matrix_value).empty();
        else
            throw exception("Type conversion error for parameter.");
    }
*/
/*
    parameter::operator int()
    {
        switch(type)
        {
            case no_type: throw exception("Uninitialized_parameter.");
            case int_type: if(int_value) return *int_value;
            case options_type: if(int_value) return *int_value;
            case double_type: if(double_value) return *double_value;
            case rate_type: if(double_value) return *double_value;    // FIXME: Take care of time base
            case string_type: if(string_value) return stof(*string_value); // FIXME: Check that it is a number
            case matrix_type: throw exception("Could not convert matrix to float"); // FIXME check 1x1 matrix
            default: ;
        }
        throw exception("Type conversion error for  parameter");
    }
*/

    int
    parameter::as_int()
    {
        switch(type)
        {
            case no_type: throw exception("Uninitialized_parameter.");
            case double_type: if(double_value) return *double_value;
            case rate_type: if(double_value) return *double_value;    // FIXME: Take care of time base
            case bool_type: if(bool_value) return *bool_value;
            case options_type: if(double_value) return *double_value;
            case string_type: if(string_value) return stoi(*string_value); // FIXME: Check that it is a number
            case matrix_type: throw exception("Could not convert matrix to int"); // FIXME check 1x1 matrix
            default: ;
        }
        throw exception("Type conversion error for  parameter");
    }


    const char* 
    parameter::c_str() const noexcept
    {
        if(string_value)
            return string_value->c_str();
        else
            return NULL;
    }


    std::string 
    parameter::json()
    {
        switch(type)
        {
            case double_type:    if(double_value)   return "[["+std::to_string(*double_value)+"]]";   // FIXME: remove if statements and use exception handling
            case bool_type: if(bool_value)          return (*bool_value? "[[true]]" : "[[false]]");
            case rate_type:     if(double_value)    return "[["+std::to_string(*double_value)+"]]";
            case string_type:   if(string_value)    return "\""+*string_value+"\"";
            case matrix_type:   if(matrix_value)    return matrix_value->json();
            default:            throw exception("Cannot convert parameter to string");
        }
    }


    std::ostream& operator<<(std::ostream& os, parameter p)
    {
        switch(p.type)
        {
            case options_type:  os << "OPTIONS"; break; // FIXME: *********** OPTIONS
            case double_type:    if(p.double_value) os <<  *p.double_value; break;
            case bool_type:     if(p.double_value) os <<  (*p.double_value == 0 ? "false" : "true"); break;
            case rate_type:    if(p.double_value) os <<  *p.double_value; break; 
            case string_type:   if(p.string_value) os <<  *p.string_value; break;
            case matrix_type:   if(p.matrix_value) os <<  *p.matrix_value; break;
            default:            throw exception("Cannot convert parameter to string for printing");
        }

        return os;
    }


// Component

    void 
    Component::print()
    {
        std::cout << "Component: " << info_["name"]  << '\n';
    }

        void 
        Component::info()
        {
            std::cout << "Component: " << info_["name"]  << '\n';
            std::cout << "Path: " << path_  << '\n';
            std::cout << "Path: " << info_  << '\n';
        }

    bool 
    Component::BindParameter(parameter & p,  std::string & name) // Handle parameter sharing
    {
        std::string bind_to = GetValue(name+".bind");
        if(bind_to.empty())
            return false;
        else
            return LookupParameter(p, bind_to);
    }





    bool 
    Component::ResolveParameter(parameter & p,  std::string & name)
    {
        try
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
            if(!v.empty())  // ****************** this does dot work for string that are allowed to be empty
            {
                std::string val = v; // Evaluate(v, p.type == string_type); ***********
                if(!val.empty())
                {
                    SetParameter(name, val);
                    return true;
                }
            }

            SetParameter(name, p.info_["default"]); //******************* Evaluate(default_value, p.type == string_type)
            return true; // IF refault value ******
        }
        catch(exception & e)
        {
            //std::cout << e.what() << std::endl; 
            Notify(msg_fatal_error, e.message);
        }
        catch(std::exception & e)
        {
            Notify(msg_fatal_error, "ERROR: Could not resolve parameter \""s +name + "\" .");  
        }
        return false;
    }



    bool 
    Component::KeyExists(const std::string & key)
    {        
        if(info_.contains(key))
            return true;
        if(parent_)
            return parent_->KeyExists(key);
        else
            return false;
    }


    std::string 
    Component::LookupKey(const std::string & key)
    {        
        if(info_.contains(key))
            return info_[key];
        if(parent_)
            return parent_->LookupKey(key);
        else
            throw exception("Name not found"); // throw not_found_exception instead
    }



    static std::string
    exchange_before_dot(const std::string& original, const std::string& replacement)
    {
        size_t pos = original.find('.');
        if (pos == std::string::npos) // No dot found, replace the whole string
            return replacement;
     else  // Replace up to the first dot
            return replacement + original.substr(pos);
    }



    static std::string
    before_dot(const std::string& original)
    {
        size_t pos = original.find('.');
        if (pos == std::string::npos)
            return original;
     else 
            return original.substr(0,pos);
    }


//
// GetValue
//
// Get value of a key/variable in the context of this component (ignores current parameter values)
// Throws and eception if value cannot be found
// Does not handle default values - this is done by parameters

    std::string 
    Component::GetValue(const std::string & path) 
    {     
        if(path.empty())
            throw exception("Name not found"); // throw not_found_exception instead

        if(path[0]=='@')
            return GetValue(exchange_before_dot(path, LookupKey( before_dot(path).substr(1))));
        
        if(path[0]=='.')
            return kernel().components.begin()->second->GetValue(path.substr(1)); // Absolute path // FIXME: Scary - main_group -

        size_t pos = path.find('.');
        if(pos != std::string::npos)
        {
            std::string head = path.substr(0, pos);
            std::string tail = path.substr(pos + 1);

            if(head[0]=='@')
                head = LookupKey(head.substr(1));

            std::string local_path = path_+'.'+head;
            if(kernel().components.count(local_path))
                return kernel().components[local_path]->GetValue(tail);
            else if(std::string(parent_->info_["name"]) == head)
                return parent_->GetValue(tail);
            else
                throw exception("Name not found"); // throw not_found_exception instead 
        }

        std::string value = LookupKey(path);
        if(value.find('@') != std::string::npos && value.find('.') != std::string::npos) // A new indirect 'path' - start over
            return GetValue(value);
        else if(value.find('@') != std::string::npos) // A new indirect 'key' - start over
            return GetValue(value.substr(1));
        else
            return value;
    }



    std::string 
    Component::GetBind(const std::string & name)
    {
        if(info_.contains(name))
            return ""; // Value set in attribute - do not bind
        if(info_.contains(name+".bind")) 
            return info_[name+".bind"];
        if(parent_)
            return parent_->GetBind(name);
        return "";
    }



    std::string 
    Component::SubstituteVariables(const std::string & s)
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


    void Component::Bind(parameter & p, std::string name)
    {
        Kernel & k = kernel();
        std::string pname = path_+"."+name;
        if(k.parameters.count(pname))
            p = kernel().parameters[pname];
        else
            throw exception("Cannot bind to \""+name+"\"");
    };


    void Component::Bind(matrix & m, std::string n) // Bind input, output or parameter
    {
        std::string name = path_+"."+n;
        try
        {
            Kernel & k = kernel();
            if(k.buffers.count(name))
                m = k.buffers[name];
            else if(k.buffers.count(name))
                m = k.buffers[name];
            else if(k.parameters.count(name))
                m = (matrix &)(k.parameters[name]);
            else if(k.parameters.count(name))
                throw exception("Cannot bind to attribute \""+name+"\". Define it as a parameter!");
            else
                throw exception("Input, output or parameter named \""+name+"\" does not exist");
        }
        catch(exception e)
        {
            throw exception("Bind:\""+name+"\" failed. "+e.message);
        }
    }

    void Component::AddInput(dictionary parameters)
    {
        std::string input_name = path_+"."+validate_identifier(parameters["name"]);
        kernel().AddInput(input_name, parameters);
    }

    void Component::AddOutput(dictionary parameters)
    {
        std::string output_name = path_+"."+validate_identifier(parameters["name"]);
        kernel().AddOutput(output_name, parameters);
      };

    void Component::AddParameter(dictionary parameters)
    {
        try
        {
           // std::string pn = parameters["name"];    // FIXME: Can probably be removed
            //if(pn=="name")                          // FIXME: Add comparison to dictionary value
            //    return;
            //if(pn=="class")
             //   return;            
            std::string parameter_name = path_+"."+validate_identifier(parameters["name"]);
            kernel().AddParameter(parameter_name, parameters);
        }
        catch(const std::exception& e)
        {
            throw exception("While adding parameter \""+std::string(parameters["name"])+"\": "+ e.what());
        }
    }


    void Component::SetParameter(std::string name, std::string value)
    {
        std::string parameter_name = path_+"."+validate_identifier(name);
        kernel().SetParameter(parameter_name, value);
    }


    bool Component::LookupParameter(parameter & p, const std::string & name)
    {
        Kernel & k = kernel();
        if(k.parameters.count(path_+"."+name))
        {
            p = k.parameters[path_+"."+name];
            return true;
        }
        else if(parent_)
            return parent_->LookupParameter(p, name);
        else
            return false;
    }

//
// GetComponent
//
// sensitive to variables and indirection
// does local substitution of vaiables unlike GetValue() / FIXME: is this correct?
//

    Component * Component::GetComponent(const std::string & s) 
    {
        std::string path = SubstituteVariables(s);
        try
        {
            if(path.empty()) // this
                return this;
            if(path[0]=='.') // global
                return kernel().components.at(path.substr(1));
            if(kernel().components.count(path_+"."+peek_head(path,"."))) // inside
                return kernel().components[path_+"."+peek_head(path,".")]->GetComponent(peek_tail(path,"."));
            if(peek_rtail(peek_rhead(path_,"."),".") == peek_head(path,".") && parent_) // parent
                return parent_->GetComponent(peek_tail(path,"."));
            throw exception("Component does not exist.");
        }
        catch(const std::exception& e)
        {
            throw exception("Component \""+path+"\" does not exist.");
        }
    }


    std::string 
    Component::Evaluate(const std::string & s, bool is_string)
    {
        if(s.empty())
            return "";

    if(!expression::is_expression(s) || is_string)
    {
        if(s[0]=='@') // Handle indirection (unless expresson)
        {
            if(s.find('.')==std::string::npos)
                return GetValue(s.substr(1));
            
            std::string component_path = peek_rhead(s.substr(1), ".");
            std::string variable_name = SubstituteVariables(peek_rtail(s, "."));
            return GetComponent(component_path)->GetValue(variable_name);
        }
        else
            return s;
    }

        // Handle mathematical expression

        if(!expression::is_expression(s) || is_string)
            return s;

         expression e = expression(s);
            std::map<std::string, std::string> vars;
            for(auto v : e.variables())
            {
                std::string value = Evaluate(v);
                if(value.empty())
                    throw exception("Variable \""+v+"\" not defined.");
                vars[v] = value;
        }
        return std::to_string(expression(s).evaluate(vars));
    }


    int 
    Component::EvaluateIntExpression(std::string & s)
    {
        expression e = expression(s);
        std::map<std::string, std::string> vars;
        for(auto v : e.variables())
            vars[v] = Evaluate(v);
        return expression(s).evaluate(vars);
    }

    std::vector<int> 
    Component::EvaluateSizeList(std::string & s) // return list of size from size list in string
    {
        //s = Evaluate(s, true); // FIXME: evaluate as string fir s should probably be used in more places
        std::vector<int> shape;
        for(std::string e : split(s, ","))
        shape.push_back(EvaluateIntExpression(e));
        return shape;
    }

    std::vector<int> 
    Component::EvaluateSize(std::string & s) // Evaluate size/shape string
    {
        if(ends_with(s, ".size")) // special case: use shape function on input // FIXME: Move to EvaluateSizeList
        {
            auto & x = rsplit(s, ".", 1); // FIXME: rhead ???
            matrix m;
            Bind(m, x.at(0));
            return m.shape(); 
        }

        return EvaluateSizeList(s); // default case: parse size list
    }


// NEW EVALUATION FUNCTIONS

    double 
    Component::EvaluateNumber(std::string v)
    {
        return stod(v); // FIXME: Add full parding of expressions and variables *************
    }


    bool 
    Component::EvaluateBool(std::string v)
    {
        return false;
    }



    std::string 
    Component::EvaluateString(std::string v)
    {
        return "";
    }



    std::string 
    Component::EvaluateMatrix(std::string v)
    {
        return "";
    }



    int 
    Component::EvaluateOptions(std::string v, std::vector<std::string> & options)
    {
        return 0;
    }




  Component::Component():
    parent_(nullptr),
    info_(kernel().current_component_info),
    path_(kernel().current_component_path)
{
          // FIXME: Make sure there are empty lists. None of this should be necessary when dictionary is fixed

    if(info_["inputs"].is_null())
        info_["inputs"] = list();

    if(info_["outputs"].is_null())
        info_["outputs"] = list();

    if(info_["parameters"].is_null())
        info_["parameters"] = list();

    if(info_["groups"].is_null())
        info_["groups"] = list();

    if(info_["modules"].is_null())
        info_["modules"] = list();

    for(auto p: info_["parameters"])
        AddParameter(p);

    for(auto input: info_["inputs"])
        AddInput(input);

    for(auto output: info_["outputs"])
        AddOutput(output);

   // Set parent

    auto p = path_.rfind('.');
    if(p != std::string::npos)
        parent_ = kernel().components.at(path_.substr(0, p));
}


    bool
    Component::Notify(int msg, std::string message)
    {
        return kernel().Notify(msg, message);
    }



  Module::Module()
    {
  
    }




INSTALL_CLASS(Module)

// The following lines will create the kernel the first time it is accessed by one of the components

    Kernel& kernel()
    {
        static Kernel * kernelInstance = new Kernel();
        return *kernelInstance;
    }


    bool
    Component::InputsReady(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections) // FIXME: Handle optional buffers
    {
        Kernel& k = kernel();

        std::string n = d["name"];   // ["attributes"]
        for(auto & c : ingoing_connections[path_+'.'+n])
            if(k.buffers.at(c->source).rank()==0)
                return false;
        return true;
    }


    void Component::SetSourceRanges(const std::string & name, std::vector<Connection *> & ingoing_connections)
    {
        for(auto & c : ingoing_connections) // Copy source size to source_range if not set
        {
            if(c->source_range.empty())
                c->source_range = kernel().buffers[c->source].get_range();
            else if(c->source_range.rank() != kernel().buffers[c->source].rank())
                throw exception("Explicitly set source range dimensionality does not match source.");
        }

    }


    void Component::SetInputSize_Flat(const std::string & name, std::vector<Connection *> & ingoing_connections, bool use_alias)
    {
        SetSourceRanges(name, ingoing_connections);
        int begin_index = 0;
        int end_index = 0;
        int flattened_input_size = 0;
        for(auto & c : ingoing_connections)
        {
            c->flatten_ = true;
            int s = c->source_range.size() * c->delay_range_.trim().b_[0];
            end_index = begin_index + s;
            c->target_range = range(begin_index, end_index);
            begin_index += s;
            flattened_input_size += s;
        }
        if(flattened_input_size != 0)
            kernel().buffers[name].realloc(flattened_input_size);   // FIXME: else buffer = {} ???

        if(!use_alias)
            return;

        begin_index = 0;
        for(auto & c : ingoing_connections)
        {
            int s = c->source_range.size() * c->delay_range_.trim().b_[0];
            if(c->alias_.empty())
                kernel().buffers[name].push_label(0, c->source, s);
            else
                kernel().buffers[name].push_label(0, c->alias_, s);
        }
    }


    void Component::SetInputSize_Index(const std::string & name, std::vector<Connection *> & ingoing_connections, bool use_alias)
    {
            SetSourceRanges(name, ingoing_connections);
            int max_delay = 0;
            for(auto & c : ingoing_connections) // STEP 0b: copy source_range to target_range if not set
            {
                if(!c->delay_range_.empty() && c->delay_range_.trim().b_[0] > max_delay)
                    max_delay = c->delay_range_.trim().b_[0];

                if(c->target_range.empty())
                    c->target_range = c->source_range;
                else
                {
                    int si = c->source_range.rank()-1;
                
                    for(int ti = c->target_range.rank()-1; ti>=0; ti--, si--)
                        if(c->target_range.b_[ti] == 0)
                        {
                            c->target_range.inc_[ti] = c->source_range.inc_[si]; // FIXME: Is this correct? Or should it shrink?
                            c->target_range.a_[ti] = c->source_range.a_[si];
                            c->target_range.b_[ti] = c->source_range.b_[si];
                            c->target_range.index_[ti] = c->target_range.a_[si]; // FIXME: Check if this is necesary
                        }
                }

                if(c->delay_range_.size() > 1) // Add extra dimension to input if connection is a delay range with more than one delay
                    c->target_range.push_front(0, c->delay_range_.trim().b_[0]);
            }

        range r;

        for(auto & c : ingoing_connections)  // STEP 1: Calculate range extent
            r |= c->target_range;

        kernel().buffers[name].realloc(r.extent());  // STEP 2: Set input size // FIXME: Check empty input

        // Set alias

        if(!use_alias)
            return;

        if(ingoing_connections.size() == 1 && !ingoing_connections[0]->alias_.empty())
        {
            kernel().buffers[name].set_name(ingoing_connections[0]->alias_);
        }
        else
        {
            // Handle multiple dimensions
        }
    }


    void Component::SetInputSize(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections)
    {
            Kernel& k = kernel();
            std::string input_name = path_ + "." + std::string(d["name"]);

            // FIXME: Use input type heuristics here ************

            std::string use_alias = d["use_alias"];

            std::string flatten = d["flatten"];
            if(is_true(flatten))
                SetInputSize_Flat(input_name, ingoing_connections[input_name],is_true(use_alias));
            else
                SetInputSize_Index(input_name, ingoing_connections[input_name],is_true(use_alias));
    }


    // Attempt to set sizes for a single component

    int
    Component::SetSizes(std::map<std::string,std::vector<Connection *>> & ingoing_connections) // FIXME: add more exception handling
    {
        Kernel& k = kernel();

        // Set input sizes (if possible)

        for(auto d : info_["inputs"])
            if(k.buffers[path_+"."+std::string(d["name"])].empty())
            {   
                if(InputsReady(d, ingoing_connections))
                    SetInputSize(d, ingoing_connections);
            }

        // Set output sizes // FIXME: Move to separate function

        int outputs_with_size = 0;
        for(auto & d : info_["outputs"])
        {
            std::string n = d["name"];  // FIXME: Throw if not set
            std::string s = d["size"];
            std::vector<int> shape = EvaluateSize(s);
            matrix o;
            Bind(o, n); // FIXME: Get directly?
            if(o.rank() != 0)
                outputs_with_size++; // Count number of buffers that are set
            
            for(int i=0; i < shape.size(); i++)
                if(shape[i]<0)
                {
                    kernel().Notify(msg_fatal_error, "Output "s+n+" has negative size for dimension "+std::to_string(i)); // FIXME: throw
                    return 0;
                }

            o.realloc(shape);
            outputs_with_size++;
        }

        if(outputs_with_size == info_["outputs"].size())
            return 0; // all buffers have been set
        else
            return 0; // needs to be called again - FIXME: Report progress later on
    }

    void
    Component::CalculateCheckSum(long & check_sum, prime & prime_number) // Calculates a value that depends on all parameters and output sizes; used for integrity testing of kernel and module
    {
        // Iterate over all outputs

        for(auto & d : info_["outputs"])
        {
            matrix output;
            Bind(output, d["name"]);
            //output.info();
            for(long d : output.shape())
                check_sum += prime_number.next() * d;
        } 

        // Iterate obver all parameters
    
        for(auto & d : info_["parameters"])
        {
            parameter p;
            Bind(p, d["name"]);
            //std::cout << "Parameter: " << d["name"] << std::endl;

            if(p.type == string_type)
                check_sum += prime_number.next() * character_sum(p);
            else
            if(p.type == matrix_type)
                check_sum += prime_number.next() * p.matrix_value->size();
            else
                check_sum += prime_number.next() *  p.as_int(); // FIXME: convert to long later 
        }

        // std::cout << "Check sum: " << check_sum << std::endl;
    }

    // Connection

        Connection::Connection(std::string s, std::string t, range & delay_range, std::string alias)
        {
            source = peek_head(s, "[");
            source_range = range(peek_tail(s, "[", true));
            target = peek_head(t, "[");
            target_range = range(peek_tail(t, "[", true));
            delay_range_ = delay_range;
            flatten_ = false;
            alias_ = alias;
        }


        void
        Connection::Print()
        {
            std::cout << "\t" << source <<  delay_range_.curly() <<  std::string(source_range) << " => " << target  << std::string(target_range);
            if(!alias_.empty())
                std::cout << " \"" << alias_ << "\"";
            std::cout << '\n'; 
        }


    // Class

        Class::Class(std::string n, std::string p) : module_creator(nullptr), name(n), path(p),info_(p)
        {
         //   info_ = dictionary(path);
        }

        Class::Class(std::string n, ModuleCreator mc) : module_creator(mc), name(n)
        {
         //   info_ = dictionary(path);
        }


        void 
        Class::print()
        {
            std::cout << name << ": " << path  << '\n';
        }

    // Request

        Request::Request(std::string  uri, long sid, std::string b):
            body(b)
        {
            url = uri;
            session_id = sid;
            uri.erase(0, 1);
            std::string params = tail(uri, "?");
            command = head(uri, "/"); 
            //view_name = tail(uri, "/"); 
            component_path = uri;
            parameters.parse_url(params);
        }

    bool operator==(Request & r, const std::string s)
    {
        return r.command == s;
    }

    // Kernel

        void
        Kernel::Clear()
        {
            //std::cout << "Kernel::Clear" << std::endl;
            // FIXME: retain persistent components
            components.clear();
            connections.clear();
            buffers.clear();   
            max_delays.clear();
            circular_buffers.clear();
            parameters.clear();

            tick = -1;
            //run_mode = run_mode_pause;
            tick_is_running = false;
            tick_time_usage = 0;
            tick_duration = 1; // default value
            actual_tick_duration = tick_duration;
            idle_time = 0;
            stop_after = -1;
            shutdown = false;
            info_ = dictionary();
            info_["filename"] = ""; //EMPTY FILENAME

            session_id = new_session_id();
        }


        void
        Kernel::New()
        {
            //std::cout << "Kernel::New" << std::endl;
            //log.push_back(Message("New file"));
            Notify(msg_print, "New file");
       
            Clear();
            Pause();
            dictionary d;

            d["_tag"] = "group";
            d["name"] = "Untitled";
            d["groups"] = list();
            d["modules"] = list();
            d["widgets"] = list();
            d["connections"] = list();       
            d["inputs"] = list();            
            d["outputs"] = list();            
            d["parameters"] = list();           
            d["stop"] = "-1";
            // d["webui_port"] = "8000";

            SetCommandLineParameters(d);
            d["filename"] = "";
            BuildGroup(d);
            info_ = d;
            session_id = new_session_id(); 
            SetUp(); // FIXME: Catch exceptions if new failes
        }


        void
        Kernel::Tick()
        {
        tick_is_running = true; // Flag that state changes are not allowed
        tick++;
         for(auto & m : components)
            try
            {
                //std::cout <<" Tick: " << m.second->info_["name"] << std::endl;
                m.second->Tick();
            }
            catch(const empty_matrix_error& e)
            {
                throw std::out_of_range(m.first+"."+e.message+" (Possibly an empty matrix or an input that is not connected).");  
            }
            catch(const std::exception& e)
            {
                throw exception(m.first+". "+std::string(e.what()));
            }

        RotateBuffers();
        Propagate();

        CalculateCPUUsage();
        tick_is_running = false; // Flag that state changes are allowed again
    }


    bool Kernel::Terminate()
    {
        return (stop_after!= -1 &&  tick >= stop_after);
    }


    void Kernel::ScanClasses(std::string path) // FIXME: Add error handling
    {
        if(!std::filesystem::exists(path))
        {
            std::cout << "Could not scan for classes \""+path+"\". Directory not found." << std::endl;
            return;
        }
        for(auto& p: std::filesystem::recursive_directory_iterator(path))
            if(std::string(p.path().extension())==".ikc")
            {
                std::string name = p.path().stem();
                classes[name].path = p.path();
                classes[name].info_ = dictionary(p.path());
            }
    }


    void Kernel::ScanFiles(std::string path, bool system)
    {
        if(!std::filesystem::exists(path))
        {
            std::cout << "Could not scan for files in \""+path+"\". Directory not found." << std::endl;
            return;
        }
        for(auto& p: std::filesystem::recursive_directory_iterator(path))
            if(std::string(p.path().extension())==".ikg")
            {
                std::string name = p.path().stem();

                if(system)
                     system_files[name] = p.path();
                else
                     user_files[name] = p.path(); 
            }
    }



    void Kernel::ListClasses()
    {
        std::cout << "\nClasses:" << std::endl;
        for(auto & c : classes)
            c.second.print();
    }


    void Kernel::ResolveParameter(parameter & p,  std::string & name)
    {
        std::size_t i = name.rfind(".");
        if(i == std::string::npos)
            return; // FIXME: Is this an error?

        auto c = components.at(name.substr(0, i));
        std::string parameter_name = name.substr(i+1, name.size());
        c->ResolveParameter(p, parameter_name);
    }


    void Kernel::ResolveParameters() // Find and evaluate value or default // FIXME: return success
    {
        bool ok = true;
        for (auto p=parameters.rbegin(); p!=parameters.rend(); p++) // Reverse order equals outside in in groups
        {
            std::size_t i = p->first.rfind(".");
            // Find component and parameter_name and resolve
            i = p->first.rfind(".");
            if(i != std::string::npos)
            {
                auto c = components.at(p->first.substr(0, i));
                std::string parameter_name = p->first.substr(i+1, p->first.size());
                  ok &= c->ResolveParameter(p->second, parameter_name);
            }
        }
        if(!ok)
            throw fatal_error("All parameters could not be resolved");
    }


    void Kernel::CalculateSizes()
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


    void Kernel::CalculateDelays()
    {
        for(auto & c : connections)
        {
            if(!max_delays.count(c.source))
                max_delays[c.source] = 0;
            if(c.delay_range_.extent()[0] > max_delays[c.source])
            {
                max_delays[c.source] = c.delay_range_.extent()[0];
            }
        }

        //std::cout << "\nDelays:" << std::endl;
        //for(auto & o : buffers)
        //    std::cout  << "\t" << o.first <<": " << max_delays[o.first] << std::endl;
    }


    void Kernel::InitCircularBuffers()
    {
        //std::cout << "\nInitCircularBuffers:" << std::endl;
        for(auto it : max_delays)
        {
            if(it.second <= 1)
                continue;
            //std::cout << "\t" << it.first << std::endl;
            circular_buffers.emplace(it.first, CircularBuffer(buffers[it.first], it.second)); // FIXME: Change to initialization list in C++20
        }
    }


    void Kernel::RotateBuffers()
    {
        for(auto & it : circular_buffers)
            it.second.rotate(buffers[it.first]);
    }


    void Kernel::ListComponents()
    {
        std::cout << "\nComponents:" << std::endl;
        for(auto & m : components)
            m.second->print();
    }


    void Kernel::ListConnections()
    {
        std::cout << "\nConnections:" << std::endl;
        for(auto & c : connections)
            c.Print();
    }


    void Kernel::ListInputs()
    {
        std::cout << "\nInputs:" << std::endl;
        for(auto & i : buffers)
            std::cout << "\t" << i.first <<  i.second.shape() << std::endl;
    }


   void Kernel::ListOutputs()
    {
        std::cout << "\nOutputs:" << std::endl;
        for(auto & o : buffers)
            std::cout  << "\t" << o.first << o.second.shape() << std::endl;
    }


    void Kernel::ListBuffers()
    {
        std::cout << "\nBuffers:" << std::endl;
        for(auto & i : buffers)
            std::cout << "\t" << i.first <<  i.second.shape() << std::endl;
    }


    void Kernel::ListCircularBuffers()
    {
        std::cout << "\nCircularBuffers:" << std::endl;
        for(auto & i : circular_buffers)
            std::cout << "\t" << i.first <<  " " << i.second.buffer_.size() << std::endl;
    }


   void Kernel::ListParameters()
    {
        std::cout << "\nParameters:" << std::endl;
        for(auto & p : parameters)
            std::cout  << "\t" << p.first << ": " << p.second << std::endl;
    }


    void Kernel::PrintLog()
    {
        for(auto & s : log)
            std::cout << "IKAROS: " << s.type << ": " << s.msg << std::endl;
        log.clear();
    }


    Kernel::Kernel():
        tick(0),
        run_mode(run_mode_pause),
        tick_is_running(false),
        tick_time_usage(0),
        actual_tick_duration(0), // FIME: Use desired tick duration here
        idle_time(0),
        stop_after(-1),
        tick_duration(1),
        shutdown(false),
        session_id(new_session_id())
        //webui_dir("../Source/WebUI/") // FIXME: get from somewhere else
    {
        cpu_cores = std::thread::hardware_concurrency();
    }

    // Functions for creating the network

    void Kernel::AddInput(std::string name, dictionary parameters) // FIXME: use name as argument insteas of parameters
    {
        buffers[name] = matrix().set_name(parameters["name"]);
    }

    void Kernel::AddOutput(std::string name, dictionary parameters)
    {
        buffers[name] = matrix().set_name(parameters["name"]);
    }

    void Kernel::AddParameter(std::string name, dictionary params)
    {
         parameters.emplace(name, parameter(params));
    }


    void Kernel::SetParameter(std::string name, std::string value)
    {
        if(!parameters.count(name))
            throw exception("Parameter \""+name+"\" could not be set because it doees not exist.");

        try
        {
            parameters[name] = value;
            parameters[name].info_["value"] = value;
        }
        catch(...)
        {
            throw exception("Parameter \""+name+"\" could not be set. Check that parameter exist and that the data type and value is correct.");
        }
    }


    void Kernel::AddGroup(dictionary info, std::string path)    // FIXME: Move to BuildGroup and rename Build -> Add
    {
        current_component_info = info;
        current_component_path = path;

        if(components.count(current_component_path)> 0)
            throw exception("Module or group named \""+current_component_path+"\" already exists.");

        components[current_component_path] = new Group(); // Implicit argument passing as for components
    }


    void Kernel::AddModule(dictionary info, std::string path)
    {
        current_component_info = info;
        current_component_path = path+"."+std::string(info["name"]);

        if(components.count(current_component_path)> 0)
            throw exception("Module or group with this name already exists.");

        std::string classname = info["class"];
        if(!classes.count(classname))
            throw exception("Class \""+classname+"\" does not exist.");

         info.merge(dictionary(classes[classname].path));  // merge with class data structure

        if(classes[classname].module_creator == nullptr)
            throw exception("Class \""+classname+"\" has no installed code. Check that it is included in CMakeLists.txt."); // TODO: Check that this works for classes that are allowed to have no code
        components[current_component_path] = classes[classname].module_creator(); // throws bad function call if not defined *** should be only difference from Group/Component
    }


    void Kernel::AddConnection(dictionary info, std::string path)
    {
         std::string souce = path+"."+std::string(info["source"]);   // FIXME: Look for global paths later - string conversion should not be necessary
         std::string target = path+"."+std::string(info["target"]);

         std::string delay_range = info.contains("delay") ? info["delay"] : "";// FIXME: return "" if name not in dict - or use containts *********
         std::string alias = info.contains("alias") ? info["alias"] : "";// FIXME: return "" if name not in dict - or use containts *********

        if(delay_range.empty() || delay_range=="null")  // FIXME: return "" if name not in dict - or use contains *********
            delay_range = "[1]";
        else if(delay_range[0] != '[')
            delay_range = "["+delay_range+"]";
        range r(delay_range);
        connections.push_back(Connection(souce, target, r, alias));
    }


    void Kernel::BuildGroup(dictionary d, std::string path) // Traverse dictionary and build all items at each level, FIXME: rename AddGroup later
    {
        std::string name = validate_identifier(d["name"]);
        if(!path.empty())
            name = path+"."+name;

        AddGroup(d, name);

        for(auto g : d["groups"])
            BuildGroup(g, name);
        for(auto m : d["modules"])
            AddModule(m, name);
        for(auto c : d["connections"])
            AddConnection(c, name);

        if(d["widgets"].is_null())
            d["widgets"] = list();

        // FIX OTHER THINGS HERE
    }


    void Kernel::InitComponents()
    {
        //std::cout << "Running Kernel::InitComponents()" << std::endl;
        // Call Init for all modules (after CalcalateSizes and Allocate)
        for(auto & c : components)
            try
            {
                c.second->Init();
            }
            catch(const fatal_error& e)
            {
                log.push_back(Message("F", "Fatal error. Init failed for \""+c.second->path_+"\": "+std::string(e.what()))); // FIXME: set end of execution
            }
            catch(const std::exception& e)
            {
                log.push_back(Message("F", "Init failed for "+c.second->path_+": "+std::string(e.what())));
            }
    }


    void Kernel::SetCommandLineParameters(dictionary & d) // Add command line arguments - will override XML - probably not correct ******************
    {
    
        for(auto & x : options_.d)
            d[x.first] = x.second;

        d["filename"] = options_.filename;

        if(d.contains("stop"))
            stop_after = d["stop"];

        if(d.contains("tick_duration"))
            tick_duration = d["tick_duration"];

//                if(run_mode >= run_mode_stop) // only look at below command line arguments started from command line
//                    {

        if(d.contains("start"))
            start = is_true(d["start"]);
/*
                    if(d.contains("real_time"))
                        if(is_true(d["real_time"]))
                        {
                            run_mode = run_mode_restart_realtime;
                            start = true;
                        }
*/
    }


    void Kernel::LoadFile()
    {
            //std::cout << "Kernel::LoadFile" << std::endl;
            try
            {
                dictionary d = dictionary(options_.filename);
                SetCommandLineParameters(d);
                BuildGroup(d);
                info_ = d;
                session_id = new_session_id(); 
                SetUp();
                log.push_back(Message("Loaded "s+options_.filename));

                CalculateCheckSum();
               // ListBuffers();
            }
            catch(const exception& e)
            {
                //log.push_back(Message("E", e.what()));
                // log.push_back(Message("E", "Load file failed for "s+options_.filename));
                //Notify(msg_fatal_error, e.what());
                Notify(msg_fatal_error, "Load file failed for "s+options_.filename);
                CalculateCheckSum();
                New();
            }
    }


    void Kernel::CalculateCheckSum()
    {
        return; // ****************************TEMPORARY *****************
        if(!info_.contains("check_sum"))
            return;

        long correct_check_sum = info_["check_sum"];
        long calculated_check_sum = 0;
        prime prime_number;
        for(auto & [n,c] : components)      
            c->CalculateCheckSum(calculated_check_sum, prime_number);
        if(correct_check_sum == calculated_check_sum)
            std::cout << "Correct Check Sum: " << calculated_check_sum << std::endl;
        else
        {
            std::string msg = "Incorrect Check Sum: "+std::to_string(calculated_check_sum)+" != "+std::to_string(correct_check_sum);
            Notify(msg_fatal_error, msg);
        }
    }


    void Kernel::Save() // Simple save function in present file
    {
        //std::cout << "Kernel::Save" << std::endl;
        std::string data = xml();

        //std::cout << data << std::endl;

        std::ofstream file;
        std::string filename = info_["filename"];
        file.open (filename);
        file << data;
        file.close();
    }


    void Kernel::SortNetwork()
    {
        // Resolve paths
        // Sort Components and Connections => Thread Groups and Partially Ordered Events (Tick & Propagate)
    }


    void Kernel::Propagate()
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

                }
            }
        }
    }


    void Kernel::InitSocket(long port)
    {
        try
        {
            socket =  new ServerSocket(port);
        }
        catch (const SocketException& e)
        {
            throw fatal_error("Ikaros is unable to start a webserver on port "+std::to_string(port)+". Make sure no other ikaros process is running and try again.");
        }
        httpThread = new std::thread(Kernel::StartHTTPThread, this);
    }


    void
    Kernel::SetUp()
    {
        try
        {
            ResolveParameters();
            CalculateDelays();
            CalculateSizes();
            InitCircularBuffers();
            InitComponents();
 /*
            ListComponents();
            ListConnections();
            //ListInputs();
            //ListOutputs();
            ListBuffers();
            ListCircularBuffers();
            ListParameters();
            PrintLog();
*/
        }
        catch(exception & e)
        {
            Notify(msg_fatal_error, e.message);
            Notify(msg_fatal_error, "SetUp Failed");
            //std::cout << "SetUp Failed" << std::endl;
            throw fatal_error("SetUp Failed");
        }
    }


        void
        Kernel::Run()
        {
            if(options_.filename.empty())
                New();
            else
                LoadFile();
            timer.Restart();
            tick = -1; // To make first tick 0 after increment

            if(run_mode == run_mode_restart_realtime)
                Realtime();
            else if(run_mode == run_mode_restart_play)
                Play();
            else
                Pause();

            // Main loop
            while(run_mode > run_mode_quit)  // Not quit or restart
            {
                while (!Terminate() && run_mode > run_mode_quit)
                {
                    while(sending_ui_data)
                        {}
                    while(handling_request)
                        {}

                    if(run_mode == run_mode_realtime)
                        lag = timer.WaitUntil(double(tick+1)*tick_duration);
                    else if(run_mode == run_mode_play)
                    {
                        timer.SetTime(double(tick+1)*tick_duration); // Fake time increase
                        Sleep(0.01);
                    }
                    else
                        Sleep(0.01); // Wait 10 ms to avoid wasting cycles if there are no requests


                    // Run_mode may have changed during the delay - needs to be checked again

                    if(run_mode == run_mode_realtime || run_mode == run_mode_play) 
                    {
                        actual_tick_duration = intra_tick_timer.GetTime();
                        intra_tick_timer.Restart();
                        Tick();
                        tick_time_usage = intra_tick_timer.GetTime();
                        idle_time = tick_duration - tick_time_usage;
                    }                    
                }


                Sleep(0.1);
            }
        }

        bool
        Kernel::Notify(int msg, std::string message)
        {
            log.push_back(Message(message));
            if(msg <= msg_fatal_error)
            {
                    std::cout << "ikaros: " << message << std::endl;
                    run_mode = run_mode_quit;
            }
            return true;
        }

    //
    //  Serialization
    //

    std::string 
    Component::json()
    {
        return info_.json(); // FIXME: Will be used when shared dictionaries are implemented
    }


    std::string 
    Component::xml()
    {
        return info_.xml("group");
    }


    std::string 
    Kernel::json()
    {
        return info_.json();
    }


    std::string 
    Kernel::xml()
    {
        if(components.empty())
            return "";
        else
            return components.begin()->second->xml();
    }



    //
    // WebUI
    //


    void
    Kernel::SendImage(matrix & image, std::string & format) // Compress image to jpg and send from memory after base64 encoding
    {
        long size = 0;
        unsigned char * jpeg = nullptr;
        size_t output_length = 0 ;
        
        if(format=="rgb" && image.rank() == 3 && image.size(0) == 3)
            jpeg = (unsigned char *)create_color_jpeg(size, image, 90);

        else if(format=="gray" && image.rank() == 2)
            jpeg = (unsigned char *)create_gray_jpeg(size, image, 0, 1, 90);

        else if(image.rank() == 2) // taking our chances with the format...
            jpeg = (unsigned char *)create_pseudocolor_jpeg(size, image, 0, 1, format, 90);

            if(!jpeg)
            {
                socket->Send("\"\"");
                return;
            }

        char * jpeg_base64 = base64_encode(jpeg, size, &output_length);
        socket->Send("\"data:image/jpeg;base64,");
        bool ok = socket->SendData(jpeg_base64, output_length);
        socket->Send("\"");
        destroy_jpeg(jpeg);
        free(jpeg_base64);
    }



    void
    Kernel::Stop()
    {
        while(tick_is_running)
            {}

        run_mode = run_mode_stop;
        tick = -1;
        timer.Pause();
        timer.SetPauseTime(0);
    }



    void
    Kernel::Pause()
    {
        while(tick_is_running)
            {}

        if(run_mode == run_mode_stop)
        {
            run_mode = run_mode_restart_pause;
        }
        else
        {
            run_mode = run_mode_pause;
            timer.Pause();
            timer.SetPauseTime(GetTime()+tick_duration);
        }
    }



/*
    void
    Kernel::Play()
    {
        while(tick_is_running)
            {}

        if(run_mode == run_mode_stop)
        {
            run_mode = run_mode_pause;
            request_restart = true;
            requested_restart_mode = run_mode_play;
            return;
        }

        run_mode = run_mode_play;
        timer.Pause();
        timer.SetPauseTime(GetTime()+tick_duration);
    }
*/

    void
    Kernel::Realtime()
    {
        while(tick_is_running)
            {}

        if(run_mode == run_mode_stop)
        {
            run_mode = run_mode_restart_realtime;
        }
        else
        {
     
            Pause();
            timer.Continue();
            run_mode = run_mode_realtime;
        }
    }



    void
    Kernel::Play()
    {
        while(tick_is_running)
            {}

        if(run_mode == run_mode_stop)
        {
            run_mode = run_mode_restart_play;
        }
        else
        {
            run_mode = run_mode_play;
            timer.Continue();
        }
    }


    void
    Kernel::DoSendDataHeader()
    {
        Dictionary header;
        header.Set("Session-Id", std::to_string(session_id).c_str());
        header.Set("Package-Type", "data");
        header.Set("Content-Type", "application/json");
        header.Set("Cache-Control", "no-cache");
        header.Set("Cache-Control", "no-store");
        header.Set("Pragma", "no-cache");
        header.Set("Expires", "0");
        socket->SendHTTPHeader(&header);
    }


    void
    Kernel::DoSendDataStatus()
    {
        std::string nm = std::string(info_["filename"]);
        if(nm.find("/") != std::string::npos)
            nm = rtail(nm,"/");

        socket->Send("\t\"file\": \"%s\",\n", nm.c_str());

#if DEBUG
        socket->Send("\t\"debug\": true,\n");
#else
        socket->Send("\t\"debug\": false,\n");
#endif

        socket->Send("\t\"state\": %d,\n", run_mode);
        if(stop_after != -1)
        {
            socket->Send("\t\"tick\": \"%d / %d\",\n", tick, stop_after);
            socket->Send("\t\"progress\": %f,\n", double(tick)/double(stop_after));
        }
        else
        {
            socket->Send("\t\"progress\": 0,\n");
        }

        // Timing information

        double uptime = uptime_timer.GetTime();
        double total_time = GetTime();

        socket->Send("\t\"timestamp\": %ld,\n", GetTimeStamp()); // duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()
        socket->Send("\t\"uptime\": %.2f,\n", uptime);
        socket->Send("\t\"tick_duration\": %f,\n", tick_duration);
        socket->Send("\t\"cpu_cores\": %d,\n", cpu_cores);
    
        switch(run_mode)
        {
            case run_mode_stop:
                socket->Send("\t\"tick\": \"-\",\n");
                socket->Send("\t\"time\": \"-\",\n");
                socket->Send("\t\"ticks_per_s\": \"-\",\n");
                socket->Send("\t\"actual_duration\": \"-\",\n");
                socket->Send("\t\"lag\": \"-\",\n");
                socket->Send("\t\"time_usage\": 0,\n");
                socket->Send("\t\"cpu_usage\": 0,\n"); 
                break;

            case run_mode_pause:
                socket->Send("\t\"tick\": %lld,\n", GetTick());
                socket->Send("\t\"time\": %.2f,\n", GetTime());
                socket->Send("\t\"ticks_per_s\": \"-\",\n");
                socket->Send("\t\"actual_duration\": \"-\",\n");
                socket->Send("\t\"lag\": \"-\",\n");
                socket->Send("\t\"time_usage\": %f,\n", actual_tick_duration> 0 ? tick_time_usage/actual_tick_duration : 0);
                socket->Send("\t\"cpu_usage\": %f,\n", cpu_usage);  
                break;

            case run_mode_realtime:
            default:
                socket->Send("\t\"tick\": %lld,\n", GetTick());
                socket->Send("\t\"time\": %.2f,\n", GetTime());
                socket->Send("\t\"ticks_per_s\": %.2f,\n", tick>0 ? double(tick)/total_time: 0);
                socket->Send("\t\"actual_duration\": %f,\n", actual_tick_duration);
                socket->Send("\t\"lag\": %f,\n", lag);
                socket->Send("\t\"time_usage\": %f,\n", actual_tick_duration> 0 ? tick_time_usage/actual_tick_duration : 0);
                socket->Send("\t\"cpu_usage\": %f,\n", cpu_usage);
                break;
        }

    }


    void
    Kernel::DoSendLog(Request & request)
    {
    socket->Send(",\n\"log\": [");
    std::string sep;
    for(auto line : log)
    {
        socket->Send(sep +line.json());
        sep = ",";
    }
    socket->Send("]");
    log.clear();
    }


    void
    Kernel::DoSendData(Request & request)
    {    
        //std::cout << "DoSendData: state = " << run_mode << std::endl;
        sending_ui_data = true; // must be set while main thread is still running
        while(tick_is_running)
            {}

        DoSendDataHeader();

        socket->Send("{\n");

        DoSendDataStatus();

        socket->Send("\t\"data\":\n\t{\n");
    
        //std::string root = tail(args, "data=");

        std::string data = request.parameters["data"];  // FIXME: Check that it exists ******** or return ""

        std::string sep = "";
        bool sent = false;

        while(!data.empty())
        {
            std::string source = head(data, ",");
            std::string format = rtail(source, ":");
            std::string source_with_root = request.component_path +"."+source;

            if(buffers.count(source_with_root))
            {
                if(format.empty())
                {
                    sent = socket->Send(sep + "\t\t\"" + source + "\": "+buffers[source_with_root].json());
                }
                else if(format=="rgb")
                { 
                        sent = socket->Send(sep + "\t\t\"" + source + ":"+format+"\": ");
                        SendImage(buffers[source_with_root], format);
                }
                else if(format=="gray" || format=="red" || format=="green" || format=="blue" || format=="spectrum" || format=="fire")
                { 
                        sent = socket->Send(sep + "\t\t\"" + source + ":"+format+"\": ");
                        SendImage(buffers[source_with_root], format);
                }
            }
            else if(parameters.count(source_with_root))
            {
                sent = socket->Send(sep + "\t\t\"" + source + "\": "+parameters[source_with_root].json());
            }
            if(sent)
                sep = ",\n";
        }

        socket->Send("\n\t}");
        DoSendLog(request);
        socket->Send(",\n\t\"has_data\": "+std::to_string(!tick_is_running)+"\n"); // new tick has started during sending; there may be data but it cannot be trusted
        socket->Send("}\n");

        sending_ui_data = false;
    }


    void
    Kernel::DoNew(Request & request)
    {
        Pause(); // Probably not necessary
        New();
        DoUpdate(request);
    }


    void
    Kernel::DoOpen(Request & request)
    {
        std::string file = request.parameters["file"];
        std::string where = request.parameters["where"];
        Stop();
        Clear();
        if(where == "system")
            options_.filename = system_files.at(file);
        else
            options_.filename = user_files.at(file);
        LoadFile();
        DoUpdate(request);
    }


    void
    Kernel::DoSave(Request & request)
    {
        //std::cout << "SAVE: " << request.body << std::endl;

        dictionary d = parse_json(request.body);
        std::filesystem::path path = std::string(d["filename"]);
        std::string filename = path.filename();
        d["filename"] = null(); // FIXME: remove propery later
        std::string data = d.xml("group");
        std::ofstream file;
        file.open (filename);
        file << data;
        file.close();
        Clear();
        options_.filename = filename;
        LoadFile();
        std::string s = "{\"status\":\"ok\"}"; 
        Dictionary rtheader;
        rtheader.Set("Session-Id", std::to_string(session_id).c_str());
        rtheader.Set("Package-Type", "network");
        rtheader.Set("Content-Type", "application/json");
        rtheader.Set("Content-Length", int(s.size()));
        socket->SendHTTPHeader(&rtheader);
        socket->SendData(s.c_str(), int(s.size()));
    }


    void
    Kernel::DoSaveAs(Request & request)
    {
    }


    void
    Kernel::DoQuit(Request & request)
    {
        log.push_back(Message("quit"));
        Stop();
        run_mode = run_mode_quit;
        DoUpdate(request);
    }


    void
    Kernel::DoStop(Request & request)
    {
        log.push_back(Message("stop"));
        Stop();
        DoUpdate(request);
    }


    void
    Kernel::DoSendFile(std::string file)
    {
        //std::this_thread::sleep_for(milliseconds(200));

        if(file[0] == '/')
            file = file.erase(0,1); // Remove initial slash

        // if(socket->SendFile(file, ikc_dir))  // Check IKC-directory first to allow files to be overriden
        //    return;

        if(socket->SendFile(file, webui_dir))   // Now look in WebUI directory
            return;
        /*
        if(socket->SendFile(file, std::string(webui_dir)+"Images/"))   // Now look in WebUI/Images directory
            return;
        
        file = "error." + rcut(file, ".");
        if(socket->SendFile("error." + rcut(file, "."), webui_dir)) // Try to send error file
            return;

        DoSendError();
        */
    }


    void
    Kernel::DoSendNetwork(Request & request)
    {
        std::string s = json(); 
        Dictionary rtheader;
        rtheader.Set("Session-Id", std::to_string(session_id).c_str());
        rtheader.Set("Package-Type", "network");
        rtheader.Set("Content-Type", "application/json");
        rtheader.Set("Content-Length", int(s.size()));
        socket->SendHTTPHeader(&rtheader);
        socket->SendData(s.c_str(), int(s.size()));
    }


    void
    Kernel::DoPause(Request & request)
    {
        log.push_back(Message("pause"));
        Pause();
        DoSendData(request);
    }


    void
    Kernel::DoStep(Request & request)
    {
        log.push_back(Message("step"));
        Pause();
        run_mode = run_mode_pause;
        Tick();
        timer.SetPauseTime(GetTime()+tick_duration);
        DoSendData(request);
    }


/*
    void
    Kernel::DoPlay(Request & request)
    {
        log.push_back("play");
        Play();
        run_mode = run_mode_play;
        timer.SetPauseTime(GetTime()+tick_duration);
        DoSendData(request);
    }
*/


    void
    Kernel::DoRealtime(Request & request)
    {
        log.push_back(Message("realtime"));
        Realtime();
        DoSendData(request);
    }


    void
    Kernel::DoPlay(Request & request)
    {
        log.push_back(Message("play"));
        Play();
        DoSendData(request);
    }


    void
    Kernel::DoCommand(Request & request)// FIXME: Not implemented **************
    {
        /*
        float x, y;
        char command[255];
        char value[1024]; // FIXME: no range chacks
        int c = sscanf(uri.c_str(), "/command/%[^/]/%f/%f/%[^/]", command, &x, &y, value);
        if(c == 4)
            SendCommand(command, x, y, value);

            */
        DoSendData(request);
    }



    void
    Kernel::DoControl(Request & request) // FIXME: No indices right now ************** NOT IMPLEMENTED
    {
        /*
        try
        {
            auto cmd = split(uri, "/");
            if(cmd.size() == 6)
            {
                auto param_name = cmd[2];
                int x = std::stoi(cmd[3]);
                int y = std::stoi(cmd[4]);
                float value = std::stof(cmd[5]);

                parameter & p = parameters.at(param_name);
                if(p.type == matrix_type)
                {
                    (*p.matrix_value)(x,y)= value;
                }
                else // if(p.type == double_type) // FIXME: scalar type
                {
                    p = value;
                }
            }
            DoSendData(request);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            DoSendData(request);
        }
        */
    }

#if FALSE
/*
    dictionary
    Kernel::GetView(std::string component, std::string view_name)   // FIXME: Remove later on
    {
        Component * c = components.at(component);
        dictionary cd = c->info_;
        list vs = cd["views"];
        dictionary v;

        for(auto u : vs) // Find view
            if(std::string(u["name"]) == view_name)
                return u;

        return dictionary();
    }


    dictionary 
    Kernel::GetView(Request & request)
    {
        Component * c = components.at(request.component_path);
        dictionary cd = c->info_;
        list vs = cd["views"];
        dictionary v;

        for(auto u : vs) // Find view
            if(std::string(u["name"]) == request.view_name)
                return u;

        return dictionary();
    }


    void
    Kernel::AddView(Request & request) // FIXME: Local exception handling
    {
        std::cout << "AddView: " << std::endl;

        Component * c = components.at(request.component_path);

        dictionary d;
        d["name"] = request.parameters["name"];
        d["widgets"] = list();
        c->info_["views"].push_back(d);

        DoSendData(request);
    }
*/


    void
    Kernel::AddWidget(Request & request) // FIXME: Local exception handling
    {
        std::cout << "AddWidget: " << std::endl;
/*
        auto view = GetView(request);
        if(view["widgets"].is_null())
            view["widgets"] = list();

        list u = list(view["widgets"]);
        u.push_back(request.parameters);

*/
        DoSendData(request);
    }


    void
    Kernel::DeleteWidget(Request & request)
    {
        //std::cout << "DeleteWidget: "  << std::endl;
/*
        int index = std::stoi(request.parameters["index"]);
        list view = list(GetView(request)["widgets"]);
        view.erase(index);

        int i=0;
        for(auto & w : view)
            w["_index_"] = i++;
*/
        DoSendData(request);
    }


    void
    Kernel::SetWidgetParameters(Request & request)
    {
        //std::cout << "SetWidgetParameters: " << std::endl;

        int index = request.parameters.get_int("_index_");
        //list u = list(GetView(request)["widgets"]);
        //u[index] = request.parameters;

        DoSendData(request);
    }


    void
    Kernel::WidgetToFront(Request & request)
    {
        //std::cout << "SetWidgetToFront: " << std::endl;

        int index = request.parameters.get_int("index");
/*
        //list u = list(GetView(request)["widgets"]);

        dictionary d = u[index];
        u.erase(index);
        u.push_back(d);
    
        int i=0;
        for(auto & item : u)
            item["_index_"] = i++;

*/
        DoSendData(request);
     }



    void
    Kernel::WidgetToBack(Request & request)
    {
        //std::cout << "SetWidgetToBack: " << std::endl;

        int index = request.parameters.get_int("index");
/*
        list u = list(GetView(request)["widgets"]);

        dictionary d = u[index];
        u.erase(index);
        u.insert_front(d);

        int i=0;
        for(auto & item : u)
            item["_index_"] = i++;
*/
        DoSendData(request);
    }

/*
    void
    Kernel::RenameView(Request & request)
    {
        std::cout << "RenameView: " << std::endl;

        dictionary u = GetView(request);
        u["name"] = request.parameters["name"];

        DoSendData(request);
    }
*/
#endif

    void
    Kernel::DoAddGroup(Request & request)
    {
        //std::cout << "DoAddGroup: *** " << std::endl;

        DoSendData(request);
    }


    void
    Kernel::DoAddModule(Request & request)
    {
       // std::cout << "DoAddModule: *** " << std::endl;

        DoSendData(request);
    }


    

    void
    Kernel::DoSetAttribute(Request & request)
    {
       // std::cout << "DoSetAttribute: " << std::endl;

        DoSendData(request);
    }


    void
    Kernel::DoAddConnection(Request & request)
    {
        //std::cout << "DoAddConnection: " << std::endl;

        DoSendData(request);
    }


    void
    Kernel::DoSetRange(Request & request)
    {
        //std::cout << "DoSetRange: " << std::endl;

        DoSendData(request);
    }


    void
    Kernel::DoUpdate(Request & request)
    {
        if(request.session_id != session_id) // request.parameters.empty() ||  ( WAS not a data request - send network)
        {
            DoSendNetwork(request);
        }
        /*
        else if(run_mode == run_mode_play && session_id == request.session_id)
        {
            Pause();
            Tick();
            DoSendData(request);
        }
        */
        else 
            DoSendData(request);
    }


    void
    Kernel::DoNetwork(Request & request)
    {
        DoSendNetwork(request);
    }


    void
    Kernel::DoSendClasses(Request & request)
    {
        Dictionary header;
        header.Set("Content-Type", "text/json");
        header.Set("Cache-Control", "no-cache");
        header.Set("Cache-Control", "no-store");
        header.Set("Pragma", "no-cache");
        socket->SendHTTPHeader(&header);
        socket->Send("{\"classes\":[\n\t\"");
        std::string s = "";
        for(auto & c: classes)
        {
            socket->Send(s.c_str());
            socket->Send(c.first.c_str());
            s = "\",\n\t\"";
        }
        socket->Send("\"\n]\n}\n");
    }



    void
    Kernel::DoSendClassInfo(Request & request)
    {
        Dictionary header;
        header.Set("Content-Type", "text/json");
        header.Set("Cache-Control", "no-cache");
        header.Set("Cache-Control", "no-store");
        header.Set("Pragma", "no-cache");
        socket->SendHTTPHeader(&header);
        socket->Send("{\n");
        std::string s = "";
        for(auto & c: classes)
        {
            socket->Send(s);
            socket->Send("\""+c.first+"\": ");
            socket->Send(c.second.info_.json());
            s = ",\n\t";
        }
        socket->Send("\n}\n");
    }



    void
    Kernel::DoSendFileList(Request & request)
    {
        // Scan for files

        system_files.clear();
        user_files.clear();
        ScanFiles(options_.ikaros_root+"/Source/Modules");
        ScanFiles(options_.ikaros_root+"/UserData", false);

        // Send result

        Dictionary header;
        header.Set("Content-Type", "text/json");
        header.Set("Cache-Control", "no-cache");
        header.Set("Cache-Control", "no-store");
        header.Set("Pragma", "no-cache");
        socket->SendHTTPHeader(&header);
        std::string sep;
    socket->Send("{\"system_files\":[\n\t\"");
        for(auto & f: system_files)
        {
            socket->Send(sep);
            socket->Send(f.first);
            sep = "\",\n\t\"";
        }
    socket->Send("\"\n],\n");
    
    sep = "";
    socket->Send("\"user_files\":[\n\t\"");
        for(auto & f: user_files)
        {
            socket->Send(sep);
            socket->Send(f.first);
            sep = "\",\n\t\"";
        }
    socket->Send("\"\n]\n");

    socket->Send("}\n");
    }


    void
    Kernel::DoSendError()
    {
    Dictionary header;
    header.Set("Content-Type", "text/plain");
    socket->SendHTTPHeader(&header);
    socket->Send("ERROR\n");
    }


    void
    Kernel::HandleHTTPRequest()
    {
        long sid = 0;
        const char * sids = socket->header.Get("Session-Id");
        if(sids)
            sid = atol(sids);

        Request request(socket->header.Get("URI"), sid, socket->body);

        //if(request.url != "/update/?data=")
        //std::cout << request.url << std::endl;

        if(request == "network")
            DoNetwork(request);

        else if(request == "update")
            DoUpdate(request);

        // Run mode commands

        else if(request == "quit")
            DoQuit(request);
        else if(request == "stop")
            DoStop(request);
        else if(request == "pause")
            DoPause(request);
        else if(request == "step")
            DoStep(request);
        else if(request == "play")
            DoPlay(request);
        else if(request == "realtime")
            DoRealtime(request);

        // File handling commands

        else if(request == "new")
            DoNew(request);
        else if(request == "open")
            DoOpen(request);
        else if(request == "save")
            DoSave(request);
        else if(request == "saveas")
            DoSaveAs(request);

        // Start up commands

        else if(request == "classes") 
            DoSendClasses(request);
        else if(request == "classinfo") 
            DoSendClassInfo(request);
        else if(request == "files") 
            DoSendFileList(request);
        else if(request == "")
            DoSendFile("index.html");

        // Control commands

        else if(request == "command")
            DoCommand(request);
        else if(request == "control")
            DoControl(request);

        // View editing
/*
        else if(request == "addview")
            AddView(request);
        else if(request == "renameview")
            RenameView(request);
        else if(request == "addwidget")
            AddWidget(request);
        else if(request == "delwidget")
            DeleteWidget(request);
        else if(request == "setwidgetparams")
            SetWidgetParameters(request);
        else if(request == "widgettofront")
            WidgetToFront(request);
        else if(request == "widgettoback")
            WidgetToBack(request);
*/
        // Network editing

        else if(request == "addgroup")
            DoAddGroup(request);
        else if(request == "addmodule")
            DoAddModule(request);
        else if(request == "setattribute")
            DoSetAttribute(request);
        else if(request == "addconnection")
            DoAddConnection(request);
        else if(request == "setrange")
            DoSetRange(request);
        else 
            DoSendFile(request.url);
    }


void
Kernel::CalculateCPUUsage() // In percent
{
    double cpu = 0;
    struct rusage rusage;
    if(getrusage(RUSAGE_SELF, &rusage) != -1)
        cpu = double(rusage.ru_utime.tv_sec) + double(rusage.ru_utime.tv_usec) / 1000000.0;
    if(actual_tick_duration > 0)
        cpu_usage = (cpu-last_cpu)/double(cpu_cores)*actual_tick_duration;
    last_cpu = cpu;
}


    void
    Kernel::HandleHTTPThread()
    {
        while(!shutdown)
        {
            if (socket != nullptr && socket->GetRequest(true))
            {
                if (equal_strings(socket->header.Get("Method"), "GET"))
                {
                    while(tick_is_running)
                        {}
                    handling_request = true;
                    HandleHTTPRequest();
                    handling_request = false;
                }
                else if (equal_strings(socket->header.Get("Method"), "PUT")) // JSON Data
                {
                    while(tick_is_running)
                        {}
                    handling_request = true;
                    HandleHTTPRequest();
                    handling_request = false;
                }
                socket->Close();
            }
        }
    }


    void *
    Kernel::StartHTTPThread(Kernel * k)
    {
    k->HandleHTTPThread();
    return nullptr;
    }

}; // namespace ikaros



Kernel::~Kernel()
{
    if(socket)
    {
        shutdown=true;
        while(handling_request)
            {}
        Sleep(0.1);
        delete socket; 
    }
}