// Ikaros 3.0

#ifndef IKAROS
#define IKAROS

#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>

#define INSTALL_CLASS(class_name)  static InitClass init_##class_name(#class_name, []() { return new class_name(); });

#include "Kernel/exceptions.h"
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
#include "Kernel/image_file_formats.h"

namespace ikaros {

const int run_mode_stop = 0;
const int run_mode_pause = 1;
const int run_mode_step = 2;    // Not used
const int run_mode_play = 3;
const int run_mode_realtime = 4;

using tick_count = long long int;

std::string  validate_identifier(std::string s);

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

    CircularBuffer() {}
    CircularBuffer(matrix &  m,  int size);
    void rotate(matrix &  m);
    matrix & get(int i);
};

//
// PARAMETERS // TODO: add bracket notation to set any type p(x,y) etc
//

enum parameter_type { no_type=0, int_type, bool_type, float_type, string_type, matrix_type, options_type, rate_type };
static std::vector<std::string> parameter_strings = {"none", "int", "bool", "float", "string", "matrix", "options", "rate"}; // TODO: Add list of strings?

class parameter // FIXME: check all types everywhere; send base rate to WebUI not current value
{
private:
public:
    dictionary                      info_;
    parameter_type                  type;
    float                           timebase;
    std::string                     default_value;
    std::shared_ptr<int>            int_value;
    std::shared_ptr<float>          float_value;
    std::shared_ptr<matrix>         matrix_value;
    std::shared_ptr<std::string>    string_value;
    std::vector<std::string>        options;

    parameter(): type(no_type) {}
    parameter(std::string type_string, std::string default_val, std::string options_string);
    void operator=(parameter & p); // this shares data with p
    int operator=(int v);
    float operator=(float v);
    float operator=(double v);
    std::string operator=(std::string v); // FIXME: Handle exceptions higher up *******  // FIXME: ADD ALL TYPE HERE!!! *** STEP 1 ***

    operator matrix & ();
    operator std::string();
    operator bool();
    //explicit operator int(); // Do not convert to int explicitly
    operator float();

    int as_int();
    const char* c_str() const noexcept;

    std::string json();    friend std::ostream& operator<<(std::ostream& os, parameter p);
};

// Operators on parameters

float operator+(parameter p, float x);
float operator+(float x, parameter p);
float operator+(parameter p, double x);
float operator+(double x, parameter p);
float operator+(parameter x, parameter p);

float operator-(parameter p, float x);
float operator-(float x, parameter p);
float operator-(parameter p, double x);
float operator-(double x, parameter p);
float operator-(parameter x, parameter p);

float operator*(parameter p, float x);
float operator*(float x, parameter p);
float operator*(parameter p, double x);
float operator*(double x, parameter p);
float operator*(parameter x, parameter p);

float operator/(parameter p, float x);
float operator/(float x, parameter p);
float operator/(parameter p, double x);
float operator/(double x, parameter p);
float operator/(parameter x, parameter p);

//
// COMPONENT
//

class Component
{
public:
    Component *     parent_;
    dictionary      info_;
    std::string     path_;

    Component();

    virtual ~Component() {};

    void print();
    void AddInput(dictionary parameters);
    void AddOutput(dictionary parameters);
    void AddParameter(dictionary parameters);
    void SetParameter(std::string name, std::string value);
    bool BindParameter(parameter & p,  std::string & name);
    bool ResolveParameter(parameter & p,  std::string & name,   std::string & default_value);
    void Bind(parameter & p, std::string n);   // Bind to parameter in global parameter table
    void Bind(matrix & m, std::string n); // Bind to input or output in global parameter table, or matrix parameter

    virtual void Tick() {}
    virtual void Init() {}

    std::string json();
    std::string xml();

    std::string GetValue(const std::string & name);    // Get value of a attribute/variable in the context of this component
    std::string GetBind(const std::string & name);
    std::string SubstituteVariables(const std::string & s);
    Component * GetComponent(const std::string & s); // Get component; sensitive to variables and indirection
    std::string Evaluate(const std::string & s, bool is_string=false);     // Evaluate an expression in the current context
    bool LookupParameter(parameter & p, const std::string & name);
    //std::string Lookup(const std::string & name) const;
    int EvaluateIntExpression(std::string & s);
    std::vector<int> EvaluateSizeList(std::string & s);
    std::vector<int> EvaluateSize(std::string & s);
    bool InputsReady(dictionary d, std::map<std::string,std::vector<Connection *>> & ingoing_connections);

    void SetSourceRanges(const std::string & name, std::vector<Connection *> & ingoing_connections);
    void SetInputSize_Flat(const std::string & name,  std::vector<Connection *> & ingoing_connections, bool use_alias);
    void SetInputSize_Index(const std::string & name, std::vector<Connection *> & ingoing_connections, bool use_alias);
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

//
// CONNECTION
//

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

    Connection(std::string s, std::string t, range & delay_range, std::string alias="");
    void Print();
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
    Class(std::string n, std::string p) : module_creator(nullptr), name(n), path(p) { };

    void print();
};

//
// KERNEL
//

class Kernel
{
public:
    dictionary                              info_;
    std::string                             webui_dir;
    std::map<std::string, Class>            classes;
    std::map<std::string, std::string>      files;                  // ikg-files
    std::map<std::string, Component *>      components;
    std::vector<Connection>                 connections;
    std::map<std::string, matrix>           buffers;                // IO-structure    
    std::map<std::string, int>              max_delays;             // Maximum delay needed for each output
    std::map<std::string, CircularBuffer>   circular_buffers;       // Circular circular_buffers for delayed buffers
    std::map<std::string, parameter>        parameters;

    bool                                    first_request;
    long                                    master_id;
    bool                                    is_running;
    std::atomic<bool>                       tick_is_running;
    std::atomic<bool>                       sending_ui_data;
    std::atomic<bool>                       handling_request;
    int                                     run_mode;
    long                                    port;    

    dictionary                              current_component_info; // Implivit parameters to create Component
    std::string                             current_component_path;

    float                                   idle_time;              // TODO: change to duration<float>             
    float                                   time_usage;             // TODO: change to duration<float>

    int                                     cpu_cores;
    double                                  cpu_usage;
    double                                  last_cpu;
    float                                   last_cpu_time;

    Timer                                   timer;          // Main timer
    bool                                    start;          // Start automatically                   

    // Timing parameters and functions
    double                                  tick_duration;  // Desired actual or simulated duration for each tick
    tick_count                              tick;
    tick_count                              stop_after;
    double                                  lag;            // Lag of a tick in real-time mode
    double                                  lag_min;        // Largest negative lag
    double                                  lag_max;        // Largest positive lag
    double                                  lag_sum;        // Sum |lag|

    ServerSocket *                          socket;
    long                                    session_id;
    std::vector<std::string>                log;
    std::thread *                           httpThread;

    Kernel();

    void Clear();        // Remove all non-persistent data and reset kernel variables - // FIXME: Init???

    static void *   StartHTTPThread(Kernel * k);
    tick_count GetTick() { return tick; }
    double GetTickDuration() { return tick_duration; } // Time for each tick in seconds (s)
    double GetTime() { return (run_mode == run_mode_realtime) ? GetRealTime() : static_cast<double>(tick)*tick_duration; }   // Time since start (in real time or simulated (tick) time dending on mode)
    double GetRealTime() { return (run_mode == run_mode_realtime) ? timer.GetTime() : static_cast<double>(tick)*tick_duration; } 
    double GetLag() { return (run_mode == run_mode_realtime) ? static_cast<double>(tick)*tick_duration - timer.GetTime() : 0; }

    bool Terminate();
    void ScanClasses(std::string path);
    void ScanFiles(std::string path);

    void ListClasses();
    void ResolveParameter(parameter & p,  std::string & name,  std::string & default_value);

    void ResolveParameters(); // Find and evaluate value or default
    void CalculateSizes();
    void CalculateDelays();
    void InitCircularBuffers();
    void RotateBuffers();
    void ListComponents();
    void ListConnections();
    void ListInputs();
    void ListOutputs();
    void ListBuffers();
    void ListCircularBuffers();
    void ListParameters();
    void PrintLog();

    // Functions for creating the network

    void AddInput(std::string name, dictionary parameters=dictionary());
    void AddOutput(std::string name, dictionary parameters=dictionary());
    void AddParameter(std::string name, dictionary params=dictionary());
    void SetParameter(std::string name, std::string value);
    void AddGroup(dictionary info, std::string path);
    void AddModule(dictionary info, std::string path);
    void AddConnection(dictionary info, std::string path); // std::string souce, std::string target, std::string delay_range, std::string alias
    //void ParseGroupFromXML(XMLElement * xml, std::string path="");
    void BuildGroup(dictionary d, std::string path="");

    void AllocateInputs();
    void InitComponents();
    void SetUp();

    void LoadFiles(std::vector<std::string> files, options & opts);
    void Save();

    void SortNetwork();
    void Propagate();

    std::string json();
    std::string xml();

    void InitSocket();

    void Pause();

    void DoNew(std::string uri, std::string args);
    void DoOpen(std::string uri, std::string args);
    void DoSave(std::string uri, std::string args);
    void DoSaveAs(std::string uri, std::string args);

    void DoStop(std::string uri, std::string args);
    void DoPause(std::string uri, std::string args);
    void DoStep(std::string uri, std::string args);
    void DoPlay(std::string uri, std::string args);
    void DoRealtime(std::string uri, std::string args);
    
    void DoCommand(std::string uri, std::string args);
    void DoControl(std::string uri, std::string args);
    
    dictionary GetView(std::string component, std::string view_name);
    void AddWidget(std::string uri, std::string args);
    void DeleteWidget(std::string uri, std::string args);
    void SetWidgetParameters(std::string uri, std::string args);
    void RenameView(std::string uri, std::string args);

    void DoSendNetwork(std::string uri, std::string args);

    void DoSendDataHeader();
    void DoSendDataStatus();
    void DoSendData(std::string uri, std::string args);
    void DoUpdate(std::string uri, std::string args);
    void DoNetwork(std::string uri, std::string args);
    void DoSendLog(std::string uri, std::string args);
    void DoSendClasses(std::string uri, std::string args);
    void DoSendFileList(std::string uri, std::string args);
    void DoSendFile(std::string file);
    void DoSendError();
    void SendImage(matrix & image, std::string & format);

    void HandleHTTPRequest();
    void HandleHTTPThread();
    void Tick();
    void Run(std::vector<std::string> files, options & opts);
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

