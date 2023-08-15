// Ikaros 3.0

#include "ikaros.h"

using namespace ikaros;

namespace ikaros
{
std::string  validate_identifier(std::string s)
{
    static std::string legal = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_1234567890";
    if(s.empty())
        throw exception("Identifier cannot be empty string");
    if('0' <= s[0] && s[0] <= '9')
        throw exception("Identifier cannot start with a number: "+s);
    for(auto c : s)
        if(legal.find(c) == std::string::npos)
            throw exception("Illegal character in identifier: "+s);
    return s;
}


    void Component::Bind(parameter & p, std::string name)
    {
        Kernel & k = kernel();
        std::string pname = name_+"."+name;
        if(k.parameters.count(pname))
            p = kernel().parameters[pname];
        else
            throw exception("Cannot bind to \""+name+"\"");
    };


    void Component::Bind(matrix & m, std::string n) // Bind input, output or parameter
    {
        std::string name = name_+"."+n;
        try
        {
            Kernel & k = kernel();
            if(k.inputs.count(name))
                m = k.inputs[name];
            else if(k.outputs.count(name))
                m = k.outputs[name];
            else if(k.parameters.count(name))
                m = (matrix &)(k.parameters[name]);
            else if(k.parameters.count(name))
                throw exception("Cannot bind to attribute \""+name+"\". Define it as a parameter!");
            else
                throw exception("Input or output named \""+name+"\" does not exist");
        }
        catch(exception e)
        {
            throw exception("Bind:\""+name+"\" failed. "+e.message);
        }
    }

    void Component::AddInput(dictionary parameters)
    {
        std::string input_name = std::string(info_["name"])+"."+validate_identifier(parameters["name"]);
        kernel().AddInput(input_name, parameters);
    }

    void Component::AddOutput(dictionary parameters)
    {
        std::string output_name = std::string(info_["name"])+"."+validate_identifier(parameters["name"]);
        kernel().AddOutput(output_name, parameters);
      };

    void Component::AddParameter(dictionary parameters)
    {
        std::string parameter_name;
        try
        {
            std::string pn = parameters["name"];
            if(pn=="name")
                return;
            if(pn=="class")
                return;            
            parameter_name = std::string(info_["name"])+"."+validate_identifier(pn);
            kernel().AddParameter(parameter_name, parameters);
            }
        catch(const std::exception& e)
        {
            throw exception("While adding parameter \""+parameter_name+"\": "+ e.what());
        }
    }


    void Component::SetParameter(std::string name, std::string value)
    {
        if(name=="name")
            return;
        if(name=="class")
            return;  
        std::string parameter_name = std::string(info_["name"])+"."+validate_identifier(name);
        kernel().SetParameter(parameter_name, value);
      }


    bool Component::LookupParameter(parameter & p, const std::string & name)
    {
        Kernel & k = kernel();
        if(k.parameters.count(name_+"."+name))
        {
            p = k.parameters[name_+"."+name];
            return true;
        }
        else if(parent_)
            return parent_->LookupParameter(p, name);
        else
            return false;
    }


/*
std::string Component::Lookup(const std::string & name) const // FIXME: Should this funciton throw????
{
    std::string component_name = name_;
    while (true)
    {
        auto it = kernel().parameters.find(component_name + "." + name);
        if (it != kernel().parameters.end() && it->second.is_set)
            return it->second;
        int p = component_name.rfind('.');
        if(p == std::string::npos)
            return std::string();
        component_name = component_name.substr(0, p);
    }
}


// Recursive

std::string Component::Lookup(const std::string & name) const
{
    auto it = kernel().parameters.find(name_ + "." + name);
    if (it != kernel().parameters.end() && it->second.is_set)
        return it->second;
    else if(parent)
        return parent->Lookup(name)
    else 
        return "";
    }
}
*/


    Component * Component::GetComponent(const std::string & s) // Get component; sensitive to variables and indirection
    {
        std::string path = SubstituteVariables(s);
        try
        {        
            std::string aaa = rhead(name_,'.');
            std::string xxx = rtail(rhead(name_,'.'),'.');

            if(path.empty()) // this
                return this;
            if(path[0]=='.') // global
                return kernel().components.at(path.substr(1));
            if(kernel().components.count(name_+"."+head(path,'.'))) // inside
                return kernel().components[name_+"."+head(path,'.')]->GetComponent(tail(path,'.'));
            if(rtail(rhead(name_,'.'),'.') == head(path,'.') && parent_) // parent
                return parent_->GetComponent(tail(path,'.'));
            throw exception("Component does not exist.");
        }
        catch(const std::exception& e)
        {
            throw exception("Component \""+path+"\" does not exist.");
        }
    }



  Component::Component():
    parent_(nullptr),
    info_(kernel().current_component_info),
    name_(info_["name"])
{
    for(auto p: info_["parameters"])
        AddParameter(p["attributes"]);

    for(auto & input: info_["inputs"])
        AddInput(input["attributes"]);

    for(auto & output: info_["outputs"])
        AddOutput(output["attributes"]);

    // Set parent

    auto p = name_.rfind('.');
    if(p != std::string::npos)
        parent_ = kernel().components.at(name_.substr(0, p));
}

  Module::Module()
    {
    // Copy module attributes into info structure taht already contains class attributes
        for(auto p: dictionary(kernel().current_module_info["attributes"]))
            info_["attributes"][p.first] = p.second;

    }


// The following lines will create the kernel the first time it is accessed by one of the components

    Kernel& kernel()
    {
        static Kernel * kernelInstance = new Kernel();
        return *kernelInstance;
    }
}; // namespace ikaros


bool
Component::InputsReady(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections) // FIXME: Handle optional inputs
{
    Kernel& k = kernel();

    std::string n = d["attributes"]["name"];
    for(auto & c : ingoing_connections[name_+'.'+n])
        if(k.outputs.at(c->source).rank()==0)
            return false;
    return true;
}


void Component::SetSourceRanges(const std::string & name, std::vector<Connection *> & ingoing_connections)
{
    for(auto & c : ingoing_connections) // Copy source size to source_range if not set
    {
        if(c->source_range.empty())
            c->source_range = kernel().outputs[c->source].get_range();
        else if(c->source_range.rank() != kernel().outputs[c->source].rank())
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
    kernel().inputs[name].realloc(flattened_input_size);

    if(!add_labels)
        return;

    begin_index = 0;
    for(auto & c : ingoing_connections)
    {
        int s = c->source_range.size() * c->delay_range_.trim().b_[0];
        if(c->alias_.empty())
            kernel().inputs[name].push_label(0, c->source, s);
        else
            kernel().inputs[name].push_label(0, c->alias_, s);
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

            if(!c->delay_range_.empty())
                 c->target_range.push_front(0, c->delay_range_.trim().b_[0]);
        }

    range r;

    for(auto & c : ingoing_connections)  // STEP 1: Calculate range extent
        r |= c->target_range;

    //if(max_delay > 1)
    //    r.push_front(0, max_delay);

    kernel().inputs[name].realloc(r.extent());  // STEP 2: Set input size
}


void Component::SetInputSize(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections)
{
        Kernel& k = kernel();
        std::string input_name = name_ + "." + std::string(d["attributes"]["name"]);

// FIXME: Use input type heuristics here ************

        std::string add_labels = d["attributes"]["add_labels"];

        std::string flatten = d["attributes"]["flatten"];
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

    for(auto & d : info_["inputs"])
        if(k.inputs[name_+"."+std::string(d["attributes"]["name"])].empty())
        {   
             if(InputsReady(dictionary(d), ingoing_connections))
                SetInputSize(dictionary(d), ingoing_connections);
        }

    // Set output sizes // FIXME: Move to separate function

    int outputs_with_size = 0;
    for(auto & d : info_["outputs"])
    {
        std::string n = d["attributes"]["name"];
        std::string s = d["attributes"]["size"];
        std::vector<int> shape = EvaluateSize(s);
        matrix o;
        Bind(o, n); // FIXME: Get directly?
        if(o.rank() != 0)
            outputs_with_size++; // Count number of inputs that are set
        
        o.realloc(shape);
        outputs_with_size++;
    }

    if(outputs_with_size == info_["class"]["outputs"].size())
        return 0; // all outputs have been set
    else
        return 0; // needs to be called again - FIXME: Report progress later on
}



double
Kernel::GetRealTime()
{
    return timer.GetTime();
}


void
Kernel::Tick()
{
    for(auto & m : components)
        m.second->Tick();

    RotateBuffers();
    Propagate();

    tick++;
}



void
Kernel::Run()
{
    // chdir(ikc_dir);

    timer.Restart();
    tick = 0;
    is_running = start;

    while (!Terminate())
    {
        if (!is_running)
        {
            std::cout << "Idle" << std::endl;
            Sleep(0.01); // Wait 10ms to avoid wasting cycles if there are no requests
        }
    /*
        while(sending_ui_data)
            {}
        while(handling_request)
            {}

*/
        if (is_running)
        {
            tick_is_running = true; // Flag that state changes are not allowed
            Tick();
            tick_is_running = false;
            
            // Calculate idle_time
            
            if(tick_duration > 0)
            {
                idle_time = (float(tick*tick_duration) - timer.GetTime()) / float(tick_duration);
                time_usage = 1 - idle_time;
            }

            if (tick_duration > 0)
            {
                lag = timer.WaitUntil(float(tick*tick_duration));
                //if (lag > 0.1) Notify(msg_warning, "Lagging %.2f ms at tick = %ld\n", lag, tick);
            }
/*            
            else if (ui_state == ui_state_realtime)
            {
                while(sending_ui_data)
                    {}
                while(handling_request)
                    {}
            }
*/            
        }

        // std::cout << "Tick()" << std::endl;
    }
}

