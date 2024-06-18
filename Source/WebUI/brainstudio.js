/*
 * New version of WebUI interface with BrainStudio functions
 */

const widget_classes = 
[
    "bar-graph",
    "plot",
    "table",
    "marker",
    "path",
    "grid",
    "image",
    "text",
    "rectangle",
    "button",
    "slider-horizontal",
    "drop-down-menu",
    "control-grid",
    "canvas3d",
    "epi-head"
]

function isEmpty(obj) 
{
    if(obj)
        for (const prop in obj) 
        {
            if (Object.hasOwn(obj, prop)) 
                return false;
        }
    return true;
}


function deepCopy(source) {
    if (source === null || typeof source !== 'object') {
      return source;
    }
  
    if (Array.isArray(source)) {
      return source.map(item => deepCopy(item));
    }
  
    const newObject = {};
    for (const key in source) {
      newObject[key] = deepCopy(source[key]);
    }
    return newObject;
  }
  
  function replaceProperties(target, source) {
    // Remove all properties from the target object
    for (const key in target) {
      if (Object.hasOwnProperty.call(target, key)) {
        delete target[key];
      }
    }
  
    // Copy properties from the source object to the target object
    for (const key in source) {
      if (Object.hasOwnProperty.call(source, key)) {
        target[key] = deepCopy(source[key]);
      }
    }
  }
  

  function toggleStrings(array, toggleItems)
    {
    toggleItems.forEach(item => {
        const index = array.indexOf(item);
        if (index === -1)
            array.push(item);
        else 
            array.splice(index, 1);
    });
  }


  function removeStringFromStart(mainString, stringToRemove)
  {
    if (mainString.startsWith(stringToRemove))
      return mainString.slice(stringToRemove.length);
    return mainString;
  }

  String.prototype.rsplit = function(sep, maxsplit) {
    var split = this.split(sep || /\s+/);
    return maxsplit ? [ split.slice(0, -maxsplit).join(sep) ].concat(split.slice(-maxsplit)) : split;
}

function toURLParams(obj)
{
    s = "";
    sep = "";
    const keys = Object.keys(obj)
    keys.forEach((key) => {
        s+= sep+`${key}=${obj[key]}`;
        sep="&";
    });

    return encodeURIComponent(s);
}

function setType(x, t)
{

    if(t == 'int')
        return parseInt(x);
    
    if(t == 'float')
        return parseFloat(x);
    
    if(t == 'bool')
        return ['on','yes','true'].includes(x.toString().toLowerCase());
    
    return x;
};

function zeroPad(x)
{
    if(x <10)
        return "0"+x;
    else
        return String(x);
}

function secondsToHMS(d)
{
    try
    {
        if(d=="-")
            return "-";

        d = Number(d);
        if(isNaN(d) || d < 0)
            return "-";

        var h = Math.floor(d / 3600);
        var m = Math.floor(d % 3600 / 60);
        var s = Math.floor(d % 3600 % 60);
        return h+":"+zeroPad(m)+":"+zeroPad(s);
    }
    catch(err)
    {
        return "-";
    }
}

function formatTime(d)
{
    try
    {
        if(d == "-")
            return "-";
        if(d < 0)
            return "-";
        else if(d < 0.001)
            return 1000000*d + " &#181;s";
        else if(d < 5)
            return 1000*d + " ms";
        else
            return secondsToHMS(d);
    }
    catch(e)
    {
        return "-";
    }
}

function changeNameInPath(path, name)
{
    const ix = path.lastIndexOf('.');
    if (ix === -1)
        return name;
    return path.substring(0, ix + 1) + name;
}



function nameInPath(path)
{
    const ix = path.lastIndexOf('.');
    if (ix === -1)
        return "";
    return path.substring(ix + 1);
}


function parentPath(path)
{
    let p = path.split('.');
    p.pop();
    return p.join('.');
    
}

// COOKIES FOR PERSISTENT STATE

function setCookie(name,value,days=100)
{
    var date = new Date();
    date.setTime(date.getTime()+(days?days:1)*86400000);
    var expires = "; expires="+date.toGMTString();
    document.cookie = name+"="+value+expires+"; path=/";
}

function getCookie(name)
{
    var nameEQ = name + "=";
    var ca = document.cookie.split(';');
    for(var i=0;i < ca.length;i++) {
        var c = ca[i];
        while (c.charAt(0)==' ') c = c.substring(1,c.length);
        if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
    }
    return null;
}

function eraseCookie(name)
{
    createCookie(name,"",-1);
}

function resetCookies()
{
    setCookie('current_view', "");
//    setCookie('root', ""); // or /
//    setCookie('inspector',"closed");
}



/*
 *
 * Dialog Scrips
 *
 *
 */
/*
let dialog = {
    confirmOpen: function()
    {
        try
        {
            let sys = document.getElementById("openDialogSystemItems");
            if(sys.style.display=='block')
            {
                let sel = document.getElementById("openDialogSystemItems");
                let text= sel.options[sel.selectedIndex].text;
                dialog.window.close(text);
                    if(dialog.callback)
                        dialog.callback(text, 'system');
            }
            else
            {
                let sel = document.getElementById("openDialogUserItems");
                let text= sel.options[sel.selectedIndex].text;
                dialog.window.close(text);
                    if(dialog.callback)
                        dialog.callback(text, 'user');
            }
        }
        catch(err)
        {
            alert("error opening file");
        }
    },
        
    cancelOpen: function()
    {
        dialog.window.close(null);
    },

    showOpenDialog: function (file_list, callback, message)
    {
        dialog.callback = callback;
        dialog.window = document.getElementById('openDialog');
        let sel = document.getElementById('openDialogSystemItems');
        sel.innerHTML = '';
        let usel = document.getElementById('openDialogUserItems');
        usel.innerHTML = '';

        if(file_list.system_files)
            for(i of file_list.system_files)
            {
                var opt = document.createElement('option');
                opt.value = i;
                opt.innerHTML = i;
                document.getElementById('openDialogSystemItems').appendChild(opt);
            }

            if(file_list.user_files)
                for(i of file_list.user_files)
                {
                    var opt = document.createElement('option');
                    opt.value = i;
                    opt.innerHTML = i;
                    document.getElementById('openDialogUserItems').appendChild(opt);
                }

            if(message)
            {
                document.getElementById('openDialogTitle').innerText = message;
            }
        dialog.window.showModal();
    }
}

*/

let dialog = {
    window: null,
    callback: null,

    confirmOpen: function() {
        try {
            let dialogType = this.getDialogType();
            let sel = document.getElementById(`open_dialog_${dialogType}_items`);
            let text = sel.options[sel.selectedIndex].text;
            
            this.window.close(text);
            
            if (this.callback) {
                this.callback(text, dialogType);
            }
        } catch (err) {
            alert("Error opening file");
        }
    },

    cancelOpen: function() {
        this.window.close(null);
    },

    showOpenDialog: function(callback, message) {



        fetch('/files', {method: 'GET', headers: {"Session-Id": controller.session_id, "Client-Id": controller.client_id}})
        .then(response => {
            if (!response.ok) {
                alert("ould not get file list from server.");
                throw new Error("HTTP error " + response.status);
            }
            return response.json();
        })
        .then(json => {
            controller.filelist = json;
            this.setupDialog(callback);
            this.populateFileList(controller.filelist);
            this.displayMessage(message);
            this.window.showModal();
        })
        .catch(function () {
            alert("ould not get file list from server.");
            console.log("Could not get file list from server.");
        })




    },

    getDialogType: function() {
        let sys = document.getElementById("open_dialog_system_items");
        return sys.style.display === 'block' ? 'system' : 'user';
    },

    setupDialog: function(callback) {
        this.callback = callback;
        this.window = document.getElementById('open_dialog');
    },

    populateFileList: function(file_list) {
        this.populateOptions('system', file_list.system_files);
        this.populateOptions('user', file_list.user_files);
    },

    populateOptions: function(type, files) {
        if (files) {
            let sel = document.getElementById(`open_dialog_${type}_items`);
            sel.innerHTML = '';  // Clear previous options
            files.forEach(file => {
                let opt = document.createElement('option');
                opt.value = file;
                opt.text = file;
                sel.appendChild(opt);
            });
        }
    },

    displayMessage: function(message) {
        if (message) {
            document.getElementById('open_dialog_title').innerText = message;
        }
    },

    showSystemFileList: function() {

        system_file_button

        document.getElementById('system_file_button').classList.add("selected");
        document.getElementById('user_file_button').classList.remove("selected");
        document.getElementById('open_dialog_system_items').style.display='block';
        document.getElementById('open_dialog_user_items').style.display='none';

    },

    showUserFileList: function() {
        document.getElementById('system_file_button').classList.remove("selected");
        document.getElementById('user_file_button').classList.add("selected");
        document.getElementById('open_dialog_system_items').style.display='none';
        document.getElementById('open_dialog_user_items').style.display='block';
    }
};


let network = {
    network: null,
    classes: null,
    classinfo: {},
    dict: {},

    init(n)
    {
        network.network = n;
        network.rebuildDict();
    },

    rebuildDict()
    {
        network.dict = {};
        network.buildDict(network.network, "");
    },

    buildDict(o, path)
    {
        network.dict[(path+'.'+o.name).substring(1)] = o;
        for(let g of o.groups || [])
            network.buildDict(g, path+'.'+o.name)

        for(let i of o.inputs || [])
            network.dict[(path+'.'+o.name).substring(1)+'.'+i.name] = i;
        for(let i of o.outputs || [])
            network.dict[(path+'.'+o.name).substring(1)+'.'+i.name] = i;

        for(let m of o.modules || [])
            network.dict[(path+'.'+o.name).substring(1)+'.'+m.name] = m;
        for(let w of o.widgets || [])
            network.dict[(path+'.'+o.name).substring(1)+'.'+w.name] = w;
        for(let c of o.connections || [])
            network.dict[(path+'.'+o.name).substring(1)+'.'+c.source+'*'+(path+'.'+o.name).substring(1)+'.'+c.target] = c;
    },

    newConnection(path, source, target)
    {
        let connection = 
        {
            "_tag": "connection",
            "source": source,
            "target": target,
            "source_range":"",
            "target_range": "",
            delay_range: "1"
        };
        let group = network.dict[path];
        if(group.connections == null) // FIXME: SHOULD NOT BE NECESSARY
        group.connections = [];
        group.connections.push(connection);
        network.dict[path+"."+source+"*"+path+"."+target]=connection;
    },

    changeModuleClass(module, new_class)
    {
        let old_module = deepCopy(network.dict[module]);
        replaceProperties(network.dict[module], network.classinfo[new_class]);
        let new_module = network.dict[module];
        new_module._tag = "module";
        new_module.class = new_class;
        new_module.name = old_module.name;
        new_module._x = old_module._x;
        new_module._y = old_module._y;
        // TODO: Check that all properties are in the class
        network.dict[module] = new_module;
        // Update existing connections if possible
    },


    changeWidgetClass(widget, new_class)
    {
        let old_widget = deepCopy(network.dict[widget]);
        let new_widget  = {
            "_tag": "widget",
            "name": old_widget.name,
            "title": old_widget.title,
            "class": new_class,
            '_x':old_widget._x,
            '_y':old_widget._y,
            'width': old_widget.width,
            'height': old_widget.height
        };
        replaceProperties(network.dict[widget], new_widget);
        //network.dict[widget] = new_widget;
    },


    renameGroupOrModule(group, old_name, new_name)
    {
        network.dict[new_name] = network.dict[old_name]; // inspector.item; // FIME: Should also change item here and not in inspector
        delete network.dict[old_name];

        // Update connections to and from this group or module

        let parent_path = parentPath(old_name);
        let parent_group = network.dict[group];
        let connection_parent = "";
        for(let c of parent_group.connections || [])
        {
            // Update if source (without output) == old_name

            connection_parent = parentPath(c.source);
            if(connection_parent == nameInPath(old_name))
            {
                let p = c.source.split('.');
                c.source = nameInPath(new_name)+'.'+p[1];
            }

            // Update of target == this


            connection_parent = parentPath(c.target);
            if(connection_parent == nameInPath(old_name))
            {
                let p = c.target.split('.');
                c.target = nameInPath(new_name)+'.'+p[1];
            }
        }
    },  

    renameInput(group, old_name, new_name) // FIXME: NEW PATH not NAME
    {
        for(let c of network.dict[group].connections || [])
        {
            if(group+'.'+c.source == old_name)
            {
                c.source = nameInPath(new_name); // FIXME: USE END OF NEW_NAME - changeNameInPath(selector.selected_background, inspector.item.name);
            }
        }
        let parent_path = parentPath(group);
        let parent_group = network.dict[parent_path];

        if(parent_group)
            for(let c of parent_group.connections || [])
            if(parent_path+'.'+c.target == old_name)
            {
                c.target = changeNameInPath(c.target, inspector.item.name);
            }
            network.rebuildDict();
    },

    renameOutput(group, old_name, new_name)
    {
        for(let c of network.dict[group].connections || [])
        {
            if(group+'.'+c.target == old_name)
            {
                c.target = nameInPath(new_name);
            }
        }
        let parent_path = parentPath(group);
        let parent_group = network.dict[parent_path];

        if(parent_group)
            for(let c of parent_group.connections || [])
            if(parent_path+'.'+c.source == old_name)
            {
                c.source = changeNameInPath(c.source, inspector.item.name);
            }
            network.rebuildDict();
            nav.populate();
    },

    debug_json()
    {
        var w = window.open("about:blank", "", "_blank");
        w.document.write('<html><body><pre>'+JSON.stringify(network.network,null,2)+ '</pre></body></html>');
    },

    debug_dict()
    {
        var w = window.open("about:blank", "", "_blank");
        w.document.write('<html><body><h1>All components (in network.dict)</h1><pre>');
        w.document.write('<html><body><pre>');
        for(let k of Object.keys(network.dict))
            w.document.write(k+"\n");
        w.document.write('</pre></body></html>');
    }
}

let webui_widgets = {
    constructors: {},
    add: function(element_name, class_object) {
        customElements.define(element_name, class_object);
        webui_widgets.constructors[element_name] = class_object;
    }
};

let controller = {
    run_mode: 'pause',
    commandQueue: ['update'],
    tick: 0,
    session_id: 0,
    client_id: Date.now(),
    files: {},
    load_count: 0,
    load_count_timeout: null,
    g_data: null,
    send_stamp: 0,
    tick_duration: 0,
    webui_interval: 0,
    webui_req_int: 100,        
    timeout: 500,            
    reconnect_interval: 1200, 
    reconnect_timer: null,
    request_timer: null,

    reconnect()
    {
        controller.get("update", controller.update);
        controller.setSystemInfo(null);
        let s = document.querySelector("#state");
        if(s.innerText == "waiting")
            s.innerHTML = "waiting &bull;";
        else
            s.innerHTML = "waiting";

    },

    defer_reconnect()
    {
        clearInterval(controller.reconnect_timer);
        //controller.reconnect_timer = setInterval(controller.reconnect, controller.reconnect_interval);
        // AUTOMATIC RECONNECT TEMPORARILY TURNED OFF - should only happen in run mode **********
    },

        saveNetwork() // FIXME: Change to allow saving of fragments
        {
            const jsonString = JSON.stringify(network.network,null,2);
    
        fetch("save", {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json',
                'Session-Id': controller.session_id,
                'Client-Id': controller.client_id
            },
            body: jsonString
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok ' + response.statusText);
            }
            return response.json(); // Assuming the server responds with JSON
        })
        .then(data => {
            //alert(data);
            //console.log('Success:', data);
            // Now request network!
        })
        .catch(error => {
            console.error('Error:', error);
            alert("Save failed");
        });
    },
    
    get: function (url, callback)
    {
        controller.send_stamp = Date.now();
        var last_request = url;
        let xhr = new XMLHttpRequest();
        //console.log("<<< controller.get: \""+url+"\"");
        xhr.open("GET", url, true);
        xhr.setRequestHeader("Session-Id", controller.session_id);
        xhr.setRequestHeader("Client-Id", controller.client_id);
        xhr.onload = function(evt)
        {
              callback(xhr.response, xhr.getResponseHeader("Session-Id"), xhr.getResponseHeader("Package-Type"));
        }
        
        xhr.responseType = 'json';
        xhr.timeout = 1000;
        try {
            xhr.send();
        }
        catch(error)
        {
            console.log("console.get: "+error);
        }
    },

    init: function ()
    {
        //controller.getFiles();
        controller.getClasses();
        controller.getClassInfo();
        controller.requestUpdate();
        controller.reconnect_timer = setInterval(controller.reconnect, controller.reconnect_interval);
    },
    
    queueCommand: function (command) {
        controller.commandQueue.push(command);
    },

    new: function () {
        controller.queueCommand('new');
    },

    openCallback: function(filename, where)
    {
        controller.get("open?where="+where+"&file="+filename, controller.update);
    },

    open: function () {

        dialog.showOpenDialog(controller.openCallback, "Select file to open");
    },

    save: function () {
        //controller.get("save", controller.update);
    if(network.network.filename=="")
            controller.saveas();
    else
        controller.saveNetwork();
    },

    saveas: function() 
    {
        const filename = prompt("Save as:");
        if (filename !== null)
        {
            //console.log("User input:", userInput);
            // FIXME: check name validity
            network.network.filename = filename;
            controller.save();
        } 
        else
        {

        }
    },

    quit: function () {
        controller.run_mode = 'quit';
        controller.get("quit", controller.update); // do not request data -  stop immediately
     },

    stop: function () {
        controller.run_mode = 'stop';
        controller.get("stop", controller.update); // do not request data -  stop immediately
     },
    
    pause: function () {
        main.cancelEditMode();
        controller.queueCommand('pause');
    },
    
    step: function ()
    {
        if(network.network.filename=="")
            controller.saveas();
        else
            controller.save();

        document.querySelector("#state").innerText = "step";
        main.cancelEditMode();
        controller.queueCommand('step');
    },
    
    play: function () {
        if(network.network.filename=="")
        {
            controller.saveas();
            return;
        }
        else
            controller.save();
    
            main.cancelEditMode()
        controller.queueCommand('play');
    },
    
    realtime: function () {
        if(network.network.filename=="")
        {
            controller.saveas();
            return;
        }
        else
            controller.save();

            main.cancelEditMode();
        controller.queueCommand('realtime');
    },

    start: function () {
        if(network.network.filename=="")
        {
            controller.saveas();
            return;
        }
        else
            controller.save();


        controller.play();  // FIXME: possibly start selected mode play/fast-forward/realtime
    },

    clear_wait()
    {
        controller.load_count = 0;
        console.log("clear_wait - drawing or data load failed"); // FIXME: Remove
    },

    wait_for_load(data)
    {
        if(controller.load_count > 0)
            setTimeout(controller.wait_for_load, 1);
        else
        {
            clearTimeout(controller.load_count_timeout);
            controller.updateWidgets(controller.g_data);
        }
    },
    
    updateImages(data)
    {
        controller.load_count = 0;
        controller.g_data = data;

        try
        {
            let w = document.getElementsByClassName('frame')
            for(let i=0; i<w.length; i++)
            {
                if(w[i].children[1].loadData)
                {
                    controller.load_count += w[i].children[1].loadData(data);
                }
            }
     
            controller.load_count_timeout = setTimeout(controller.clear_wait, 200); // give up after 1/5 s and continue
            setTimeout(controller.wait_for_load, 1);
        }
        catch(err)
        {
            console.log("updateImage: exception: "+err);
        }
    },

    setSystemInfo(response)
    {
        try
        {
            if(response == null)
            {
                document.querySelector("#file").innerText = "-";
                document.querySelector("#tick").innerText = "-";
                document.querySelector("#state").innerText = "-";
                document.querySelector("#uptime").innerText = "-";
                document.querySelector("#time").innerText = "-";
                document.querySelector("#ticks_per_s").innerText = "-";
                document.querySelector("#tick_duration").innerText = "-";
                document.querySelector("#actual_duration").innerText =  "-";
                document.querySelector("#lag").innerText = "-";
                document.querySelector("#cpu_cores").innerText = "-";
                document.querySelector("#time_usage").innerText = "-";
                document.querySelector("#usage").innerText = "-";
                document.querySelector("#webui_updates_per_s").innerText = "-";
                document.querySelector("#webui_interval").innerText = "-";
                document.querySelector("#webui_req_int").innerText = "-";
                document.querySelector("#webui_ping").innerText = "-";
                document.querySelector("#webui_lag").innerText = "-";
                let p = document.querySelector("#progress");
                p.style.display = "none";
                return;
            }

            if(response.debug)
                document.querySelector("#debug_row").style.display="block";
            else
                document.querySelector("#debug_row").style.display="none";

            document.querySelector("#file").innerText = response.file;
            document.querySelector("#tick").innerText = (Number.isInteger(response.tick) && response.tick >= 0 ?  response.tick : "-");
            document.querySelector("#state").innerText = controller.run_mode;
            document.querySelector("#uptime").innerText = secondsToHMS(response.uptime);
            document.querySelector("#time").innerText = secondsToHMS(response.time);
            document.querySelector("#ticks_per_s").innerText = response.ticks_per_s;
            document.querySelector("#tick_duration").innerHTML = formatTime(response.tick_duration);
            document.querySelector("#actual_duration").innerHTML = formatTime(response.actual_duration);
            document.querySelector("#lag").innerHTML = formatTime(response.lag);
            document.querySelector("#cpu_cores").innerText = response.cpu_cores;
            document.querySelector("#time_usage").value = response.time_usage;
            document.querySelector("#usage").value = response.cpu_usage;
            document.querySelector("#webui_updates_per_s").innerText = (1000/controller.webui_interval).toFixed(1) + (response.has_data ? "": " (no data)");
            document.querySelector("#webui_interval").innerText = controller.webui_interval+" ms";
            document.querySelector("#webui_req_int").innerText = controller.webui_req_int+" ms";
            document.querySelector("#webui_ping").innerText = controller.ping+" ms";
            document.querySelector("#webui_lag").innerText = (Date.now()-response.timestamp)+" ms";
            
            let p = document.querySelector("#progress");
            if(response.progress > 0) // FIXME: not working
            {
                p.value = response.progress;
                p.style.display = "table-row";
            }
            else
                p.style.display = "none";

            controller.tick = response.tick;
            controller.run_mode = ['quit', 'stop','pause','play','realtime','restart'][response.state];
        }
        catch(err)
        {
            console.log("controller.setSystemInfo: incorrect package received form ikaros.")
        }
    },

    update(response, session_id, package_type)
    {
        if(isEmpty(response))
        {
            console.log("ERROR: empty or malformed response");
            return;
        }

        controller.ping = Date.now() - controller.send_stamp;
        controller.defer_reconnect(); // we are still connected

        // NETWORK

        if(package_type == "network")
        {
            document.querySelector("header").style.display="block"; // Show page when network is loaded
            document.querySelector("#load").style.display="none";
            controller.session_id = session_id;
            controller.tick = response.tick;
            network.init(response);
            nav.populate();
            let top = network.network.name;
            selector.selectItems([], top);
            //controller.views = {};
            //controller.buildViewDictionary(response, "");
            
            let v = getCookie('current_view');

            /*
            if(Object.keys(controller.views).includes(v))
                controller.selectView(v);
            else
                controller.selectView(Object.keys(controller.views)[0]);
            */
       }

        // NEW SESSION

        else if(controller.session_id != session_id) // new session
        {
            session_id = session_id;
            controller.get("network", controller.update);
            return;
        }

        // DATA

        else if(package_type == "data")
        {
            controller.setSystemInfo(response);
            controller.tick_duration = response.tick_duration || 0;
            if(response.has_data)
                controller.updateImages(response.data);
        }

        if(response.log)
        {
            let logElement = document.querySelector('.log');
            response.log.forEach((element) => {
                if(element[0] == "M")
                logElement.innerHTML += "<p class='message'>"+element[1]+"</p>\n";
                else if(element[0] == "W")
                    logElement.innerHTML += "<p class='warning'>"+element[1]+"</p>\n";
                else if(element[0] == "E" || element[0] == "F")
                {
                    logElement.innerHTML += "<p class='error'>"+element[1]+"</p>\n";
                    log.showView();
                }
        });
 
            logElement.scrollTop = logElement.scrollHeight;
        }
    },

    requestUpdate()
    {
        clearTimeout(controller.request_timer);
        controller.request_timer = setTimeout(controller.requestUpdate, controller.webui_req_int); // immediately schdeule next

        if(selector.selected_background == null)
            return;

        controller.webui_interval = Date.now() - controller.last_request_time;
        controller.last_request_time = Date.now();

        let group_path ="";
        let data_string = "";
    /*
        if(!interaction.currentView) // no view selected
        {
            controller.get("update", controller.update);
            return;
        }
*/

        // Request new data
        let data_set = new Set();
        
        let w = document.getElementsByClassName('widget'); // FiXME: only in view later
        for(let i=0; i<w.length; i++)
            try
            {
                w[i].children[1].requestData(data_set);
            }
            catch(err)
            {}

        group_path = selector.selected_background; // interaction.currentViewName.split('#')[0].split('/').toSpliced(0,1).join('.');
        data_string = ""; // should be added to names to support multiple clients
        let sep = "";
        for(s of data_set)
        {
            data_string += (sep + s);
            sep = ","
         }

         while(controller.commandQueue.length>0)
            controller.get(controller.commandQueue.shift()+"/"+group_path+"?data="+encodeURIComponent(data_string), controller.update); // FIXME: ADD id in header; "?id="+controller.client_id+
        controller.queueCommand('update');
    },

    getClasses()
    {
        fetch('/classes', {method: 'GET', headers: {"Session-Id": controller.session_id, "Client-Id": controller.client_id}})
        .then(response => {
            if (!response.ok) {
                throw new Error("HTTP error " + response.status);
            }
            return response.json();
        })
        .then(json => {
            network.classes = json.classes.sort();
        })
        .catch(function () {
            console.log("Could not get class list from server.");
        })
    },

    getClassInfo()
    {
        fetch('/classinfo', {method: 'GET', headers: {"Session-Id": controller.session_id, "Client-Id": controller.client_id}})
        .then(response => {
            if (!response.ok) {
                throw new Error("HTTP error " + response.status);
            }
            return response.json();
        })
        .then(json => {
            network.classinfo = json;
        })
        .catch(function () {
            console.log("Could not get class list from server.");
        })
    },

    getFiles() { // not called
        fetch('/files', {method: 'GET', headers: {"Session-Id": controller.session_id, "Client-Id": controller.client_id}})
        .then(response => {
            if (!response.ok) {
                throw new Error("HTTP error " + response.status);
            }
            return response.json();
        })
        .then(json => {
            controller.filelist = json;
        })
        .catch(function () {
            console.log("Could not get file list from server.");
        })
    },


    updateWidgets(data)
    {
        // Update the views with data in response
        let w = document.getElementsByClassName('frame')

        for(let i=0; i<w.length; i++)
            try
            {
                w[i].children[1].receivedData = data;
                w[i].children[1].update(data); // include data for backward compatibility
            }
            catch(err)
            {
                console.log("updateWidgets failed: "+controller.client_id);
            }
    }
}

/*
 *
 * Breadcrums scripts
 *
 */

let breadcrumbs = {

    init()
    {
        breadcrumbs.breadcrumbs = document.querySelector("#breadcrumbs");
    },

    selectItem(item) // Item must be a group
    {
        const crum = breadcrumbs.breadcrumbs.querySelectorAll('.dynamic');
        crum.forEach(crum => { crum.remove(); });
        let sep = "";
        let path = "";
        let h = "";
        for(g of item.split('.'))
        {
            path += sep+g;
            sep=".";
            let styleStr = "";
            if(path==item)
            {
                styleStr = "style='--breadcrumb-element-color: var(--breadcrumb-active-color); border-radius: 5px;'";
                h += "<div class='dynamic' "+styleStr+" onclick='selector.selectItems([], \""+path+"\")'>"+g+"</div>";
            }
            else
            {
                h += "<div class='bread dynamic' "+styleStr+" onclick='selector.selectItems([], \""+path+"\")'>"+g+"</div>";
            }
        }
        h += "</div>";

        // IF GROUP BACKGROUND IS VISIBLE
/*
        h+= "<div class='add_button dynamic' style=''>Add&nbsp;Group</div>";
        h+= "<div class='add_button dynamic' style=''>Add&nbsp;Module</div>";
        h+= "<div class='add_button dynamic' style=''>Add&nbsp;Widget</div>";
*/
            document.querySelector("#nav").insertAdjacentHTML('afterend', h);
    }
}

/*
 *
 * Navigator scripts
 *
 */

let nav = {

    init() 
    {
        nav.navigator = document.getElementById('navigator');
  
    },
    toggle()
    {
        let s = window.getComputedStyle(nav.navigator , null);
        if (s.display === 'none')
            nav.navigator.style.display = 'block';
        else 
            nav.navigator.style.display = 'none';
    },
    toggleGroup(e)
    {
        if(e.target.classList.contains("group-open"))
            e.target.classList.replace("group-open", "group-closed");
        else if(e.target.classList.contains("group-closed"))
            e.target.classList.replace("group-closed", "group-open");

        e.stopPropagation();
    },
    openGroup(item)
    {
        let g = nav.navigator.querySelector("[data-name='"+item+"']");
        g = g.parentElement;
        while(g)
        {
            //g.setAttribute("class", "group-open");
            g.classList.remove("group-closed");
            g.classList.add("group-open");
            g = g.parentElement;
        }
    },
    selectItem(item)
    {
        nav.traverseAndSelect(nav.navigator, item);
        nav.traverseAndOpen(nav.navigator, item);
    },
    selectModule(evt)
    {
    
    },
    navClick(e)
    {
        let fg = bg = e.target.parentElement.dataset.name;
        selector.selectItems([], bg);
        e.stopPropagation();
    },

    buildList(group, name) {
        if (isEmpty(group)) return "";
    
        let fullName = name ? `${name}.${group.name}` : group.name;

        if(group.groups.length == 0)
        {
            return `<li data-name='${fullName}' class='group-empty' ><span onclick='return nav.navClick(event)' >${group.name}</span></li>`;
        }
        else
        {
            let s = `<li data-name='${fullName}' class='group-closed' onclick='return nav.toggleGroup(event)'><span onclick='return nav.navClick(event)' >${group.name}</span>`;
            s += `<ul>${group.groups.map(subGroup => nav.buildList(subGroup, fullName)).join('')}</ul>`;
            s +=  "</li>";
            return s;
         }
      },

      traverseAndSelect(element, data_name)
      {
        if (!element) return;
    
        if(element.dataset.name == data_name)
            element.classList.add("selected");
        else
            element.classList.remove("selected");

        if (element.children)
        {
            Array.from(element.children).forEach((child) => { nav.traverseAndSelect(child, data_name); });
        }
    },

    traverseAndOpen(element, data_name)
    {
        // Open tree to make selection visible
    },

    populate() 
    {
        nav.navigator.innerHTML = "<ul>"+nav.buildList(network.network, "")+"</ul>";
    }
}

let inspector = {
    subview: {},
    current_t_body: null,
    component: null,
    system: null,

    init()
    {
        inspector.system = document.querySelector('#system_inspector');
        inspector.component = document.querySelector('#component_inspector');

        inspector.subview.nothing  = document.querySelector('#inspector_nothing'); 
        inspector.subview.multiple  = document.querySelector('#inspector_multiple');
        inspector.subview.table  = document.querySelector('#inspector_table');
        inspector.subview.group_background = document.querySelector('#inspector_group_background');
        inspector.subview.group = document.querySelector('#inspector_group');
        inspector.subview.module =  document.querySelector('#inspector_module');
        inspector.subview.widget =  document.querySelector('#inspector_widget');

        inspector.hideSubviews();
        inspector.subview.nothing.style.display='block';
    },
    toggleSystem()
    {
        if (window.getComputedStyle(inspector.system, null).display === 'none')
        {
            inspector.system.style.display = 'block';
            inspector.component.style.display = 'none';
        }
        else
        {
            inspector.system.style.display = 'none';
            inspector.component.style.display = 'none';
        }
    },

    toggleComponent()
    {
        if (window.getComputedStyle(inspector.component, null).display === 'none')        
            {
            inspector.component.style.display = "block";
            inspector.system.style.display = "none";
        }
        else
        {
            inspector.system.style.display = "none";
            inspector.component.style.display = "none";
        }
    },

    hideSubviews()
    {
        Object.entries(inspector.subview).forEach(([key, value]) => { value.style.display = 'none'; });
    },

    setTable(table)
    {
        current_t_body = table.tBodies[0];
        current_t_body.innerHTML = "";
    },

    addTableRow(label, content="")
    {
        let row = current_t_body.insertRow(-1);
        let cell1 = row.insertCell(0);
        let cell2 = row.insertCell(1);
        cell1.innerText = label;
        cell2.innerHTML = content;
        return cell2.firstElementChild;
    },

    addHeader(header)
    {
        current_t_body.innerHTML += `<tr><td colspan='2' class='header'>${header}</td></tr>`;
    },

    addAttributeValue(attribute, value) // SET ALSO VARIABLE AND LINK GROUP for EDITABLE
    {
        this.addTableRow(attribute,value);
    },

    addMenu(name, value, opts, attribute) // SET ALSO VARIABLE AND LINK GROUP for EDITABLE
    {
        var s = '<select name="'+name+'" oninput="this">';
        for(var j in opts)
        {
            let val= opts[j];
            if(opts[j] == value)
                s += '<option value="'+val+'" selected >'+opts[j]+'</option>';
            else
                s += '<option value="'+val+'">'+opts[j]+'</option>';
        }
        s += '</select>';

        return inspector.addTableRow(name, s);
    },

    createTemplate(component)
    {
        let t = []; // {'name':'name','control':'textedit', 'type':'source'}] // FIXME: should be "identifier"
        for(let [a,v] of Object.entries(component))
        {
            if(!["name","class"].includes(a) && a[0]!='_')
             t.push({'name':a,'control':'textedit', 'type':'source'});
        }
        return t;
    },

    /* type, name, control */

    addDataRows: function (item, template, notify=null) // TODO: Add exclude list - attributes already added or added afterward
    {

        //let widget = component.widget;
        //let parameters = widget.parameters;

        inspector.item = item;
        inspector.template = template;
        inspector.notify = notify;      // object that will be notified on change

        for(let p of inspector.template)
        {
            {
                let row = current_t_body.insertRow(-1);
                if(p.name == undefined)
                    {
                        console.log("HELP!!!");
                    }
                let value = item[p.name];
                let cell1 = row.insertCell(0);
                let cell2 = row.insertCell(1);
                cell1.innerText = p.name;
                cell2.innerHTML = value != undefined ? value : "";
                cell2.setAttribute('class', p.type);
                cell2.addEventListener("paste", function(e) {
                    e.preventDefault();
                    var text = e.clipboardData.getData("text/plain");
                    document.execCommand("insertHTML", false, text); // FIXME: uses deprecated functions
                });

                switch(p.control)
                {
                    case 'header':
                        cell1.setAttribute("colspan", 2);
                        cell1.setAttribute("class", "header");
                        row.deleteCell(1);
                        break;

                    case 'textedit':
                        cell2.contentEditable = true;
                        cell2.className += ' textedit';
                        cell2.addEventListener("keypress", function(evt) {
                            if(evt.keyCode == 13)
                            {
                                evt.target.blur();
                                evt.preventDefault();
                                if(inspector.notify)
                                    inspector.notify.parameterChangeNotification(p);
                                return;
                            }
                            if(p.type == 'int' && "-0123456789".indexOf(evt.key) == -1)
                                evt.preventDefault();
                            else if(p.type == 'float' && "-0123456789.".indexOf(evt.key) == -1)
                                evt.preventDefault();
                            else if(p.type == 'source' && "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-_.0123456789*".indexOf(evt.key) == -1)
                                evt.preventDefault();
                        });
                        cell2.addEventListener("blur", function(evt) {
                            if(p.type == 'int')
                            item[p.name] = parseInt(evt.target.innerText);
                            else if(p.type == 'float')
                            item[p.name] = parseFloat(evt.target.innerText);
                            else
                            {
                                item[p.name] = evt.target.innerText.replace(String.fromCharCode(10), "").replace(String.fromCharCode(13), "");
                            // if(p.name == "style")
                                //    widget.updateStyle(widget, evt.target.innerText);     // FIXME ***********
                                //if(p.name == "frame-style")
                                //    widget.updateStyle(component, evt.target.innerText);   // FIXME ***********
                            }
                            if(inspector.notify)
                            inspector.notify.parameterChangeNotification(p);
                        });
                    break;

                    case 'slider':
                        if(p.type == 'int' || p.type == 'float')
                        {
                            cell2.innerHTML= '<div>'+value+'</div><input type="range" value="'+value+'" min="'+p.min+'" max="'+p.max+'" step="'+(p.type == 'int' ?  1: 0.01)+'"/>';
                            cell2.addEventListener("input", function(evt) {
                                evt.target.parentElement.querySelector('div').innerText = evt.target.value;
                                item[p.name] = evt.target.value;
                                if(inspector.notify) inspector.notify.parameterChangeNotification(p);
                            });
                        }
                    break;
                    
                    case 'menu':
                        var opts = p.values.split(',').map(o=>o.trim());
                        
                        var s = '<select name="'+p.name+'">';
                        for(var j in opts)
                        {
                            let value = p.type == 'int' ? j : opts[j];
                            if(opts[j] == item[p.name])
                                s += '<option value="'+value+'" selected >'+opts[j]+'</option>';
                            else
                                s += '<option value="'+value+'">'+opts[j]+'</option>';
                        }
                        s += '</select>';
                        cell2.innerHTML= s;
                        cell2.addEventListener("input", function(evt) { 
                                component[p.name] = evt.target.value.trim();
                                if(inspector.notify)
                                inspector.notify.parameterChangeNotification(p);
                            });
                    break;
                    
                    case 'checkbox':
                        if(p.type == 'bool')
                        {
                            if(value)
                                cell2.innerHTML= '<input type="checkbox" checked />';
                            else
                                cell2.innerHTML= '<input type="checkbox" />';
                            cell2.addEventListener("change", function(evt) { item[p.name] = evt.target.checked; if(notify) notify.parameterChangeNotification(p);});
                        }
                    break;
                    
                    case 'number':
                        if(p.type == 'int')
                        {
                            cell2.innerHTML= '<input type="number" value="'+value+'" min="'+p.min+'" max="'+p.max+'"/>';
                            cell2.addEventListener("input", function(evt) { 
                                item[p.name] = evt.target.value; if(inspector.notify) inspector.notify.parameterChangeNotification(p);});
                        }
                    break;

                    default:

                    break;
                }
            }
        }
    },

    parameterChangeNotification(p)
    {
        if(inspector.item._tag == "connection")
        {
            // alert("Connection parameters cannot be edited yet");
        }
        else if(inspector.item._tag == "group" && selector.selected_foreground.length == 0) // background is selected
            {
                let old_name = selector.selected_background;
                let new_name = changeNameInPath(selector.selected_background, inspector.item.name);
                if(new_name != old_name)
                {
                    network.dict[new_name] = inspector.item;
                    delete network.dict[old_name];
                    selector.selectItems([], new_name);
                }
        }
        else if (inspector.item._tag == "input")
        {
            let old_name = selector.selected_foreground[0];
            let new_name = selector.selected_background+'.'+inspector.item.name;
            let group = selector.selected_background;

            network.renameInput(group, old_name, new_name);
            selector.selectItems([new_name], null);
         }
        else if (inspector.item._tag == "output")
        {
            let old_name = selector.selected_foreground[0];
            let new_name = selector.selected_background+'.'+inspector.item.name;
            let group = selector.selected_background;

            network.renameOutput(group, old_name, new_name);
            selector.selectItems([new_name], null);
         }
        else // foreground group selected
        {
            let old_name = selector.selected_foreground[0];
            let new_name = selector.selected_background+'.'+inspector.item.name;
            let group = selector.selected_background;

            if(new_name != old_name)
            {
                network.renameGroupOrModule(group, old_name, new_name);

                // network.dict[new_name] = inspector.item;
                // delete network.dict[old_name];

                selector.selectItems([new_name], null);
            }
        }
        network.rebuildDict(); // FIXME: Remove later when done in network functions // ***** + breadcrumbs *************
        nav.populate();
    },

    showGroupBackground(bg)
    {
        inspector.hideSubviews();
        let item = network.dict[bg];

        inspector.setTable(inspector.subview.table);
        inspector.subview.table.style.display = 'block';

        let edit_mode = main.grid.style.display == 'block';
        inspector.addHeader("Group");
        if(edit_mode)
        {
            inspector.addDataRows(item, [{'name':'name', 'control':'textedit', 'type':'source'}], inspector);
        }
        else
        {
            inspector.addAttributeValue("name", item.name);
        }

        inspector.addAttributeValue("subgroups", (item.groups || []).length);
        inspector.addAttributeValue("modules", (item.modules || []).length);
        inspector.addAttributeValue("connections", (item.connections || []).length);
        inspector.addAttributeValue("widgets", (item.widgets || []).length);
    },

    showSingleSelection(c)
    {
        let item = network.dict[c];
        inspector.hideSubviews();
        inspector.setTable(inspector.subview.table);
        inspector.subview.table.style.display = 'block';

        let edit_mode = main.grid.style.display == 'block';
        if(item == undefined)
        {
            inspector.addHeader("Internal Error");
        }
        else if(item._tag == undefined)
        {
            inspector.addHeader("Unknown Component");
            inspector.addAttributeValue("name", item.name);
            inspector.addDataRows(item, inspector.createTemplate(item));
        }
        else if(item._tag=="module")
        {
            inspector.addHeader("MODULE");
            if(edit_mode)
            {
                inspector.addDataRows(item, [{'name':'name', 'control':'textedit', 'type':'source'}]); // , this
                inspector.addMenu("class", item.class, network.classes).addEventListener('change', function ()  
                { 
                    network.changeModuleClass(selector.selected_foreground[0], this.value); 
                    selector.selectItems(selector.selected_foreground);
                });
                let template = item.parameters || [];
                Object.keys(template).forEach(key => {
                    template[key].control = "textedit";
                  });

                inspector.addDataRows(item, template, this);
            }
            else
            {
                inspector.addAttributeValue("name", item.name);
                inspector.addAttributeValue("class", item.class);
            }
        }

        else if(item._tag=="group")
        {
            inspector.addHeader("GROUP");
            if(edit_mode)
            {
                inspector.addDataRows(item, [{'name':'name', 'control':'textedit', 'type':'source'}], this);
            }
            else
            {
                inspector.addAttributeValue("name", item.name);
            }
        }

        else if(item._tag=="input")
        {
            inspector.addHeader("INPUT");
            if(edit_mode)
                inspector.addDataRows(item, [{'name':'name', 'control':'textedit', 'type':'source'}], this);
            else
                inspector.addAttributeValue("name", item.name); 
        }

        else if(item._tag=="output")
        {
            inspector.addHeader("OUTPUT");

            if(edit_mode)
            {
                inspector.addDataRows(item, [{'name':'name', 'control':'textedit', 'type':'source'}, {'name':'size', 'control':'textedit', 'type':'source'}], this);
            }
            else
            {
                inspector.addAttributeValue("name", item.name);
                inspector.addAttributeValue("size", item.size);
            }
        }
        else if(item._tag=="widget")
        {
            let widget_container = document.getElementById(selector.selected_background+'.'+item.name);

            inspector.addHeader("WIDGET");
            if(edit_mode)
                {
                inspector.addMenu("class", item.class, widget_classes).addEventListener('change', function ()  
                { 
                        network.changeWidgetClass(selector.selected_foreground[0], this.value); 
                        selector.selectItems(selector.selected_foreground);
                    });
                    let template = widget_container.widget.parameter_template;
                    inspector.addDataRows(item, template, widget_container.widget);
                }
                else
                {
                    inspector.addAttributeValue("name", item.name);
                }
        }


        //inspector.addMenu("color","black",['black','red','green','blue','yellow'],"_color", c);
        /*
        inspector.addDataRows({"a":5, "b":42}, 
            [{'name':'a','control':'textedit', 'type':'int'}, 
             {'name':'b','control':'slider', 'type':'int'}]);

             inspector.addDataRows(item, inspector.createTemplate(component));
                */
    },

    showConnection(connection)
    {
        let item = network.dict[connection];

        inspector.hideSubviews();
        inspector.setTable(inspector.subview.table);
        inspector.subview.table.style.display = 'block';

        let edit_mode = main.grid.style.display == 'block';

        inspector.addHeader("Connection");
        inspector.addAttributeValue("source", item.source);
        inspector.addAttributeValue("target", item.target);

        inspector.addDataRows(item, [
            {'name':'source_range', 'control':'textedit', 'type':'source'},
            {'name':'target_range', 'control':'textedit', 'type':'source'},
            {'name':'delay_range', 'control':'textedit', 'type':'source'},
            {'name':'alias', 'control':'textedit', 'type':'source'}     
        ], this);
    },

    showMultipleSelection(n)
    {
        inspector.hideSubviews();
        inspector.setTable(inspector.subview.table);
        inspector.subview.table.style.display = 'block';
        inspector.addHeader("Multiple");
        inspector.addAttributeValue("selected", n); 
    },

    showInspectorForSelection()
    {
        if(selector.selected_connection)
            inspector.showConnection(selector.selected_connection)
        else if(selector.selected_foreground.length == 0)
            inspector.showGroupBackground(selector.selected_background);    
        else if(selector.selected_foreground.length > 1)
                inspector.showMultipleSelection(selector.selected_foreground.length);
            else
                inspector.showSingleSelection(selector.selected_foreground[0]);
    }               
}


let log = {

    init()
    {

        log.view = document.querySelector('footer');
    },

    toggleLog()
    {
        let s = window.getComputedStyle(log.view, null);
        if (s.display === 'none')
            log.view.style.display = 'block';
        else 
        log.view.style.display = 'none';
    },

    showView()
    {
        log.view.style.display = 'block';

    }
}

let selector = {

/*
 *
 * Selector     -       select in 'navigator', 'breadcrums', 'inspector' and 'main'
 * 
 */

    selected_foreground: [],
    selected_connection: null,
    selected_background: null,

    selectItems(foreground=[], background=null, toggle=false, extend=false)
    {
        selector.selected_connection = null;

        // Select background

        if(background != null)
            selector.selected_background = background;

        // Toggle foreground

        if(toggle)
            toggleStrings(selector.selected_foreground, foreground);
        else if(!selector.selected_foreground.includes(foreground[0]))
            selector.selected_foreground = foreground;

         if(extend)
            selector.selected_foreground = [...new Set([...selector.selected_foreground , ...foreground])];

        if(selector.selected_background == null)
            return;

        if(selector.selected_foreground.length==0) // select background group
        {
            nav.selectItem(selector.selected_background);
            breadcrumbs.selectItem(selector.selected_background);    
            main.selectItem([], selector.selected_background);
            inspector.showInspectorForSelection();
        }

        else // select forground components
        {
            main.selectItem(selector.selected_foreground, selector.selected_background);
            inspector.showInspectorForSelection();
        }
    },

    selectConnection(connection)
    {
        //alert("CONNECTION "+connection);
        selector.selected_foreground = [];
        selector.selected_connection = connection;
        main.selectItem(selector.selected_foreground, selector.selected_background);
        main.deselectConnection(selector.selected_connection);
        main.selectConnection(connection);
        inspector.showInspectorForSelection();
    }
}

let main = 
{
    view: null,
    grid: null,
    connections: "",
    grid_spacing: 24,
    grid_active: false,
    edit_mode: false,
    map: {},

    module_counter: 1,
    group_counter: 1,
    input_counter: 1,
    output_counter: 1,
    widget_counter: 1,
    new_position_x: 100,
    new_position_y: 100,

    init()
    {
        main.main = document.querySelector("#main");
        main.view = document.querySelector("#main_view");
        main.grid = document.querySelector("#main_grid");
        //main.group_commands = document.querySelector("#group_commands");
        main.grid_canvas = document.querySelector("#main_grid_canvas");
        main.drawGrid();

        // let g = network.dict["Brain.Amygdala.Central"];
        // main.addComponents(g);
    },

    drawGrid()
    {
        const ctx = main.grid_canvas.getContext("2d");
        ctx.fillStyle = "white";
        ctx.fillRect(0, 0, main.grid_canvas.width, main.grid_canvas.height);
        ctx.lineWidth = 0.2;
        ctx.strokeStyle = "gray";
        for(x=main.grid_spacing; x<3000; x+=main.grid_spacing)
        {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, 3000);
            ctx.moveTo(0,x);
            ctx.lineTo(3000,x);
            ctx.stroke();
        }
    },

    increaseGrid()
    {
        main.grid_spacing = main.grid_spacing*2;
        if(main.grid_spacing > 192)
        main.grid_spacing = 192;
        main.drawGrid();
    },

    decreaseGrid()
    {
        main.grid_spacing = main.grid_spacing/2;
        if(main.grid_spacing < 8)
        main.grid_spacing = 8;
        main.drawGrid();
    },

    centerComponents()
    {
        let g = network.dict[selector.selected_background];

        let sum_x = 0;
        let sum_y = 0;
        let n = 0;
        let comps = [...g.groups||[], ...g.modules||[], ...g.inputs||[], ...g.outputs||[], ...g.widgets||[]];
        for(let c of comps)    // ******* widgets??? ******* USE MIN/MAX instead?
        {
            sum_x += parseInt(c._x);
            sum_y += parseInt(c._y);
            n += 1;
        }

        if(n==0)
            return;

        let old_center_x = sum_x/n;
        let old_center_y = sum_y/n;    
        
        let new_center_x = main.main.offsetWidth/2;
        let new_center_y = main.main.offsetHeight/2;

        let dx = new_center_x-old_center_x;
        let dy = new_center_y-old_center_y;

        for(let c of comps)
        {
            c._x = `${parseInt(c._x) + dx}`;
            c._y = `${parseInt(c._y) + dy}`;
        }

            selector.selectItems([], null);
    },
    
    newModule()
    {
        let name = "Untitled_"+(Object.keys(network.dict).length+1);
        let m =
        {
            'name':name,
            'class':"Module",
            '_tag':"module",
            '_x':main.new_position_x,
            '_y':main.new_position_y,
            'inputs': [],
            'outputs': [],
            'parameters':[]
        };
        let full_name = selector.selected_background+'.'+name;
        network.dict[selector.selected_background].modules.push(m);
        network.dict[full_name]=m;
        // TODO: SEND TO KERNEL **********************************************
        main.new_position_x += 30;
        main.new_position_y += 30;

        if(main.new_position_y >600)
        {
            main.new_position_x -= 350;
            main.new_position_y = 100;   
        }
        nav.populate();
        selector.selectItems([full_name]);
    },

    newGroup() // FIXME: Move to network
    {
        let name = "Group_"+(Object.keys(network.dict).length+1);
        let m =
        {
            'name':name,
            '_tag':"group",
            '_x':main.new_position_x,
            '_y':main.new_position_y,
            'inputs': [],
            'outputs': [],
            'parameters':[],
            'modules': [],
            'groups':[],
            'widgets':[]
        };
        let full_name = selector.selected_background+'.'+name;
        network.dict[selector.selected_background].groups.push(m);
        network.dict[full_name]=m;
        // TODO: SEND TO KERNEL **********************************************
        main.new_position_x += 30;
        main.new_position_y += 30;

        if(main.new_position_y >600)
        {
            main.new_position_x -= 350;
            main.new_position_y = 100;   
        }

        nav.populate();
        selector.selectItems([full_name]);
    },

    newInput() // FIXME: Move to network
    {
        let name = "Input_"+(Object.keys(network.dict).length+1);
        let m =
        {
            'name':name,
            '_tag':"input",
            '_x':main.new_position_x,
            '_y':main.new_position_y
        };
        let full_name = selector.selected_background+'.'+name;
        network.dict[selector.selected_background].inputs.push(m);
        network.dict[full_name]=m;
        // TODO: SEND TO KERNEL **********************************************
        main.new_position_x += 30;
        main.new_position_y += 30;

        if(main.new_position_y >600)
        {
            main.new_position_x -= 350;
            main.new_position_y = 100;   
        }
        selector.selectItems([full_name]);
    },

    newOutput() // FIXME: Move to network
    {
        let name = "Output_"+(Object.keys(network.dict).length+1);
        let m =
        {
            'name':name,
            //'size':"1",
            '_tag':"output",
            '_x':main.new_position_x,
            '_y':main.new_position_y
        };
        let full_name = selector.selected_background+'.'+name;
        network.dict[selector.selected_background].outputs.push(m);
        network.dict[full_name]=m;
        main.new_position_x += 30;
        main.new_position_y += 30;

        if(main.new_position_y >600)
        {
            main.new_position_x -= 350;
            main.new_position_y = 100;   
        }
        selector.selectItems([full_name]);
    },

    newWidget() // (w) FIXME: Move (data structure only) to network
    {
        let name = "Widget_"+(Object.keys(network.dict).length+1);
        let w = {
            "_tag": "widget",
            "name": name,
            "title": name,
            "class": "bar-graph",
            '_x':main.new_position_x,
            '_y':main.new_position_y,
            'width': 200,
            'height': 200
        };

        let full_name = selector.selected_background+'.'+name;
        network.dict[selector.selected_background].widgets.push(w);
        network.dict[full_name]=w;
        selector.selectItems([full_name]);

        main.new_position_x += 30;
        main.new_position_y += 30;

        if(main.new_position_y >600)
        {
            main.new_position_x -= 350;
            main.new_position_y = 100;   
        }



        return; // The rest should be moved to view building

        let newObject = document.createElement("div");
        newObject.setAttribute("class", "frame visible");

        let newTitle = document.createElement("div");
        newTitle.setAttribute("class", "title");
        newTitle.innerHTML = w.name;
        newObject.appendChild(newTitle);

        let index = main.view.querySelectorAll(".widget").length;
        main.view.appendChild(newObject);
        newObject.addEventListener('mousedown', main.startDragComponents, false);

        let constr = webui_widgets.constructors["webui-widget-"+w['class']];
        if(!constr)
        {
            console.log("Internal Error: No constructor found for "+"webui-widget-"+w['class']);
            newObject.widget = new webui_widgets.constructors['webui-widget-text'];
            newObject.widget.element = newObject; // FIXME: why not also below??
            //newObject.widget.groupName = this.currentViewName.split('#')[0].split('/').slice(1).join('.');   // get group name - temporary ugly solution
            newObject.widget.parameters['text'] = "\""+"webui-widget-"+w['class']+"\" not found. Is it included in index.html?";
            newObject.widget.parameters['_index_'] = index;
        }
        else
        {
            newObject.widget = new webui_widgets.constructors["webui-widget-"+w['class']];
            //newObject.widget.groupName = this.currentViewName.split('#')[0].split('/').slice(1).join('.');   // get group name - temporary ugly solution
            // Add default parameters from CSS - possibly...
            for(let k in newObject.widget.parameters)
            if(w[k] === undefined)
            {
                w[k] = newObject.widget.parameters[k];
            }
            else
            {
                let tp = newObject.widget.param_types[k]
                w[k] = setType(w[k], tp);
            }

            newObject.widget.parameters = w;
            newObject.widget.parameters['_index_'] = index;
        }

        newObject.widget.setAttribute('class', 'widget');
        newObject.appendChild(newObject.widget);    // must append before next section

        // Section below should not exists - probably...

        newObject.style.top = w._y+"px";
        newObject.style.left = w._x+"px";
        newObject.style.width = (w.width || 200) +"px";
        newObject.style.height = (w.height || 200) +"px";

        newObject.handle = document.createElement("div");
        newObject.handle.setAttribute("class", "handle");
        newObject.handle.onmousedown = main.startResize;
        newObject.appendChild(newObject.handle);

        newObject.setAttribute("id", full_name);
        
        network.dict[full_name]=w;

        selector.selectItems([full_name]);

        try
        {
            newObject.widget.updateAll();
        }
        catch(err)
        {
            console.log(err);
        }
        
        return newObject;
    },


    deleteComponent()
    {
        alert("No, still not possible...");
    },

    changeComponentPosition(c, dx,dy)
    {
        let e = document.getElementById(c);

        let new_x = main.map[c][0] + dx;
        let new_y = main.map[c][1] + dy;

        if(new_x < 0)
                new_x = 0;

            if(new_y < 0)
                new_y = 0;

        if(main.grid_active) // snap
        {
            let g = main.grid_spacing;
            new_x = g*Math.round(new_x/g);
            new_y = g*Math.round(new_y/g);
        }

        e.style.left = new_x+"px";
        e.style.top = new_y+"px";

        network.dict[c]._x = new_x;
        network.dict[c]._y = new_y;
    },


    changeComponentSize(dX, dY)
    {
        let w_id = selector.selected_foreground[0];
        let w = document.getElementById(w_id);

        let newWidth = main.grid_spacing*Math.round((main.startX + dX)/main.grid_spacing)+1;
        let newHeight = main.grid_spacing*Math.round((main.startY + dY)/main.grid_spacing)+1;
        w.style.width = newWidth + 'px';
        w.style.height = newHeight + 'px';

        network.dict[w_id].width = newWidth;
        network.dict[w_id].height = newHeight;

        w.widget.parameterChangeNotification(network.dict[w_id]);
    },

    moveComponents(evt)
    {
        if(Object.keys(main.map).length == 0) // FIME: SHOULD NOT BE NEEDED
            return;

        evt.stopPropagation()
        let dx = evt.clientX - main.initialMouseX;
        let dy = evt.clientY - main.initialMouseY;

        for(let c of selector.selected_foreground)
            main.changeComponentPosition(c, dx,dy);
        main.addConnections();
    },


    releaseComponents(evt)
    {
        main.view.removeEventListener('mousemove',main.moveComponents, true);
        main.view.removeEventListener('mouseup',main.releaseComponents, true);
        main.view.removeEventListener('mousemove',main.moveComponents, false);
        main.view.removeEventListener('mouseup',main.releaseComponents, false);
        main.map = {};
    },

    releaseResizeComponent(evt)
    {
        main.view.removeEventListener('mousemove',main.resizeComponent, true);
        main.view.removeEventListener('mouseup',main.releaseResizeComponent, true);
        main.view.removeEventListener('mousemove',main.resizeComponent, false);
        main.view.removeEventListener('mouseup',main.releaseResizeComponent, false);
        main.map = {}; // just in case

        let dX = evt.clientX - main.initialMouseX;
        let dY = evt.clientY - main.initialMouseY;
        main.changeComponentSize(dX,dY);

        let w_id = selector.selected_foreground[0];
        let w = document.getElementById(w_id);

        let newWidth = main.grid_spacing*Math.round((main.startX + dX)/main.grid_spacing)+1;
        let newHeight = main.grid_spacing*Math.round((main.startY + dY)/main.grid_spacing)+1;
        w.style.width = newWidth + 'px';
        w.style.height = newHeight + 'px';

        network.dict[w_id].width = newWidth;
        network.dict[w_id].height = newHeight;
    },

    startResize(evt)
    {
        evt.stopPropagation();
        main.startX = this.offsetLeft;
        main.startY = this.offsetTop;
        selector.selectItems([this.parentElement.id], null);
        this.parentElement.classList.add("resized");

        main.initialMouseX = evt.clientX;
        main.initialMouseY = evt.clientY;
        main.view.addEventListener('mousemove',main.resizeComponent,true);
        main.view.addEventListener('mouseup',main.releaseResizeComponent,true);
        return false;
    },

    resizeComponent: function (evt) {
        let dX = evt.clientX - main.initialMouseX;
        let dY = evt.clientY - main.initialMouseY;
        main.changeComponentSize(dX,dY);
        return false;
    },

    startDragComponents(evt)
    {
        if(!main.edit_mode)
            return;

    //    alert("startDragComponents");

        if(evt.detail == 2) // ignore double clicks
            return;

        main.initialMouseX = evt.clientX;
        main.initialMouseY = evt.clientY;

        selector.selectItems([this.dataset.name], null, evt.shiftKey);
        //evt.stopPropagation();

        if(!selector.selected_foreground.includes(this.dataset.name))
            return;

        main.map = {};
        for(let c of selector.selected_foreground)
        {
            let e = document.getElementById(c);
            main.map[c] = [e.offsetLeft, e.offsetTop];
        };

        main.view.addEventListener('mousemove',main.moveComponents, true);
        main.view.addEventListener('mouseup',main.releaseComponents,true);
        //main.view.style.cursor = "e-resize";
        //main.view.cursor = "e-resize";
        return false;
    },

    startTrackConnection(evt)
    {
        this.style.backgroundColor="orange";
        evt.stopPropagation();
        let id = this.id;
        let e = document.getElementById(id);
        let x = e.getBoundingClientRect().left-main.view.getBoundingClientRect().left+4.5;
        let y = e.getBoundingClientRect().top-main.view.getBoundingClientRect().top+4.5;

        main.tracked_connection =  { "x1": x, "y1": y, "x2": x, "y2": y, "source": id.split(':')[0], "target": null, "source_element": e };
        main.view.addEventListener('mousemove',main.moveTrackedConnection, true);
        main.view.addEventListener('mouseup',main.releaseTrackedConnection,true);
    },

    moveTrackedConnection(evt)
    {
        if(!main.tracked_connection)
            return;
        //evt.stopPropagation();

        let ox = main.view.getBoundingClientRect().left;
        let oy = main.view.getBoundingClientRect().top;

        main.tracked_connection.x2 = evt.clientX - ox;
        main.tracked_connection.y2 = evt.clientY - oy;
        main.addConnections();
    },

    releaseTrackedConnection(evt)
    {
        evt.stopPropagation();
        if(!main.tracked_connection)
            return;

        if(main.tracked_connection.target==null)
        {
            main.tracked_connection = null;
        }
        else
        {
            let source = removeStringFromStart(main.tracked_connection.source.split(':')[0], selector.selected_background+".")
            let target = removeStringFromStart(main.tracked_connection.target.split(':')[0], selector.selected_background+".")
            network.newConnection(selector.selected_background, source, target);
            main.tracked_connection = null;
            //main.addConnections();
            selector.selectConnection(selector.selected_background+"."+source+"*"+selector.selected_background+"."+target);
        }
    },

    setConnectectionTarget(evt)
    {
        if(!main.tracked_connection)
            return;
        this.style.backgroundColor="orange";
        main.tracked_connection.target = this.id;
    },

    resetConnectectionTarget(evt)
    {
        if(!main.tracked_connection)
            return;
            main.tracked_connection.target = null;
        this.style.backgroundColor="gray";
    },

    addGroup(g,path)
    {
        let fullName = `${path}.${g.name}`;
        let s = "";
        s += `<div class='gi module group' style='top:${g._y}px;left:${g._x}px;'  id='${fullName}' data-name='${fullName}'>`;
        s += `<table>`;
        s += `<tr><td class='title' colspan='3'>${g.name}</td></tr>`;

       for(let i of g.inputs || [])
           s += `<tr><td class='input'><div class='i_spot' id='${path}.${g.name}.${i.name}:in' onclick='alert(this.id)'></div></td><td>${i.name}</td><td></td></tr>`;
 
       for(let o of g.outputs || [])
           s += `<tr><td></td><td>${o.name}</td><td class='output'><div  class='o_spot' id='${path}.${g.name}.${o.name}:out'></div></td></tr>`;

        s += `</table>`;
        s += `</div>`;
       main.view.innerHTML += s;
   },

    addInput(i,path)
    {
        main.view.innerHTML += `<div class='gi group_input' id='${path}.${i.name}' data-name='${path}.${i.name}' style='top:${i._y}px;left:${i._x}px;'>
        ${i.name}
        <div class='o_spot'  id='${path}.${i.name}:out'></div>
        </div>`;
    },

    addOutput(o,path)
    {
        main.view.innerHTML += `<div class='gi group_output'  id='${path}.${o.name}' data-name='${path}.${o.name}'  style='top:${o._y}px;left:${o._x}px;'><div class='i_spot' id='${path}.${o.name}:in'></div>${o.name}</div>`;
    },

    addModule(m,path)
    {
         let s = "";
         s += `<div class='gi module' style='top:${m._y}px;left:${m._x}px;'   id='${path}.${m.name}' data-name='${path}.${m.name}'>`;
         s += `<table>`;
         s += `<tr><td class='title' colspan='3'>${m.name}</td></tr>`;

        for(let i of m.inputs || [])
            s += `<tr><td class='input'><div class='i_spot' id='${path}.${m.name}.${i.name}:in' onclick='alert(this.id)'></div></td ><td>${i.name}</td><td class='output'></td></tr>`;
  
        for(let o of m.outputs || [])
            s += `<tr><td class='output'></td><td>${o.name}</td><td class='output'><div class='o_spot' id='${path}.${m.name}.${o.name}:out'></div></td></tr>`;

         s += `</table>`;
         s += `</div>`;
        main.view.innerHTML += s;
    },

    addWidget(w,path)
    {
        // main.view.innerHTML += `<div class='gi widget' style='height:40px; width:160px;background-color:#dd0;border:1px solid #666;position:absolute;top:${w._y}px;left:${w._x}px;'>${w.name}</div>`;

        let newObject = document.createElement("div");
        newObject.setAttribute("class", "frame visible gi widget");

        let newTitle = document.createElement("div");
        newTitle.setAttribute("class", "title");
        newTitle.innerHTML = w.name;
        newObject.appendChild(newTitle);

        let index = main.view.querySelectorAll(".widget").length;
        main.view.appendChild(newObject);
        newObject.addEventListener('mousedown', main.startDragComponents, false);

        let constr = webui_widgets.constructors["webui-widget-"+w['class']];
        if(!constr)
        {
            console.log("Internal Error: No constructor found for "+"webui-widget-"+w['class']);
            newObject.widget = new webui_widgets.constructors['webui-widget-text'];
            newObject.widget.element = newObject; // FIXME: why not also below??
            //newObject.widget.groupName = this.currentViewName.split('#')[0].split('/').slice(1).join('.');   // get group name - temporary ugly solution
            newObject.widget.parameters['text'] = "\""+"webui-widget-"+w['class']+"\" not found. Is it included in index.html?";
            newObject.widget.parameters['_index_'] = index;
        }
        else
        {
            newObject.widget = new webui_widgets.constructors["webui-widget-"+w['class']];
            for(let k in newObject.widget.parameters)
            if(w[k] === undefined)
            {
                w[k] = newObject.widget.parameters[k];
            }
            else
            {
                let tp = newObject.widget.param_types[k]
                w[k] = setType(w[k], tp);
            }

            newObject.widget.parameters = w;
            newObject.widget.parameters['_index_'] = index;
        }

        newObject.widget.setAttribute('class', 'widget');
        newObject.appendChild(newObject.widget);    // must append before next section

        // Section below should not exists - probably...

        newObject.style.top = w._y+"px";
        newObject.style.left = w._x+"px";
        newObject.style.width = (w.width || 200) +"px";
        newObject.style.height = (w.height || 200) +"px";

        newObject.handle = document.createElement("div");
        newObject.handle.setAttribute("class", "handle");
        newObject.handle.onmousedown = main.startResize;    // SET IN ADD COMPONENTS
        newObject.appendChild(newObject.handle);

        let full_name = `${path}.${w.name}`;
        newObject.setAttribute("id", full_name);
        newObject.setAttribute("data-name", full_name);

        try
        {
            newObject.widget.updateAll();
        }
        catch(err)
        {
            console.log(err);
        }
        
    },
    
    addConnection(c,path)
    {
        let source_point = document.getElementById(`${path}.${c.source}:out`);
        let target_point = document.getElementById(`${path}.${c.target}:in`);

        if(source_point == undefined ||target_point == undefined)
            return;
        
        let ox = main.view.getBoundingClientRect().left;
        let oy = main.view.getBoundingClientRect().top;

        let x1 = source_point.getBoundingClientRect().left-ox+4.5;
        let y1 = source_point.getBoundingClientRect().top-oy+4.5;

        let x2 = target_point.getBoundingClientRect().left-ox+4.5;
        let y2 = target_point.getBoundingClientRect().top-oy+4.5;

        let cc = `<line x1='${x1}' y1='${y1}' x2='${x2}' y2='${y2}' class='connection_line' data-source='${c.source}' id="${path}.${c.source}*${path}.${c.target}" data-target='${c.target}'onclick='selector.selectConnection("${path}.${c.source}*${path}.${c.target}")'/>`; 
        main.connections += cc;
    },

    addTrackedConnection()
    {
        if(!main.tracked_connection)
            return;
        let x1 = main.tracked_connection.x1;
        let y1 = main.tracked_connection.y1;
        let x2 = main.tracked_connection.x2;
        let y2 = main.tracked_connection.y2;
        let cc = `<line x1='${x1}' y1='${y1}' x2='${x2}' y2='${y2}' class='connection_line tracked'/>`; 
        main.connections += cc;
    },

    selectConnection(connection)
    {
        let c = document.getElementById(connection);
        if(c != undefined) // FIXME: Use exception above instead
        {
            c.classList.add("selected");

            let st = connection.split('*');

            let s = document.getElementById(st[0]+":out");
            s.style.backgroundColor = "orange";

            let t = document.getElementById(st[1]+":in");
            t.style.backgroundColor = "orange";
        }
    },

    deselectConnection(connection)
    {
        let c = document.getElementById(connection);
        if(c != undefined) // FIXME: Use exception above instead
            c.classList.remove("selected");

    },

    addConnections()
    {
        let path = selector.selected_background;
        let group = network.dict[path];
        let s = document.getElementById("connections");
        if(s)
            s.innerHTML = "";
        main.connections = "<svg xmlns='http://www.w3.org/2000/svg' id='connections'>";
        for(let c of group.connections || [])
            main.addConnection(c,path);
        main.addTrackedConnection();
        main.connections += "</svg>";
        s.innerHTML += main.connections;
    },

    addComponents(group, selectionList, path)
    {
        if(group == undefined)
            return;

        main.view.innerHTML = "";

        for(let i of group.inputs || [])
            main.addInput(i,path);

        for(let o of group.outputs || [])
                main.addOutput(o,path);

            for(let g of group.groups || [])
                main.addGroup(g,path);

        for(let m of group.modules || [])
            main.addModule(m,path);

        for(let w of group.widgets || [])
            main.addWidget(w,path);

        main.addConnections();

        // Add object handlers

        for(let e of main.view.querySelectorAll(".group"))
            e.ondblclick = function(evt) { selector.selectItems([], this.dataset.name); 
            }; // Jump into group

        for(let e of main.view.querySelectorAll(".gi"))
        {
            e.addEventListener('mousedown', main.startDragComponents, false);
            if(selectionList.includes(e.dataset.name))
                e.classList.add("selected")
            else
                e.classList.remove("selected");
        }

        for(let o of main.view.querySelectorAll(".o_spot"))
            o.addEventListener('mousedown', main.startTrackConnection, false);

            for(let i of main.view.querySelectorAll(".i_spot"))
            {
                i.addEventListener('mouseover', main.setConnectectionTarget, true);
                i.addEventListener('mouseleave', main.resetConnectectionTarget, true);
            }
    },

    cancelEditMode()
    {
        main.grid.style.display = 'none';
        main.edit_mode = false;
    },

    toggleEditMode()
    {
        let s = window.getComputedStyle(main.grid, null);
        if (s.display === 'none')
        {
            main.grid.style.display = 'block';
            main.edit_mode = true;
            main.grid_canvas.style.display = 'block';
            main.grid_active = true;

            controller.run_mode = 'stop';
            controller.get("stop", controller.update)
        }
        else
        {
            main.grid.style.display = 'none';
            main.edit_mode = false;
            main.grid_canvas.style.display = 'none';
            main.grid_active = false;
        }
        inspector.showInspectorForSelection();
    },

    toggleGrid()
    {
        let s = window.getComputedStyle(main.grid_canvas, null);
        if (s.display === 'none')
        {
         main.grid_canvas.style.display = 'block';
         main.grid_active = true;
        }
        else
        {
            main.grid_canvas.style.display = 'none';
            main.grid_active = false;
        }
    },

    selectItem(foreground, background)
    {
        if(background != null)
        {
            let group = network.dict[background];
            main.addComponents(group, foreground, background);
        }
    },


    /*
        TODO:
            Tab: select next component (in main view; not in inspectors)
            shift+Tab: select previous component (in main view; not in inspectors)
            PageUp: exit current group
            Enter/PageDown: jump into selected group
            Arrow keys: jump between components
    */

    keydown(evt)
    {
        if(evt.key== "Escape")
        {
            inspector.toggleSystem();
            evt.preventDefault();
            return;
        }

        if(evt.key== "Backspace")
        {

            return;
            alert("Delete selected items. (NOT IMPLEMENTED YET)");
        }

        if(!evt.metaKey)
            return;
        if(evt.keyIdentifier=="Meta")
            return;

        if(evt.key=="a")
        {
            let bg = selector.selected_background;
            let g = network.dict[bg];
            let comps = [...g.groups||[], ...g.modules||[], ...g.inputs||[], ...g.outputs||[], ...g.widgets||[]];
            selector.selectItems(comps.map((x) => bg+'.'+x.name), null, false, true);
        }

        else if (evt.key=="c")
        {
            evt.preventDefault();
            alert("Copy selected items. (NOT IMPLEMENTED YET)");
        }

        else if (evt.key=="x")
        {
            evt.preventDefault();
            alert("Cut selected items. (NOT IMPLEMENTED YET)");
        }


        else if (evt.key=="v")
        {
            evt.preventDefault();
            alert("Paste copied items. (NOT IMPLEMENTED YET)");
        }

        else if (evt.key=="d")
        {
            evt.preventDefault();
            alert("Duplicate selected items and possibly connections. (NOT IMPLEMENTED YET)");
        }
/*
        else if (evt.key=="g")
        {
            evt.preventDefault();
            main.toggleGrid();
            return;
        }
*/
        else if (evt.key=="i")
        { 
            evt.preventDefault();
            inspector.toggleComponent();
            return;
        }
    }
}


let brainstudio = {

    init()
    {
        log.init();
        inspector.init();
        controller.init();
        nav.init();
        breadcrumbs.init();
        main.init();

        document.onkeydown = function (evt){ main.keydown(evt) };
    }
}

