// Ikaros 3.0

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
        info_(info), 
        type(no_type), 
        has_options(info_.contains("options"))
    {
        std::string type_string = info_["type"];

        if(type_string=="float" || type_string=="int" || type_string=="double")  // Temporary
            type_string = "number";

        auto type_index = std::find(parameter_strings.begin(), parameter_strings.end(), type_string);
        if(type_index == parameter_strings.end())
            throw exception("Unkown parameter type: "+type_string+".");

        type = parameter_type(std::distance(parameter_strings.begin(), type_index));

        // Init shared pointers
        switch(type)
        {
            case number_type:
            case rate_type:
            case bool_type:
                number_value = std::make_shared<double>(0); 
                break;

            case string_type: 
                string_value = std::make_shared<std::string>(""); 
                break;

            case matrix_type: 
                matrix_value = std::make_shared<matrix>(); 
                break;

            default: 
                break;
        } 
    }



    parameter::parameter(const std::string type, const std::string options):
        parameter(options.empty() ? dictionary({{"type", type}}) : dictionary({{"type", type},{"options", options}}))
    {}


    void 
    parameter::operator=(parameter & p) // this shares data with p 
    {
        info_ = p.info_;
        type = p.type;
        has_options = p.has_options;

        number_value = p.number_value;
        matrix_value = p.matrix_value;
        string_value = p.string_value;
    }



    double 
    parameter::operator=(double v) // FIXME: handle matrix type as well
    {
        if(has_options)
        {
            auto options = split(info_["options"],",");
            int c = options.size();
            if(v > c-1)
                v = c-1;
            switch(type)
            {
                case number_type:
                case rate_type:
                case bool_type:
                    *number_value = double(v);
                    break;
                case string_type:
                    *string_value = options[round(v)];
                    break;
                default:
                    break; // FIXME: error?
            }
            return v;
        }

        switch(type)
        {
            case number_type:
            case rate_type:
            case bool_type:
                *number_value = double(v);
                break;
            case string_type:
                    *string_value = std::to_string(v);
                break;
            default:
                break; // FIXME: error?
        }
        return v;
    }

    std::string 
    parameter::operator=(std::string v)
    {
        double val = 0;
        if(has_options)
        {
            auto options = split(info_["options"],",");
            auto it = std::find(options.begin(), options.end(), v);
            if (it != options.end())
                val = std::distance(options.begin(), it);
            else
                throw exception("Invalid parameter value");
        }
        else if(is_number(v))
            val = stod(v);

        switch(type)
        {
            case number_type:
            case rate_type:
                *number_value = val;
                break;

            case bool_type:
                *number_value = (val!=0 ? 1 : 0);
                break;

            case string_type:
                *string_value = v;
                break;

            case matrix_type:
                *matrix_value = v;
                break;

            default:
                break; // FIXME: error? Handle matrix with single element
        }
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
        if(string_value)
            return  *string_value;

        if(has_options)
        {
            int index = int(*number_value);
            auto options = split(info_["options"],","); // FIXME: Check trim in split
            if(index < 0 || index>= options.size())
                return std::to_string(index)+" (OUT-OF-RANGE)";
            else
                return options[index];
        } 

        switch(type)
        {
            case no_type: throw exception("Uninitialized or unbound parameter.");
            case number_type: if(number_value) return std::to_string(*number_value);
            case bool_type: if(number_value) return (*number_value>0 ? "true" : "false");
            case rate_type: if(number_value) return std::to_string(*number_value);
            case string_type: if(string_value) return *string_value;
            case matrix_type: return matrix_value->json();
            default:  throw exception("Type conversion error for parameter.");
        }
    }


    parameter::operator double()
    {
        if(type==rate_type)
            return *number_value * kernel().tick_duration;
        if(number_value) 
            return *number_value;
        else if(string_value) 
        {
            if(has_options)
            {
                auto options = split(info_["options"],","); // FIXME: Check trim in split
            auto it = std::find(options.begin(), options.end(), *string_value);
            if (it != options.end())
                return std::distance(options.begin(), it);
            else
                return 0;
            }
            else
                return stod(*string_value); // FIXME: may fail ************
        }
        else if(matrix_value)
            return 0;// FIXME check 1x1 matrix ************
        else
            throw exception("Type conversion error. Parameter does not have a type. Bind?");
    }


    int
    parameter::as_int()
    {
        switch(type)
        {
            case no_type: throw exception("Uninitialized_parameter.");
            case number_type: if(number_value) return *number_value;
            case rate_type: if(number_value) return *number_value;    // FIXME: Take care of time base
            case bool_type: if(number_value) return *number_value;
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


    void 
        parameter::print(std::string name)
    {
        if(!name.empty())
            std::cout << name << " = ";
        std::cout << std::string(*this) << std::endl;
    }

    void 
    parameter::info()
    {
        std::cout << "type: " << type << std::endl;
        std::cout << "default: " << info_["default"] << std::endl;
        std::cout << "has_options: " << has_options << std::endl;
        std::cout << "options: " << info_["options"] << std::endl;
        std::cout << "value: " << std::string(*this) << std::endl;
    }

    std::string 
    parameter::json()
    {
        switch(type)     // FIXME: remove if statements and use exception handling
        {
            case number_type:    if(number_value)   return "[["+std::to_string(*number_value)+"]]";
            case bool_type:     if(number_value)    return (*number_value!=0 ? "[[true]]" : "[[false]]");
            case rate_type:     if(number_value)    return "[["+std::to_string(*number_value)+"]]";
            case string_type:   if(string_value)    return "\""+*string_value+"\"";
            case matrix_type:   if(matrix_value)    return matrix_value->json();
            default:            throw exception("Cannot convert parameter to string");
        }
    }


    std::ostream& operator<<(std::ostream& os, parameter p) // FIXME: Handle options
    {
        switch(p.type)
        {
            case number_type:    if(p.number_value) os <<  *p.number_value; break; // FIXME: Is string conversion sufficient?
            case bool_type:     if(p.number_value) os <<  (*p.number_value == 0 ? "false" : "true"); break;
            case rate_type:    if(p.number_value) os <<  *p.number_value; break; 
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
            if(!v.empty())  // ****************** this does not work for string that are allowed to be empty
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
            Notify(msg_fatal_error, "ERROR: Could not resolve parameter \""s +name + "\" .", name);  
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
            return ""; // throw exception("Name not found"); // throw not_found_exception instead
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
// Throws and exception if value cannot be found
// Does not handle default values - this is done by parameters

    std::string 
    Component::GetValue(const std::string & path) 
    {     
        if(path.empty())
            return ""; // throw exception("Name not found"); // throw not_found_exception instead

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
                return ""; // throw exception("Name not found"); // throw not_found_exception instead 
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

    void Component::AddOutput(std::string name, int size, std::string description)
    {
        dictionary o = {
            {"name", name},
            {"size", std::to_string(size)},
            {"description", description},
            {"_tag", "output"}
        };
        list(info_["outputs"]).push_back(o);
        AddOutput(o);
    }

    void Component::ClearOutputs()
    {
       info_["outputs"] = list(); 
    }


    void Component::AddParameter(dictionary parameters)
    {
        try
        {         
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



    std::string
    Component::EvaluateVariable(const std::string & s)
    {
        parameter p;
        if(LookupParameter(p, s.substr(1)))
            return p;
        else
            return Evaluate(s);

    }


    int 
    Component::EvaluateIntExpression(std::string & s)
    {
        expression e = expression(s);
        std::map<std::string, std::string> vars;
        for(auto v : e.variables())
            vars[v] = EvaluateVariable(v);
        return expression(s).evaluate(vars);
    }


    std::vector<int> 
    Component::EvaluateSizeList(std::string & s) // return list of size from size list in string
    {
        //s = Evaluate(s, true); // FIXME: evaluate as string for s should probably be used in more places
        std::vector<int> shape;
        for(std::string e : split(s, ","))
        {
            if(ends_with(e, ".size")) // special case: use shape function on input and push each dimension on list
            {
                auto & x = rsplit(e, ".", 1);
                matrix m;
                Bind(m, x.at(0));
                for(auto d : m.shape())
                    shape.push_back(d);
            }
            else
            {
                int d = EvaluateIntExpression(e);
                if(d>0)
                    shape.push_back(d);
            }
        }
        return shape;
    }



// NEW EVALUATION FUNCTIONS

    double 
    Component::EvaluateNumber(std::string v)
    {
        return stod(v); // FIXME: Add full parsing of expressions and variables *************
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

        if(!info_.contains("log_level"))
            info_["log_level"] = "5";

    // Set parent

        auto p = path_.rfind('.');
        if(p != std::string::npos)
            parent_ = kernel().components.at(path_.substr(0, p));
    }


    bool
    Component::Notify(int msg, std::string message, std::string path)
    {
        int ll = msg_warning;
        if(info_.contains("log_level"))
            ll = info_["log_level"];

        if(msg <= ll) 
             return kernel().Notify(msg, message);
        return true;
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


    void 
    Component::SetSourceRanges(const std::string & name, std::vector<Connection *> & ingoing_connections)
    {
        for(auto & c : ingoing_connections) // Copy source size to source_range if not set
        {
            if(c->source_range.empty())
                c->source_range = kernel().buffers[c->source].get_range();
            else if(c->source_range.rank() != kernel().buffers[c->source].rank())
                throw exception("Explicitly set source range dimensionality does not match source.");
        }

    }


    void 
    Component::SetInputSize_Flat(const std::string & name, std::vector<Connection *> & ingoing_connections, bool use_alias)
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


    void 
    Component::SetInputSize_Index(const std::string & name, std::vector<Connection *> & ingoing_connections, bool use_alias)
    {
            SetSourceRanges(name, ingoing_connections);
            int max_delay = 0;
            bool first_ingoing_connection = true;
            for(auto & c : ingoing_connections) // STEP 0b: copy source_range to target_range if not set
            {
                if(!c->delay_range_.empty() && c->delay_range_.trim().b_[0] > max_delay)
                    max_delay = c->delay_range_.trim().b_[0];

                if(c->target_range.empty())
                {
                    if(!first_ingoing_connection)
                        throw exception("Target ranges must be set explicitly for multiple connections to \""+name+"\".");
                    c->target_range = c->source_range;
                }
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

                first_ingoing_connection = false;
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


    void 
    Component::SetInputSize(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections)
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

            if(dictionary(d).contains("size")) // FIXME: use automatic conversions
            {
                std::string s = d["size"];
                std::vector<int> shape = EvaluateSizeList(s);
                matrix o;
                Bind(o, n); // FIXME: Get directly?
                if(o.rank() != 0)
                    outputs_with_size++; // Count number of buffers that are set
                
                for(int i=0; i < shape.size(); i++)
                    if(shape[i]<0)
                    {
                        kernel().Notify(msg_fatal_error, "Output "s+n+" has negative size for dimension "+std::to_string(i), n); // FIXME: throw
                        return 0;
                    }

                o.realloc(shape);
            }
            else // Group output: look for connection - only one-to-one right now // FIXME: combine connections to this output
            {
                std::string full_name = path_ +"."+n;
                if(ingoing_connections.count(full_name))
                {
                    auto c = ingoing_connections[full_name];
                    if(c.size() != 1)
                        throw exception("Cannot set group output size. There must be exactly one connection to an output.");// FIXME: add proper error handling that continues with the others
                    matrix s = kernel().buffers[c[0]->source];
                    if(s.rank() != 0)
                    {
                        kernel().buffers[full_name].realloc(s.shape());
                        c[0]->source_range = kernel().buffers[full_name].get_range(); // FIME: use range from shape
                        c[0]->target_range = kernel().buffers[full_name].get_range();
                        outputs_with_size++; // Count number of buffers that are set
                    }
                int i = 0;
                }
            }

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
    Connection::Tick()
    {
        auto & k = kernel();

        if(delay_range_.is_delay_0())
        {
            k.buffers[target].copy(k.buffers[source], target_range, source_range);
            //std::cout << source << " =0=> " << target << std::endl; 
        }
        else if(delay_range_.empty() || delay_range_.is_delay_1())
        {
            //std::cout << source << " =1=> " << target << std::endl; 
            k.buffers[target].copy(k.buffers[source], target_range, source_range);
        }

        else if(flatten_) // Copy flattened delayed values
        {
            //std::cout << source << " =F=> " << target << std::endl; 
            matrix ctarget = k.buffers[target];
            int target_offset = target_range.a_[0];
            for(int i=delay_range_.a_[0]; i<delay_range_.b_[0]; i++)  // FIXME: assuming continous range (inc==1)
            {   
                matrix s = k.circular_buffers[source].get(i);

                for(auto ix=source_range; ix.more(); ix++)
                {
                    int source_index = s.compute_index(ix.index());
                    ctarget[target_offset++] = (*(s.data_))[source_index];
                }
            }
        }

        else if(delay_range_.a_[0]+1 == delay_range_.b_[0]) // Copy indexed delayed value with single delay
        {
            //std::cout << source << " =D=> " << target << std::endl;
            matrix s = k.circular_buffers[source].get(delay_range_.a_[0]);
            k.buffers[target].copy(s, target_range, source_range);
        }

        else // Copy indexed delayed values with more than one element
        {
            //std::cout << source << " =DD=> " << target << std::endl;
            for(int i=delay_range_.a_[0]; i<delay_range_.b_[0]; i++)  // FIXME: assuming continous range (inc==1)
            {   
                matrix s = k.circular_buffers[source].get(i);
                int target_ix = i - delay_range_.a_[0]; // trim!
                range tr = target_range.tail();
                matrix t = k.buffers[target][target_ix];
                t.copy(s, tr, source_range);

            }
        }
    };

    void
    Connection::Print()
    {
        std::cout << "\t" << source <<  delay_range_.curly() <<  std::string(source_range) << " => " << target  << std::string(target_range);
        if(!alias_.empty())
            std::cout << " \"" << alias_ << "\"";
        std::cout << '\n'; 
    }


std::string
Connection::Info()
{
    std::string s = source + delay_range_.curly() +  std::string(source_range) + " => " + target  + std::string(target_range);
        if(!alias_.empty())
            s+=  " \"" + alias_ + "\"";
    return s;
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
    Class::Print()
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
        //std::cout << params << std::endl;
        command = head(uri, "/"); 
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
        //std::cout <<" Tick: " << GetTick() << std::endl;


            for(auto & task_group : tasks)
                for(auto & task: task_group)
                    task->Tick();

/*
        }
        else
        {

            for(auto & m : components)
                try
                {
                    //std::cout <<" Tick: " << m.second->info_["name"] << std::endl;
                    if(m.second != nullptr)  // Allow classes without code
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
        }
*/
        RotateBuffers();
        Propagate();

        CalculateCPUUsage();
        tick_is_running = false; // Flag that state changes are allowed again
    }


    bool 
    Kernel::Terminate()
    {
        if(stop_after!= -1 &&  tick >= stop_after)
        {
            if(options_.is_set("batch_mode"))
                run_mode = run_mode_quit;
            else
                run_mode = run_mode_pause;
            
        }
        return (stop_after!= -1 &&  tick >= stop_after);
    }


    void 
    Kernel::ScanClasses(std::string path) // FIXME: Add error handling
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


    void 
    Kernel::ScanFiles(std::string path, bool system)
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


    void 
    Kernel::ListClasses()
    {
        std::cout << "\nClasses:" << std::endl;
        for(auto & c : classes)
            c.second.Print();
    }


    void 
    Kernel::ResolveParameter(parameter & p,  std::string & name)
    {
        std::size_t i = name.rfind(".");
        if(i == std::string::npos)
            return; // FIXME: Is this an error?

        auto c = components.at(name.substr(0, i));
        std::string parameter_name = name.substr(i+1, name.size());
        c->ResolveParameter(p, parameter_name);
    }


    void 
    Kernel::ResolveParameters() // Find and evaluate value or default // FIXME: return success
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


    void 
    Kernel::CalculateSizes()
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


    void 
    Kernel::CalculateDelays()
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
    }


    void 
    Kernel::InitCircularBuffers()
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


    void 
    Kernel::RotateBuffers()
    {
        for(auto & it : circular_buffers)
            it.second.rotate(buffers[it.first]);
    }


    void 
    Kernel::ListComponents()
    {
        std::cout << "\nComponents:" << std::endl;
        for(auto & m : components)
            m.second->print();
    }


    void 
    Kernel::ListConnections()
    {
        std::cout << "\nConnections:" << std::endl;
        for(auto & c : connections)
            c.Print();
    }


    void 
    Kernel::ListInputs()
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


    void 
    Kernel::ListBuffers()
    {
        std::cout << "\nBuffers:" << std::endl;
        for(auto & i : buffers)
            std::cout << "\t" << i.first <<  i.second.shape() << std::endl;
    }


    void 
    Kernel::ListCircularBuffers()
    {
        std::cout << "\nCircularBuffers:" << std::endl;
        for(auto & i : circular_buffers)
            std::cout << "\t" << i.first <<  " " << i.second.buffer_.size() << std::endl;
    }


   void 
   Kernel::ListParameters()
    {
        std::cout << "\nParameters:" << std::endl;
        for(auto & p : parameters)
            std::cout  << "\t" << p.first << ": " << p.second << std::endl;
    }


    void 
    Kernel::PrintLog()
    {
        for(auto & s : log)
            std::cout << "IKAROS: " << s.level_ << ": " << s.message_ << std::endl;
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
    {
        cpu_cores = std::thread::hardware_concurrency();
    }

    // Functions for creating the network

    void 
    Kernel::AddInput(std::string name, dictionary parameters) // FIXME: use name as argument insteas of parameters
    {
        buffers[name] = matrix().set_name(parameters["name"]);
    }

    void 
    Kernel::AddOutput(std::string name, dictionary parameters)
    {
        buffers[name] = matrix().set_name(parameters["name"]);
    }

    void 
    Kernel::AddParameter(std::string name, dictionary params)
    {
         parameters.emplace(name, parameter(params));
    }


    void 
    Kernel::SetParameter(std::string name, std::string value)
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


    void 
    Kernel::AddGroup(dictionary info, std::string path)
    {
        current_component_info = info;
        current_component_path = path;

        if(components.count(current_component_path)> 0)
            throw exception("Module or group named \""+current_component_path+"\" already exists.");

        components[current_component_path] = new Group(); // Implicit argument passing as for components
    }


    void 
    Kernel::AddModule(dictionary info, std::string path)
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
        {
            if(info.is_not_set("no_code"))
             std::cout << "Class \""<< classname << "\" has no installed code. Creating group." << std::endl; // throw exception("Class \""+classname+"\" has no installed code. Check that it is included in CMakeLists.txt."); // TODO: Check that this works for classes that are allowed to have no code
            info["_tag"]="module";
            BuildGroup(info, path); // FIXME: This is probably not working correctly
        }
        else
            components[current_component_path] = classes[classname].module_creator();
    }


    void 
    Kernel::AddConnection(dictionary info, std::string path)
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



    void Kernel::LoadExternalGroup(dictionary d)
    {
        std::string path = d["external"];
        dictionary external(path);
        external["name"] = d["name"]; // FIXME: Just in case - check for errors later
        d.merge(external);
        
    }



    void 
    Kernel::BuildGroup(dictionary d, std::string path) // Traverse dictionary and build all items at each level, FIXME: rename AddGroup later
    {
        if(!d.contains("name"))
            throw fatal_error("Groups must have a name.");

        std::string name = validate_identifier(d["name"]);
        if(!path.empty())
            name = path+"."+name;

        if(d.contains("external"))
            LoadExternalGroup(d);


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


    void 
    Kernel::InitComponents()
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
                throw fatal_error(u8"Fatal error. Init failed for \""+c.second->path_+"\": "+std::string(e.what()));
            }
            catch(const std::exception& e)
            {
                throw fatal_error(u8"Init failed for "+c.second->path_+": "+std::string(e.what()));
            }
    }


    void 
    Kernel::SetCommandLineParameters(dictionary & d) // Add command line arguments - will override XML - probably not correct ******************
    {
    
        for(auto & x : options_.d)
            d[x.first] = x.second;

        d["filename"] = options_.filename;

        if(d.contains("stop"))
            stop_after = d["stop"];

        if(d.contains("tick_duration"))
            tick_duration = d["tick_duration"];
    }


    void 
    Kernel::LoadFile()
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
                Notify(msg_print, u8"Loaded "s+options_.filename);

                CalculateCheckSum();
                //ListBuffers();
                //ListConnections();
            }
            catch(const exception& e)
            {
                Notify(msg_fatal_error, e.message);
                Notify(msg_fatal_error, u8"Load file failed for "s+options_.filename);
                CalculateCheckSum();
                New();
            }
    }


    void 
    Kernel::CalculateCheckSum()
    {
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


     dictionary 
     Kernel::GetModuleInstantiationInfo()
     {
        dictionary d;
        d["Constant"] = "1";
        d["Print"] = "2";
        return d;
     }


    void 
    Kernel::Save() // Simple save function in present file
    {
        //std::cout << "Kernel::Save" << std::endl;
        std::string data = xml();

        //std::cout << data << std::endl;

        std::ofstream file;
        std::string filename = add_extension(info_["filename"], ".ikg");
        file.open (filename);
        file << data;
        file.close();
    }


   
    void 
    Kernel::Propagate()
    {

         for(auto & c : connections)
            if(c.delay_range_.is_delay_0())
                continue;
            else
                c.Tick();

   /*
         for(auto & c : connections)
        {
            if(c.delay_range_.empty() || c.delay_range_.is_delay_0())
            {
                // Do not handle here. Handled in Connection.Tick()
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
    */
    }



    void
    Kernel::InitSocket(long port)
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
    Kernel::PruneConnections()
    {
        for (auto it = connections.begin(); it != connections.end(); ) 
        {
            if (buffers.count(it->source) && buffers.count(it->target))
                it++;
            else
            {
                Notify(msg_print, u8"Pruning "  + it->source + "=>" + it->target);
                it = connections.erase(it);
            }
        }
    }


/*************************
 * 
 *  Task sorting
 * 
 *************************/



    bool 
    Kernel::dfsCycleCheck(const std::string& node, const std::unordered_map<std::string, std::vector<std::string>>& graph, std::unordered_set<std::string>& visited, std::unordered_set<std::string>& recStack) 
    {
        if (recStack.find(node) != recStack.end())
            return true;

        if (visited.find(node) != visited.end())
            return false;

        visited.insert(node);
        recStack.insert(node);

        if (graph.find(node) != graph.end()) 
            for (const std::string& neighbor : graph.at(node)) 
                if (dfsCycleCheck(neighbor, graph, visited, recStack)) 
                    return true;
        recStack.erase(node);
        return false;
    }



    bool 
    Kernel::hasCycle(const std::vector<std::string>& nodes, const std::vector<std::pair<std::string, std::string>>& edges) 
    {
        std::unordered_map<std::string, std::vector<std::string>> graph;
        for (const auto& edge : edges)
            graph[edge.first].push_back(edge.second);

        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> recStack;

        for (const std::string& node : nodes)
            if (visited.find(node) == visited.end() &&  (dfsCycleCheck(node, graph, visited, recStack)))
                    return true;

        return false;
    }

    void 
    Kernel::dfsSubgroup(const std::string& node, const std::unordered_map<std::string, std::vector<std::string>>& graph, std::unordered_set<std::string>& visited, std::vector<std::string>& component) 
    {
        visited.insert(node);
        component.push_back(node);

        if (graph.find(node) != graph.end()) 
        {
            for (const std::string& neighbor : graph.at(node)) 
            {
                if (visited.find(neighbor) == visited.end()) 
                    dfsSubgroup(neighbor, graph, visited, component);
            }
        }
    }


    std::vector<std::vector<std::string>> 
    Kernel::findSubgraphs(const std::vector<std::string>& nodes, const std::vector<std::pair<std::string, std::string>>& edges) 
    {
        std::unordered_map<std::string, std::vector<std::string>> graph;
        for (const auto& edge : edges) {
            graph[edge.first].push_back(edge.second);
            graph[edge.second].push_back(edge.first);
        }

        std::unordered_set<std::string> visited;
        std::vector<std::vector<std::string>> components;

        for (const std::string& node : nodes) {
            if (visited.find(node) == visited.end()) {
                std::vector<std::string> component;
                dfsSubgroup(node, graph, visited, component);
                components.push_back(component);
            }
        }

        return components;
    }


    void 
    Kernel::topologicalSortUtil(const std::string& node, const std::unordered_map<std::string, std::vector<std::string>>& graph, std::unordered_set<std::string>& visited, std::stack<std::string>& Stack) 
    {
        visited.insert(node);

        if (graph.find(node) != graph.end()) {
            for (const std::string& neighbor : graph.at(node)) {
                if (visited.find(neighbor) == visited.end()) {
                    topologicalSortUtil(neighbor, graph, visited, Stack);
                }
            }
        }
        Stack.push(node);
    }



    std::vector<std::string> 
    Kernel::topologicalSort(const std::vector<std::string>& component, const std::unordered_map<std::string, std::vector<std::string>>& graph) 
    {
        std::unordered_set<std::string> visited;
        std::stack<std::string> Stack;

        for (const std::string& node : component) 
            if (visited.find(node) == visited.end())
                topologicalSortUtil(node, graph, visited, Stack);

        std::vector<std::string> sortedSubgraph;
        while (!Stack.empty()) 
        {
            sortedSubgraph.push_back(Stack.top());
            Stack.pop();
        }

        return sortedSubgraph;
    }


    std::vector<std::vector<std::string>>
    Kernel::sort(std::vector<std::string> nodes, std::vector<std::pair<std::string, std::string>> edges)
    {
            if (hasCycle(nodes, edges)) 
            {
            std::cout << "Graph has a cycle" << std::endl;
            throw;
        } else 
        {

            std::vector<std::vector<std::string>> components = findSubgraphs(nodes, edges);

            // Rebuild the original graph for directed edges
            std::unordered_map<std::string, std::vector<std::string>> graph;
            for (const auto& edge : edges) {
                graph[edge.first].push_back(edge.second);
            }

            std::vector<std::vector<std::string>>  result;

            for (const auto& component : components) {
                std::vector<std::string> sortedSubgraph = topologicalSort(component, graph);
                result.push_back(sortedSubgraph);
                for (const auto& node : sortedSubgraph) {
                }
                std::cout << std::endl;
            }

            return result;
        }
    }



    void
    Kernel::SortTasks()
    {
        std::vector<std::string> nodes;
        std::vector<std::pair<std::string, std::string>> arcs;
        std::map<std::string, Task *> task_map;

        for(auto & [s,c] : components)
        {
            nodes.push_back(s);
            task_map[s] = c; // Save in task map
        }

        for(auto & c : connections) // ONLY ZERO CONNECTIONS
        if(c.delay_range_.is_delay_0())
            {
                std::string s = peek_rhead(c.source,".");
                std::string t = peek_rhead(c.target,".");
                std::string cc = "CON("+s+","+t+")"; // Connection node name

                nodes.push_back(cc);
                arcs.push_back({s, cc});
                arcs.push_back({cc, t});
                task_map[cc] = &c; // Save in task map
            }
/*
       for(auto & s : nodes)
            std::cout <<  "NODE: " << s << std::endl;
       for(const auto [s,t] : arcs)
            std::cout <<  "ARC: " << s << "->" << t << std::endl;
*/
        auto r = sort(nodes, arcs);

/*
        for(auto s : r)
        {
            for(auto ss: s)
                std::cout << ss << " ";
                std::cout  << std::endl;
        }
        std::cout  << std::endl;
*/
        // Fill task list

        tasks.clear();
        for(auto s : r)
        {
            std::vector<Task *> task_list;
            for(auto ss: s)
                task_list.push_back(task_map[ss]);   // Get task ponter here
            tasks.push_back(task_list);
        }

/*
        // Push delayed connections

        for(auto & c : connections)
            if(!c.delay_range_.is_delay_0())
            {
                std::vector<Task *> connection_task_list;
                connection_task_list.push_back(&c);
                tasks.push_back(connection_task_list);
            }
*/
//        std::cout << "SORTING COMPLETE" << std::endl;



    }



    void
    Kernel::RunTasks()
    {
        for(auto & task_group : tasks)
            for(auto & task: task_group)
                task->Tick();
    }



    void
    Kernel::SetUp()
    {
        try
        {
            SortTasks();
            ResolveParameters();
            CalculateDelays();
            CalculateSizes();

            PruneConnections();
            InitCircularBuffers();
            InitComponents();

            if(info_.is_set("info"))
            {
                ListParameters();
                ListComponents();
                ListConnections();
                ListInputs();
                ListOutputs();
                ListBuffers();
                ListCircularBuffers();
            }

            PrintLog();
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

        // Check start-up arguments

        if(info_.is_set("start"))
        {
            if(info_.is_set("realtime"))
                run_mode = run_mode_restart_realtime;
            else
                run_mode = run_mode_restart_play;
        }
        
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
                        timer.SetTime(double(tick+1)*tick_duration); // Fake time increase // FIXME: remove sleep in batch mode
                        Sleep(0.01);
                    }
                    else
                        Sleep(0.01); // Wait 10 ms to avoid wasting cycles if there are no requests

                    // Run_mode may have changed during the delay - needs to be checked again

                    if(run_mode == run_mode_realtime || run_mode == run_mode_play) 
                    {
                        actual_tick_duration = intra_tick_timer.GetTime();
                        intra_tick_timer.Restart();
                        try
                        {
                            Tick();
                        }
                        catch(std::exception & e)
                        {
                            //std::cout << e.what() << std::endl;
                            Notify(msg_fatal_error, (e.what()));
                            return;
                        }
                        tick_time_usage = intra_tick_timer.GetTime();
                        idle_time = tick_duration - tick_time_usage;
                    }                    
                }
                Sleep(0.1);
            }
        }

        bool
        Kernel::Notify(int msg, std::string message, std::string path)
        {
            log.push_back(Message(msg, message, path));
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
        return info_.json();
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

        std::string data = request.parameters["data"];  // FIXME: Check that it exists ******** or return ""
        std::string root;
            if(request.parameters.contains("root"))
                root = std::string(request.parameters["root"]);

        std::string sep = "";
        bool sent = false;

        while(!data.empty())
        {
            std::string source = head(data, ",");
            std::string format = rtail(source, ":");
            std::string source_with_root = root +"."+source; // request.component_path

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
        rtheader.Set("Package-Type", "network"); // FIXME: wrong packade type
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
        Notify(msg_print, "quit");
        Stop();
        run_mode = run_mode_quit;
        DoUpdate(request);
    }


    void
    Kernel::DoStop(Request & request)
    {
        Notify(msg_print, "stop");
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
        Notify(msg_print, "pause");
        Pause();
        DoSendData(request);
    }


    void
    Kernel::DoStep(Request & request)
    {
        Notify(msg_print, "step");
        Pause();
        run_mode = run_mode_pause;
        Tick();
        timer.SetPauseTime(GetTime()+tick_duration);
        DoSendData(request);
    }




    void
    Kernel::DoRealtime(Request & request)
    {
        Notify(msg_print, "realtime");
        Realtime();
        DoSendData(request);
    }


    void
    Kernel::DoPlay(Request & request)
    {
        Notify(msg_print, "play");
        Play();
        DoSendData(request);
    }


    void
    Kernel::DoCommand(Request & request)
    {
        if(!components.count(request.component_path))
            {
                Notify(msg_warning, "Component '"+request.component_path+"' could not be found.");
                DoSendData(request);
                return;
            }

        if(!request.body.empty()) // FIXME: Move to request and check content-type first
        {
            request.parameters = parse_json(request.body);
        }

        if(!request.parameters.contains("command"))
        {
                Notify(msg_warning, "No command specified for  '"+request.component_path+"'.");
                DoSendData(request);
                return;
        }

        components.at(request.component_path)->Command(request.parameters["command"], request.parameters);
        DoSendData(request);
    }



    void
    Kernel::DoControl(Request & request)
    {
        try
        {
            std::string root;
         
            if(request.parameters.contains("root"))
                root= std::string(request.parameters["root"]);

            if(!parameters.count(request.component_path))
            {
                Notify(msg_warning, "Parameter '"+request.component_path+"' could not be found.");
                DoSendData(request);
                return;
            }

            parameter & p = parameters.at(request.component_path);
            if(p.type == matrix_type)
            {
                int x = 0;
                int y = 0;
                double value = 1;

                if(request.parameters.contains("x"))
                    x = request.parameters["x"];

                if(request.parameters.contains("y"))
                    y = request.parameters["y"];
                
                if(request.parameters.contains("value"))
                    value = request.parameters["value"];
                    
                if(p.matrix_value->rank() == 1)
                    (*p.matrix_value)(x)= value;
                else if(p.matrix_value->rank() == 2)
                    (*p.matrix_value)(x,y)= value;
                else
                    ;   // FIXME: higher dimensional parameter
            }
            else
            {
                p = std::string(request.parameters["value"]);
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            DoSendData(request);
        }
    }

/*
    void
    Kernel::AddWidget(Request & request) // FIXME: Local exception handling
    {
        std::cout << "AddWidget: " << std::endl;
        auto view = GetView(request);
        if(view["widgets"].is_null())
            view["widgets"] = list();

        list u = list(view["widgets"]);
        u.push_back(request.parameters);


        DoSendData(request);
    }


    void
    Kernel::DeleteWidget(Request & request)
    {
        //std::cout << "DeleteWidget: "  << std::endl;

        int index = std::stoi(request.parameters["index"]);
        list view = list(GetView(request)["widgets"]);
        view.erase(index);

        int i=0;
        for(auto & w : view)
            w["_index_"] = i++;

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

        //list u = list(GetView(request)["widgets"]);

        dictionary d = u[index];
        u.erase(index);
        u.push_back(d);
    
        int i=0;
        for(auto & item : u)
            item["_index_"] = i++;


        DoSendData(request);
     }



    void
    Kernel::WidgetToBack(Request & request)
    {
        //std::cout << "SetWidgetToBack: " << std::endl;

        int index = request.parameters.get_int("index");

        list u = list(GetView(request)["widgets"]);

        dictionary d = u[index];
        u.erase(index);
        u.insert_front(d);

        int i=0;
        for(auto & item : u)
            item["_index_"] = i++;

        DoSendData(request);
    }
    void
    Kernel::RenameView(Request & request)
    {
        std::cout << "RenameView: " << std::endl;

        dictionary u = GetView(request);
        u["name"] = request.parameters["name"];

        DoSendData(request);
    }
*/

/*
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
*/

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