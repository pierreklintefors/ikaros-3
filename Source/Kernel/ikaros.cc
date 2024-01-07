// Ikaros 3.0

#include "ikaros.h"

using namespace ikaros;

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

    parameter::parameter(std::string type_string, std::string default_val, std::string options_string): type(no_type), timebase(1)
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

    void 
    parameter::operator=(parameter & p) // this shares data with p
    {
        type = p.type;
        default_value = p.default_value;
        int_value = p.int_value;
        float_value = p.float_value;
        matrix_value = p.matrix_value;
        string_value = p.string_value;
        options = p.options;
    }

    int 
    parameter::operator=(int v)
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

    float 
    parameter::operator=(float v)
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

    float 
    parameter::operator=(double v) 
    { 
        return operator=(float(v));
    }


    std::string 
    parameter::operator=(std::string v) // FIXME: ADD ALL TYPE HERE!!! *** 
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


    parameter::operator matrix & ()
    {
        if(type==matrix_type && matrix_value) 
            return *matrix_value;

        throw exception("Not a matrix value.");
    }

    parameter::operator std::string()
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

    parameter::operator bool()
    {
        return *int_value!=0;   // FIXME: Check all types here
    }

/*
    parameter::operator int()
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
*/

    int parameter::as_int()
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


    const char* parameter::c_str() const noexcept
    {
        if(string_value)
            return string_value->c_str();
        else
            return NULL;
    }


    parameter::operator float()
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



    std::string 
    parameter::json()
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

    std::ostream& operator<<(std::ostream& os, parameter p)
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

float operator+(parameter p, float x) { return (float)p+x; }
float operator+(float x, parameter p) { return (float)p+x; }
float operator+(parameter p, double x) { return (float)p+x; }
float operator+(double x, parameter p) { return (float)p+x; }
float operator+(parameter x, parameter p) { return (float)p+(float)x; }

float operator-(parameter p, float x) { return (float)p-x; }
float operator-(float x, parameter p) { return x-(float)p; }
float operator-(parameter p, double x) { return (float)p-x; }
float operator-(double x, parameter p) { return x-(float)p; }
float operator-(parameter x, parameter p) { return (float)x-(float)p; }

float operator*(parameter p, float x) { return (float)p*x; }
float operator*(float x, parameter p) { return (float)p*x; }
float operator*(parameter p, double x) { return (float)p*x; }
float operator*(double x, parameter p) { return (float)p*x; }
float operator*(parameter x, parameter p) { return (float)p*(float)x; }


float operator/(parameter p, float x) { return (float)p*x; }
float operator/(float x, parameter p) { return x/(float)p; }
float operator/(parameter p, double x) { return (float)p*x; }
float operator/(double x, parameter p) { return x/(float)p; }
float operator/(parameter x, parameter p) { return (float)x/(float)p; }

// Component

    void 
    Component::print()
    {
        std::cout << "Component: " << info_["name"]  << '\n';
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
    Component::ResolveParameter(parameter & p,  std::string & name,   std::string & default_value)
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
            SetParameter(name, Evaluate(v, p.type == string_type));
            return true;
        }

        SetParameter(name, Evaluate(default_value, p.type == string_type));
        return true; // IF refault value ******
    }



    std::string 
    Component::GetValue(const std::string & name)    // Get value of a attribute/variable in the context of this component
    {        
        if(dictionary(info_).contains(name))
            return Evaluate(info_[name]);
        if(parent_)
            return parent_->GetValue(name);
        return "";
    }


  std::string 
    Component::GetBind(const std::string & name)
    {
         if(dictionary(info_).contains(name))
            return ""; // Value set in attribute - do not bind
        if(dictionary(info_).contains(name+".bind")) 
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
        //if(name=="name")
        //    return;
        //if(name=="class")
        //    return;  
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


    Component * Component::GetComponent(const std::string & s) // Get component; sensitive to variables and indirection - changed to peek versions
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
        s = Evaluate(s, true); // FIXME: evaluate as string fir s shold probably be used in more places
        std::vector<int> shape;
        for(std::string e : split(s, ","))
        shape.push_back(EvaluateIntExpression(e));
        return shape;
    }

    std::vector<int> 
    Component::EvaluateSize(std::string & s) // Evaluate size/shape string
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


  Component::Component():
    parent_(nullptr),
    info_(kernel().current_component_info),
    path_(kernel().current_component_path)
{
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

  Module::Module()
    {
    }


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


    void Component::SetInputSize_Flat(const std::string & name, std::vector<Connection *> & ingoing_connections, bool add_labels)
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

        if(!add_labels)
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


    void Component::SetInputSize_Index(const std::string & name, std::vector<Connection *> & ingoing_connections)
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
    }


    void Component::SetInputSize(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections)
    {
            Kernel& k = kernel();
            std::string input_name = path_ + "." + std::string(d["name"]);

            // FIXME: Use input type heuristics here ************

            std::string add_labels = d["add_labels"];

            std::string flatten = d["flatten"];
            if(is_true(flatten))
                SetInputSize_Flat(input_name, ingoing_connections[input_name],is_true(add_labels));
            else
                SetInputSize_Index(input_name, ingoing_connections[input_name]);
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
                if(InputsReady(dictionary(d), ingoing_connections))
                    SetInputSize(dictionary(d), ingoing_connections);
            }

        // Set output sizes // FIXME: Move to separate function

        int outputs_with_size = 0;
        for(auto & d : info_["outputs"])
        {
            std::string n = d["name"];
            std::string s = d["size"];
            std::vector<int> shape = EvaluateSize(s);
            matrix o;
            Bind(o, n); // FIXME: Get directly?
            if(o.rank() != 0)
                outputs_with_size++; // Count number of buffers that are set
            
            o.realloc(shape);
            outputs_with_size++;
        }

        if(outputs_with_size == info_["outputs"].size())
            return 0; // all buffers have been set
        else
            return 0; // needs to be called again - FIXME: Report progress later on
    }


    // Connection

        Connection::Connection(std::string s, std::string t, range & delay_range, std::string alias)
        {
            source = peek_head(s, "[");
            source_range = range(peek_tail(s, "[")); // FIXME: CHECK NEW TAIL FUNCTION (used to include separator)
            target = peek_head(t, "[");
            target_range = range(peek_tail(t, "[")); // FIXME: CHECK NEW TAIL FUNCTION (used to include separator)
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

        void 
        Class::print()
        {
            std::cout << name << ": " << path  << '\n';
        }

    // Kernel

        void
        Kernel::Clear()
        {
            // FIXME: retain persistent components
            components.clear();
            connections.clear();
            buffers.clear();   
            max_delays.clear();
            circular_buffers.clear();
            parameters.clear();

            tick = 0;
            is_running = false;
            tick_is_running = false;
            time_usage = 0;
            idle_time = 0;

           // AddGroup("Untitled");     // FIXME: RESORE LATER

            first_request = true;
            session_id = std::time(nullptr);
        }


    void
    Kernel::Tick()
    {
        for(auto & m : components)
            try
            {
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

        tick++;
    }


    bool Kernel::Terminate()
    {
        return stop_after!= -1 &&  tick >= stop_after;
    }


    void Kernel::ScanClasses(std::string path)
    {
        int i=0;
        for(auto& p: std::filesystem::recursive_directory_iterator(path))
            if(std::string(p.path().extension())==".ikc")
            {
                std::string name = p.path().stem();
                classes[name] = Class(name, p.path());
            }
    }



    void Kernel::ScanFiles(std::string path)
    {
        int i=0;
        for(auto& p: std::filesystem::recursive_directory_iterator(path))
            if(std::string(p.path().extension())==".ikg")
            {
                std::string name = p.path().stem();
                 files[name] = p.path();
            }
    }


    void Kernel::ListClasses()
    {
        std::cout << "\nClasses:" << std::endl;
        for(auto & c : classes)
            c.second.print();
    }


    void Kernel::ResolveParameter(parameter & p,  std::string & name,  std::string & default_value)
    {
        std::size_t i = name.rfind(".");
        if(i == std::string::npos)
            return;

         auto c = components.at(name.substr(0, i));
        std::string parameter_name = name.substr(i+1, name.size());
        c->ResolveParameter(p, parameter_name, default_value);
    }


    void Kernel::ResolveParameters() // Find and evaluate value or default
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
                  c->ResolveParameter(p->second, parameter_name, p->second.default_value);
            }
        }
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

        std::cout << "\nDelays:" << std::endl;
        for(auto & o : buffers)
            std::cout  << "\t" << o.first <<": " << max_delays[o.first] << std::endl;
    }


    void Kernel::InitCircularBuffers()
    {
        std::cout << "\nInitCircularBuffers:" << std::endl;
        for(auto it : max_delays)
        {
            if(it.second <= 1)
                continue;
            std::cout << "\t" << it.first << std::endl;
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
            std::cout << "IKAROS: " << s << std::endl;
        log.clear();
    }


    Kernel::Kernel():
        tick(0),
        is_running(false),
        tick_is_running(false),
        time_usage(0),
        idle_time(0),
        stop_after(-1),
        tick_duration(0.01), // 10 ms
        webui_dir("Source/WebUI/") // FIXME: get from somewhere else
    {

            ScanClasses("Source/Modules");
            ScanFiles("Source/Modules");
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
        std::string type_string = params["type"];
        std::string default_value = params["default"];
        std::string options = params["options"];
        parameter p(type_string, default_value, options);
        parameters.emplace(name, p);
    }


    void Kernel::SetParameter(std::string name, std::string value)
    {
        if(parameters.count(name))
        {
            parameters[name] = value;
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
    }


    void Kernel::InitComponents()
    {
        // Call Init for all modules (after CalcalateSizes and Allocate)
        for(auto & c : components)
            try
            {
                c.second->Init();
            }
            catch(const fatal_error& e)
            {
                log.push_back("Fatal error. Init failed for \""+c.second->path_+"\": "+std::string(e.what())); // FIXME: set end of execution
            }
            catch(const std::exception& e)
            {
                log.push_back("Init failed for \""+c.second->path_+"\": "+std::string(e.what()));
            }
    }


    void Kernel::LoadFiles(std::vector<std::string> files, options & opts)
    {
            if(files.empty())
            {
                /*
                dictionary d;
                for(auto & x : opts.d)
                    d[x.first] = x.second;
                // AddGroup("Untitled", d); // FIXME: RESTORE LATER

                // FIXME: ONLY ONCE!!!!

                if(d.contains("webui_port"))
                    port = d["webui_port"];

                if(d.contains("start"))
                    start = is_true(d["start"]);

                if(d.contains("stop"))
                    stop_after = d["stop"];

                if(d.contains("tick_duration"))
                    tick_duration = d["tick_duration"];

                if(d.contains("real_time"))
                    if(is_true(d["real_time"]))
                        run_mode = run_mode_realtime;

                session_id = std::time(nullptr);
                */
                return;
            }

            // Load files

            for(auto & filename: files)
            try
                {
                    dictionary d(filename.c_str());

                    // Add command line arguments - will override XML - probably not correct ******************

                    for(auto & x : opts.d)
                        d[x.first] = x.second;

                        if(d.contains("webui_port"))
                            port = d["webui_port"];

                        if(d.contains("start"))
                            start = is_true(d["start"]);

                        if(d.contains("stop"))
                            stop_after = d["stop"];

                        if(d.contains("tick_duration"))
                            tick_duration = d["tick_duration"];

                        if(d.contains("real_time"))
                            if(is_true(d["real_time"]))
                                run_mode = run_mode_realtime;

                    // Build the network


                    BuildGroup(d);


                    info_ = d;

                    return; // Only use one file
                }
                catch(const std::exception& e)
                {
                    log.push_back(e.what());
                    throw;
                }
    
        session_id = std::time(nullptr); 
        SetUp();
    }


    void Kernel::Save()
    {

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


    void Kernel::InitSocket()
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


    void
    Kernel::SetUp()
    {
        ResolveParameters();
        CalculateDelays();
        CalculateSizes();
        InitCircularBuffers();
        InitComponents();
        /*
        InitComponents();
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


    void
    Kernel::Run(std::vector<std::string> files, options & opts)
    {
    LoadFiles(files, opts); // INIT

    InitSocket();
    SetUp();

    ListInputs();
    // Run loop
    // chdir(ikc_dir);

    timer.Restart();
    tick = 0;
    is_running = start;

        httpThread = new std::thread(Kernel::StartHTTPThread, this);

    while (!Terminate())
    {
        //std::cout << GetTick() << std::endl;

        if (!is_running)
        {
            // std::cout << "Idle: " << run_mode << std::endl;
            Sleep(0.01); // Wait 10ms to avoid wasting cycles if there are no requests
        }

        while(sending_ui_data)
            {}
        while(handling_request)
            {}

        if (is_running)
        {
            tick_is_running = true; // Flag that state changes are not allowed
            Tick(); // FIXME: Check activation status
            tick_is_running = false;
            
            // Calculate idle_time
            
            if(run_mode == run_mode_realtime)
            {
                idle_time = (float(tick*tick_duration) - timer.GetTime()) / float(tick_duration);
                time_usage = 1 - idle_time;
            }

            if (run_mode == run_mode_realtime)
            {
                lag = timer.WaitUntil(static_cast<double>(tick)*tick_duration);

                if(lag > lag_max)
                    lag_max = lag;

                if(lag < lag_min)
                    lag_min = lag;

                //if (lag > 0.1) Notify(msg_warning, "Lagging %.2f ms at tick = %ld\n", lag, tick);
            }
    /*            
            else if (run_mode == run_mode_realtime)
            {
                while(sending_ui_data)
                    {}
                while(handling_request)
                    {}
            }
    */
        }
    }
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
    Kernel::Pause()
    {
    is_running = false;
    while(tick_is_running)
        {}
    }



    long
    get_session_id(std::string args)
    {
    std::string id_string =  ::head(args, "&");
    std::string id_number = tail(id_string, "id=");

    try
    {
        return stol(id_number);
    }
    catch(const std::invalid_argument)
    {
        return 0;
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
        socket->Send("\t\"state\": %d,\n", run_mode);
        if(stop_after != -1)
        {
            socket->Send("\t\"tick\": \"%d / %d\",\n", GetTick(), stop_after);
            socket->Send("\t\"progress\": %f,\n", float(tick)/float(stop_after));
        }
        else
        {
            socket->Send("\t\"tick\": %lld,\n", GetTick());
            socket->Send("\t\"progress\": 0,\n");
        }

        // Timing information

        float total_time = 0; // timer->GetTime()/1000.0; // in seconds // FIXME: ---------

        socket->Send("\t\"timestamp\": %ld,\n", 0); // Timer::GetRealTime() // FIXME: -----------
        socket->Send("\t\"total_time\": %.2f,\n", total_time);
        socket->Send("\t\"ticks_per_s\": %.2f,\n", 1); // FIXME: float(tick)/total_time
        socket->Send("\t\"timebase\": %d,\n", tick_duration);
        socket->Send("\t\"timebase_actual\": %.0f,\n", tick > 0 ? 1000*float(total_time)/float(tick) : 0);
        socket->Send("\t\"lag\": %.0f,\n", lag);
        socket->Send("\t\"cpu_cores\": %d,\n", cpu_cores);
        socket->Send("\t\"time_usage\": %.3f,\n", time_usage);  // TODO: move to kernel from WebUI
        socket->Send("\t\"cpu_usage\": %.3f,\n", cpu_usage);
    }


    void
     Kernel::DoSendLog(std::string uri, std::string args)
     {
        socket->Send(",\n\"log\": [");
        std::string sep;
        for(std::string line : log)
        {
            socket->Send(sep + "\""+line+"\"");
        }
        socket->Send("]");
        log.clear();
     }

    void
    Kernel::DoSendData(std::string uri, std::string args)
    {    
        sending_ui_data = true; // must be set while main thread is still running
        while(tick_is_running)
            {}

        DoSendDataHeader();

        socket->Send("{\n");

        DoSendDataStatus();

        socket->Send("\t\"data\":\n\t{\n");
    
        std::string root = tail(args, "data=");
        std::string data = tail(root, "#");
        std::string sep = "";
        bool sent = false;

        while(!data.empty())
        {
            std::string source = head(data, "#");
            std::string format = rtail(source, ":");
            std::string source_with_root = root+"."+source;

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
        DoSendLog(uri, args);
        socket->Send(",\n\t\"has_data\": "+std::to_string(!tick_is_running)+"\n"); // new tick has started during sending; there may be data but it cannot be trusted
        socket->Send("}\n");

        sending_ui_data = false;
    }


    void
    Kernel::DoNew(std::string uri, std::string args)
    {
        Pause();
        run_mode = run_mode_stop;
        Clear();    // Remove all things
        DoUpdate(uri, args);
    }


    void
    Kernel::DoOpen(std::string uri, std::string args)
    {
        std::string file = peek_tail(args,"=");
        //std::cout << "Open: " << file << std::endl;
        Pause();
        run_mode = run_mode_stop;
        Clear();    // Remove all things
        options o; // FIXME: Use global options in kernel
        std::vector<std::string> files_;
        files_.push_back(files.at(file));
        LoadFiles(files_, o);
        DoUpdate(uri, args);
    }


    void
    Kernel::DoSave(std::string uri, std::string args)
    {
        Save();
    }


    void
    Kernel::DoSaveAs(std::string uri, std::string args)
    {
    }


    void
    Kernel::DoStop(std::string uri, std::string args)
    {
        log.push_back("stop");
        Pause();
        run_mode = run_mode_stop;
        DoUpdate(uri, args);
    }


    void
    Kernel::DoSendFile(std::string file)
    {
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
    Kernel::DoSendNetwork(std::string uri, std::string args)
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
    Kernel::DoPause(std::string uri, std::string args)
    {
    log.push_back("pause");
    Pause();
    run_mode = run_mode_pause;
    master_id = get_session_id(args);
    DoSendData(uri, args);
    }


    void
    Kernel::DoStep(std::string uri, std::string args)
    {
    log.push_back("step");
    Pause();
    run_mode = run_mode_pause;
    master_id = get_session_id(args);
    Tick();
    DoSendData(uri, args);
    }


    void
    Kernel::DoPlay(std::string uri, std::string args)
    {
    log.push_back("play");
    Pause();
    run_mode = run_mode_play;
    master_id = get_session_id(args);
    Tick();
    DoSendData(uri, args);
    }


    void
    Kernel::DoRealtime(std::string uri, std::string args)
    {
    log.push_back("realtime");
    run_mode = run_mode_realtime;
    master_id = get_session_id(args);
    is_running = true;
    DoSendData(uri, args);
    }


    void
    Kernel::DoCommand(std::string uri, std::string args)
    {
    /*
    float x, y;
    char command[255];
    char value[1024]; // FIXME: no range chacks
    int c = sscanf(uri.c_str(), "/command/%[^/]/%f/%f/%[^/]", command, &x, &y, value);
    if(c == 4)
        SendCommand(command, x, y, value);

        */
    DoSendData(uri, args);
    }


    void
    Kernel::DoControl(std::string uri, std::string args)
    {
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
                else // if(p.type == float_type) // FIXME: scalar type
                {
                    p = value;
                }
            }
            DoSendData(uri, args);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            DoSendData(uri, args);
        }
    }



    void
    Kernel::AddWidget(std::string uri, std::string args)
    {
        std::cout << "AddWidget: " << args << std::endl;

        DoSendData(uri, args);
    }


    void
    Kernel::DeleteWidget(std::string uri, std::string args)
    {
      std::cout << "DeleteWidget: " << args << std::endl;
    }


    void
    Kernel::SetWidgetParameter(std::string uri, std::string args)
    {
      std::cout << "SetWidgetParameter: " << args << std::endl;
    }


    void
    Kernel::DoUpdate(std::string uri, std::string args)
    {
        if(args.empty() || first_request) // not a data request - send view data
        {
            first_request = false;
            DoSendNetwork(uri, args);
        }
        else if(run_mode == run_mode_play && master_id == get_session_id(args))
        {
            Pause();
            Tick();
            DoSendData(uri, args);
        }
        else 
            DoSendData(uri, args);
    }


    void
    Kernel::DoNetwork(std::string uri, std::string args)
    {
        first_request = false;
        DoSendNetwork(uri, args);
    }


    void
    Kernel::DoSendClasses(std::string uri, std::string args)
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
    Kernel::DoSendFileList(std::string uri, std::string args)
    {
        Dictionary header;
        header.Set("Content-Type", "text/json");
        header.Set("Cache-Control", "no-cache");
        header.Set("Cache-Control", "no-store");
        header.Set("Pragma", "no-cache");
        socket->SendHTTPHeader(&header);
        std::string sep;
        socket->Send("{\"filelist\":\"");
        for(auto & f: files)
        {
            socket->Send(sep);
            socket->Send(f.first);
            sep = ",";
        }
        socket->Send("\"}");
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
    std::string uri = socket->header.Get("URI");
    if(uri.empty())
    {
        log.push_back("No URI"); // warning
        return;
    }

    std::string args = tail(uri, "?");

    // std::cout << uri << " " << args << std::endl;

    if(uri == "/update")
        DoUpdate(uri, args);
    else if(uri == "/network")
        DoNetwork(uri, args);
    else if(uri == "/pause")
        DoPause(uri, args);
    else if(uri == "/step")
        DoStep(uri, args);
    else if(uri == "/play")
        DoPlay(uri, args);
    else if(uri == "/realtime")
        DoRealtime(uri, args);
    else if(uri == "/new")
        DoNew(uri, args);
    else if(uri == "/open")
        DoOpen(uri, args);
    else if(uri == "/save")
        DoSave(uri, args);
    else if(uri == "/saveas")
        DoSaveAs(uri, args);
    else if(uri == "/stop")
        DoStop(uri, args);
    else if(uri == "/classes") 
        DoSendClasses(uri, args);
    else if(uri == "/filelist") 
        DoSendFileList(uri, args);
    else if(uri == "/")
        DoSendFile("index.html");
    else if(starts_with(uri, "/command/"))
        DoCommand(uri, args);
    else if(starts_with(uri, "/control/"))
        DoControl(uri, args);

    else if(starts_with(uri, "/addwidget"))
        AddWidget(uri, args);
    else if(starts_with(uri, "/delwidget"))
        DeleteWidget(uri, args);
    else if(starts_with(uri, "/setwidgetparam"))
        SetWidgetParameter(uri, args);

    else 
        DoSendFile(uri);
    }


    void
    Kernel::HandleHTTPThread()
    {
        while(!Terminate())
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

