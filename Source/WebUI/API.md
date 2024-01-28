Ikaros Kernel API
==================

The API is mainly used internally by BrainStudio to communicate with the kernal,
but can also be used by other programs to communicate with the kernel. Unless
you are writing a new user interface for Ikaros, this information is probably
not useful.

API requests have the following syntax:

    host:port/command[/path[/view]][?(attribute=value)*]

_Path_ is the path to the current component - a group or a module.

_View_ is the name of the current view if selected.

    The _attribute-value_ pairs are converted to a dictionary that is used as an argument to the handlers in the kernel.

The possible commands are listed below. The path is the dotted path to a
component (a group or a module).  The view part is optional and used to set values in views. There
can be zero or  multiple attribute value-pairs depedning on the command.

The command should be URL encoded if they include non-ascii characters.

Each command has its own set of possible parameters

URL parameters used internally by Ikaros:

- none currently

The response from the kernel is always a JSON package that can contain different types of data. Currently there are only _data_ and _network_ messages. Data are used to update the state in the WebUI and network is used to represent the current state of the network loaded into the kernel.

The type of response is indicated in the HTTP header Package-Type as "data" or "network".

## Basic API Requests

### /

Get the main BrainStudio html code defined in index.html. This will in turn load all the required additional files into the web browser and start the BrainStudio interface. A BrainStudio spash screen is shown while additional script files (mainly widgets) are loaded.

Example:

    http://localhost:8000/

### /classes

Get list of all classes defined in Ikaros as JSON.

Example request:

    http://localhost:8000/classes

Example reponse:

```json
{"classes":
    [
        "Add",
        "Constant",
        "Logger",
        "Nucleus",
        "Oscillator",
        "OutputFile",
        "Print",
        "Protocol",
        "Randomizer"
    ]
}
```

### /files

Get list of all available ikg files. These files are used bu the Open command.

Example request:

    http://localhost:8000/files

Example response:

```json
{"files":
    [
        "Add_test",
        "Constant_test",
        "Oscillator_test",
        "OutputFile_test"
    ]
}
```

### /network

Get the active network description as a JSON structure.

Example request:

    http://localhost:8000/network

Example reponse:

```json
{
    "name":"topgroup",
    "groups": [...],
    "modules": [...],
    "conections": [...],
    :
    :
}
```

### /update

Basic function to get the state of Ikaros. The data parameter contains a list of variables in the current context that should be sent to the WebUI in the returned JSON package. A variable can contain a format specification if it contain an image. A format specification is indicated bya colon followed by the format name.


Synopsis:

    http://localhost:8000/update/path?data=var,var,var,var

var == path[:format]

Example request:

    http://localhost:8000/update/path?data=X,Y,Z

Example reponse:

```json
{
    :
    :
    "data":"
        {
            "X": "12",
            "Y": "42",
            "Z": "0"
        },
    :
    :
}
```

The update response also contain status information.

## Control Requests

### /pause

Pause execution of the kernel

### /step

Run a single tick and set pause mode.

### /play

Run a single tick and set run mode.

### /realtime

Start realtime mode.

### /stop

Stop execution of a network. The kernel is still responsive and can load and start a new network. 

## File Handling Requests

### /new

Create a new empty network.

### /open

Open an ikg file

### /save

Overwrite the current ikg file with changes made.

### /saveas?name=<new_file_name>

Save current network with a new name. The parameter name indicates the path and name of the new file.

Example:

    /saveas?name=new_name

### /command (not implemented yet)

Execute a command in a module. A dictionary with hall parameters are sent to the module.

Example:

    /command?cmd=mycommand&x=42



### /control (not implemented yet)

Change a parameter value in a module.

Parameters:

- path. 

- value. The new value.

Old syntax:

/control/path/x/y/value

Example:

    /control/a.b.c.p?p=96

Example with index into matrix parameter:

    /control/a.b.c.p?X[0][2]=96

## View Editing Requests

### /addview

    /addview/path?name=viewname

Create a new view.

### /renameview

Change the name of a view.

    /renameview/path/view?name=newname

### /addwidget

Add a widget to a view.

    http://localhost:8000/addwidget/path/view?x=1&y=3&....

## /delwidget

Delete a widget in a view.

Example:    

    http://localhost:8000/delwidget?index=5

### setwidgetparams

Set all paramaters of a widget.

Synopsis:

    addwidgetsetwidgetparams/path.to.component/widget?attribute=value&...

### widgettofront

Send a widget to front.

Example:    

    http://localhost:8000/widgettofront?index=5

## /widgettoback

Sena widget to back.

Example:    

    http://localhost:8000/widgettoback?index=5


## Network Editing Requests

The remote commands changes the network but changes do not take place until after a restart, that is, a save, a stop and a reload.

### /addgroup

Parameters:

- path: path to the outer group
- name: name of the new group

Example:

    http://localhost:8000/addgroup/path?name=newname


### /addmodule

Example:

    http://localhost:8000/addmodule/path?name=newname&class=Constant

### /setattributes

Parmeters:

- path: path to the component with the attributes to set
- &lt;attribute>
- &lt;value>

Synopsis:

    http://localhost:8000/setattributes?<path>?<attribute>=<value>&...

Example:

    http://localhost:8000/setattributes?root=path&x=42&y=32

If attribute is "name" the hashes in the kernel are also rebuilt.

### /addconnection

Example:

    http://localhost:8000/addconnection/path?source=M.X&target=N.Y

### /changeconnection

Example:

    http://localhost:8000/changeconnection/path?source=M.X&target=N.Y

### /deleteconnection

Example:

    http://localhost:8000/deleteconnection/path?source=M.X&target=N.Y