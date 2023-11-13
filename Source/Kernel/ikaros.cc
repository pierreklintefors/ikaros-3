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
            if(k.buffers.count(name))
                m = k.buffers[name];
            else if(k.buffers.count(name))
                m = k.buffers[name];
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
Component::InputsReady(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections) // FIXME: Handle optional buffers
{
    Kernel& k = kernel();

    std::string n = d["attributes"]["name"];
    for(auto & c : ingoing_connections[name_+'.'+n])
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
    kernel().buffers[name].realloc(flattened_input_size);

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

    //if(max_delay > 1)
    //    r.push_front(0, max_delay);

    kernel().buffers[name].realloc(r.extent());  // STEP 2: Set input size
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
        if(k.buffers[name_+"."+std::string(d["attributes"]["name"])].empty())
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
            outputs_with_size++; // Count number of buffers that are set
        
        o.realloc(shape);
        outputs_with_size++;
    }

    if(outputs_with_size == info_["class"]["outputs"].size())
        return 0; // all buffers have been set
    else
        return 0; // needs to be called again - FIXME: Report progress later on
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
    // Set-up - // TODO: Move to separate function?

    ResolveParameters();
    CalculateDelays();
    CalculateSizes();
    InitCircularBuffers();

    InitComponents();
    ListComponents();
    ListConnections();

    //ListInputs();
    //ListOutputs();

    ListBuffers();
    ListCircularBuffers();

    ListParameters();
    PrintLog();

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
            std::cout << "Idle: " << run_mode << std::endl;
            Sleep(0.01); // Wait 10ms to avoid wasting cycles if there are no requests
        }
  
        while(sending_ui_data)
            {}
        while(handling_request)
            {}

        if (is_running)
        {
            tick_is_running = true; // Flag that state changes are not allowed
            Tick();
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
    Component::JSONString()
    {
        // return info_.json(); // FIXME: Will be used when shared dictionaries are implemented

        // Collect group strings

        std::string gsep;
        std::string gs;
        if(info_["groups"].size() > 0)
            for(auto & g :  info_["groups"])
            {
                std::string cn =  std::string(info_["name"])+"."+std::string(g["attributes"]["name"]);
                Kernel &k = kernel();
                gs += gsep + k.components.at(cn)->JSONString();
                gsep =", ";
            }

        // Collect module strings

        std::string msep;
        std::string ms;
        if(info_["modules"].size() > 0)
            for(auto & m :  info_["modules"])
            {
                std::string mn =  std::string(info_["name"])+"."+std::string(m["attributes"]["name"]);
                Kernel &k = kernel();
                ms += msep + k.components.at(mn)->JSONString();
                msep =", ";
            }


        std::string s;
        s += "{";

        s += "\"name\":\""+name_+"\"";

        if(std::string(info_["is_group"])=="true")
                s += ", \"is_group\": true";
        else
                s += ", \"is_group\": false";

        s+= " ,\"attributes\": " + info_["attributes"].json();
        s+= " , \"parameters\": " + info_["parameters"].json();
        s+= " , \"buffers\": " + info_["buffers"].json();
        //s+= " , \"buffers\": " + info_["buffers"].json();
        s+= " , \"connections\": " + info_["connections"].json();

        s+= " , \"groups\": ["+gs+"]";
        s+= " , \"modules\": ["+ms+"]";

           s+= " , \"views\": " + info_["views"].json();

        s+= "}";

        return s;
    }


    std::string 
    Kernel::JSONString()
    {
        if(components.empty())
            return "{}";
        else
            return components.begin()->second->JSONString();
    }


//
// WebUI
//

void
Kernel::Pause()
{
    is_running = false;
    while(tick_is_running)
        {}
}



long
get_client_id(std::string & args)
{
    std::string id_string =  head(args, "&");
    std::string id_number = cut(id_string, "id=");

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
Kernel::DoSendData(std::string uri, std::string args)
{    
    sending_ui_data = true; // must be set while main thread is still running
    while(tick_is_running)
        {}

std::string root = cut(args, "data=");
std::string data = cut(root, "#");

    Dictionary header;
    header.Set("Session-Id", std::to_string(session_id).c_str()); // FIXME: GetValue("session_id")
    header.Set("Package-Type", "data");
    header.Set("Content-Type", "application/json");
    header.Set("Cache-Control", "no-cache");
    header.Set("Cache-Control", "no-store");
    header.Set("Pragma", "no-cache");
    header.Set("Expires", "0");
    
    socket->SendHTTPHeader(&header);
    

    socket->Send("{\n");
    socket->Send("\t\"state\": %d,\n", run_mode);
    
    if(stop_after > 0)
    {
        socket->Send("\t\"iteration\": \"%d / %d\",\n", GetTick(), stop_after);
        socket->Send("\t\"progress\": %f,\n", float(tick)/float(stop_after));
    }
    else
    {
        socket->Send("\t\"iteration\": %lld,\n", GetTick());
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
    socket->Send("\t\"cpu_usage\": %.3f", cpu_usage);

    socket->Send(",\n\t\"data\":\n\t{\n");
    std::string sep = "";

    while(!data.empty())
    {
        std::string source = cut_head(data, "#");
        std::string  format = rcut(source, ":");

        //auto root_group = main_group->GetGroup(root);
        
        std::string src = source;
        //if(!root.empty())    // FIXME: Empty or Not empty?
        //    src = root+"."+src;
        
        if(!source.empty())
        {
            // Use data from module function if available
           /*
            auto module_source = rsplit(source, ".", 1);
            if(GroupElement * g = root_group->GetGroup(module_source.at(0)))
            {
                std::string json_data;
                if(g->module)
                    json_data = g->module->GetJSONData(module_source.at(1)); //  : ""
            */

    
          
            if(format == "") // as default, send a matrix
            {
                if(buffers.count(root+"."+source))
                {
                    std::string json_data = buffers[root+"."+source].json();
                    if(!json_data.empty())
                    {
                        socket->Send(sep);
                        std::string s = "\t\t\"" + source + "\": "+json_data;
                        socket->Send(s);
                        sep = ",\n";
                    }
                }
                else
                {
                    // FIXME: output not found error
                }
            }
                  /*
                Module_IO * io = root_group->GetSource(source); // FIXME: Also look for buffers here
                if(io)
                {
                    socket->Send(sep);
                    SendJSONMatrixData(socket, source, *io->matrix[0], io->sizex, io->sizey);
                    sep = ",\n";
                }
            */
            /*
                else if(bindings.count(src))
                {
                    socket->Send(sep);
                    auto bs = bindings.at(src);
                    Binding * b = bs.at(0);   // Use first binding
                    switch(b->type)
                    {
                        case data_source_int:
                        case data_source_list:
                            socket->Send("\t\t\"%s\": [[%d]]", source.c_str(), *(int *)(b->value));
                            break;

                        case data_source_bool:
                            socket->Send("\t\t\"%s\": [[%d]]", source.c_str(), int(*(bool *)(b->value)));
                            break;

                        case data_source_float:
                            socket->Send("\t\t\"%s\": [[%f]]", source.c_str(), *(float *)(b->value));
                            break;

                        case data_source_string:
                            socket->Send("\t\t\"%s\": \"%s\"", source.c_str(), ((std::string *)(b->value))->c_str());
                            break;
         
                        case data_source_matrix:
                            SendJSONMatrixData(socket, source, *(float **)(b->value), b->size_x, b->size_y);
                            break;
                            
                        case data_source_array:
                            SendJSONArrayData(socket, source, (float *)(b->value), b->size_x);
                            break;
                            
                        default:
                            socket->Send("\"ERRROR_type_is\": %d", b->type);
                    }
                    sep = ",\n";
                }
            }
            else if(format == "gray" && source[0])
            {
                if(Module_IO * io = root_group->GetSource(source))
                {
                    socket->Send(sep);
                    socket->Send("\t\t\"%s:gray\": ", source.c_str()); // FIXME: Check return
                    SendColorJPEGbase64(socket, *io->matrix[0], *io->matrix[0], *io->matrix[0], io->sizex, io->sizey);
                    sep = ",\n";
                }
            }
            else if(format == "rgb" && !source.empty())
            {
                auto a = rsplit(source, ".", 1); // separate out buffers
                auto o = split(a[1], "+"); // split channel names
                
                if(o.size() == 3)
                {
                    auto c1 = a[0]+"."+o[0];
                    auto c2 = a[0]+"."+o[1];
                    auto c3 = a[0]+"."+o[2];

                    Module_IO * io1 = root_group->GetSource(c1);
                    Module_IO * io2 = root_group->GetSource(c2);
                    Module_IO * io3 = root_group->GetSource(c3);
                    
                    // TODO: check that all buffers have the same size

                    if(io2 && io2 && io3)
                    {
                        socket->Send(sep);
                        socket->Send("\t\t\"%s:rgb\": ", source.c_str()); // FIXME: Check return
                        SendColorJPEGbase64(socket, *io1->matrix[0], *io2->matrix[0], *io3->matrix[0], io1->sizex, io1->sizey);
                        sep = ",\n";
                    }
                }
            }
            else if(source[0] && (format == "fire" || format == "spectrum" || format == "red" || format == "green" || format == "blue"))
            {
                if(Module_IO * io = root_group->GetSource(source))
                {
                    socket->Send(sep);
                    socket->Send("\t\t\"%s:%s\": ", source.c_str(), format.c_str()); // FIXME: Check return
                    SendPseudoColorJPEGbase64(socket, *io->matrix[0], io->sizex, io->sizey, format.c_str());
                    sep = ",\n";
                }
            }

            */
        }

    }


    socket->Send("\n\t}");

    if(tick_is_running) // new tick has started during sending
    {
        socket->Send(",\n\t\"has_data\": 0\n"); // there may be data but it cannot be trusted
    }
    else
    {
        socket->Send(",\n\t\"has_data\": 1\n");
    }
    socket->Send("}\n");

    sending_ui_data = false;
}



void
Kernel::DoStop(std::string uri, std::string args)
{
    Pause();
    run_mode = run_mode_stop;

    // Notify(msg_terminate, "Sent by WebUI.\n");
    DoSendData(uri, args);
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
        std::string s = JSONString(); 
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
    std::cout << "DoPause" << std::endl;
    Pause();
    run_mode = run_mode_pause;
    master_id = get_client_id(args);
    DoSendData(uri, args);
}



void
Kernel::DoStep(std::string uri, std::string args)
{
    std::cout << "DoPStep" << std::endl;
    Pause();
    run_mode = run_mode_pause;
    master_id = get_client_id(args);
    Tick();
    DoSendData(uri, args);
}




void
Kernel::DoPlay(std::string uri, std::string args)
{
    std::cout << "DoPlay" << std::endl;
    Pause();
    run_mode = run_mode_play;
    master_id = get_client_id(args);
    Tick();
    DoSendData(uri, args);
}


void
Kernel::DoRealtime(std::string uri, std::string args)
{
    std::cout << "DoRealtime" << std::endl;
    run_mode = run_mode_realtime;
    master_id = get_client_id(args);
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
    //printf(">>> %s %s\n", uri.c_str(), args.c_str());
    char module_name[255];
    char parameter[255];
    int x, y;
    float value;
    int c = sscanf(uri.c_str(), "/control/%[^/]/%d/%d/%f", parameter, &x, &y, &value);
    if(c == 4)
        // SetParameter(parameter, x, y, value); // TODO: check if groups are handled correctly *********************
    DoSendData(uri, args);
}


void
Kernel::DoUpdate(std::string uri, std::string args)
{
    if(args.empty() || first_request) // not a data request - send view data
    {
        first_request = false;
        DoSendNetwork(uri, args);
    }
    else if(run_mode == run_mode_play && master_id == get_client_id(args))
    {
        Pause();
        Tick();
        DoSendData(uri, args);
    }
    else 
        DoSendData(uri, args);
}


void
Kernel::DoGetLog(std::string uri, std::string args)
{
    /*
    if (logfile)
        socket->SendFile("logfile", webui_dir);
    else
        socket->Send("ERROR - No logfile found\n");
    */
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
        // Notify(msg_warning, "No URI");
        return;
    }

    std::string args = cut(uri, "?");


    // SELECT METHOD

        if(uri == "/update")
        DoUpdate(uri, args);

        else if(uri == "/pause")
            DoPause(uri, args);

        else if(uri == "/step")
            DoStep(uri, args);


        else if(uri == "/play")
            DoPlay(uri, args);

        else if(uri == "/realtime")
            DoRealtime(uri, args);

        else if(uri == "/stop")
            DoStop(uri, args);

        else if(uri == "/getlog")
            DoGetLog(uri, args);
        
        else if(uri == "/classes") 
            DoSendClasses(uri, args);

        else if(uri == "/")
            DoSendFile("index.html");

        else if(starts_with(uri, "/command/"))
            DoCommand(uri, args);
            
        else if(starts_with(uri, "/control/"))
            DoControl(uri, args);

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


