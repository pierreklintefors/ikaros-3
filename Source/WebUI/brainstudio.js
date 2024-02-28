/*
 * New version of WebUI interface wit BrainStudio functions
*/

function isEmpty(obj) 
{
    if(obj)
        for (const prop in obj) {
        if (Object.hasOwn(obj, prop)) {
            return false;
        }
        }

    return true;
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



network = {
    network: null
}

webui_widgets = {
    constructors: {},
    add: function(element_name, class_object) {
        customElements.define(element_name, class_object);
        webui_widgets.constructors[element_name] = class_object;
    }
};


controller = {
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
        controller.reconnect_timer = setInterval(controller.reconnect, controller.reconnect_interval);
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
        controller.getFiles();
        controller.requestUpdate();
        controller.reconnect_timer = setInterval(controller.reconnect, controller.reconnect_interval);
    },
    
    queueCommand: function (command) {
        controller.commandQueue.push(command);
    },

    new: function () {
        controller.queueCommand('new');
    },

    openCallback: function(x)
    {
        controller.get("open?file="+x, controller.update);
    },

    open: function () {

        dialog.showOpenDialog(controller.filelist, controller.openCallback, "Select file to open");
    },

    save: function () {
        controller.get("save", controller.update);
    },

    saveas: function () {
        alert("SAVE AS coming soon");
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
        controller.queueCommand('pause');
    },
    
    step: function ()
    {
        document.querySelector("#state").innerText = "step";
        controller.queueCommand('step');
    },
    
    play: function () {
        controller.queueCommand('play');
    },
    
    realtime: function () {
        controller.queueCommand('realtime');
    },

    start: function () {
        controller.play();  // FIXME: possibly start selected mode play/fast-forward/realtime
    },

    getGroup(path) // FIXME: 
    {
        let p = path.split("/");
        let g = [controller.network];
        for(let i in p)
        {
            // Find group named p[i]

            for(let gi in g)
            {
                if(g[gi].name == p[i])
                {
                    g = g[gi];
                }

                if(i == p.length-1)
                {
                    return g;
                }
            }
        }
        return null;
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
            //nav.init(response);
            controller.network = response;
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

        // Request new data
        let data_set = new Set();
        
        // Very fragile loop - maybe use class to mark widgets or something
        let w = document.getElementsByClassName('frame');
        for(let i=0; i<w.length; i++)
            try
            {
                w[i].children[1].requestData(data_set);
            }
            catch(err)
            {}

        let group_path = interaction.currentViewName.split('#')[0].split('/').toSpliced(0,1).join('.');
        let data_string = ""; // should be added to names to support multiple clients
        let sep = "";
        for(s of data_set)
        {
            data_string += (sep + s);
            sep = ","
         }
*/

         while(controller.commandQueue.length>0)
            controller.get(controller.commandQueue.shift()+group_path+"?data="+encodeURIComponent(data_string), controller.update); // FIXME: ADD id in header; "?id="+controller.client_id+
        controller.queueCommand('update/');
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
            interaction.classes = json.classes.sort();
        })
        .catch(function () {
            console.log("Could not get class list from server.");
        })
    },

    getFiles() {
        fetch('/files', {method: 'GET', headers: {"Session-Id": controller.session_id, "Client-Id": controller.client_id}})
        .then(response => {
            if (!response.ok) {
                throw new Error("HTTP error " + response.status);
            }
            return response.json();
        })
        .then(json => {
            controller.filelist = json.files || [];
        })
        .catch(function () {
            console.log("Could not get file list from server.");
        })
    }
}

inspector = {
    init()
    {
        inspector.system = document.querySelector('#system_inspector');
    },

    showInspector(i)
    {
        for (const s of document.querySelectorAll('aside'))
            if(s === i)
                s.style.display = 'block';
            else
                s.style.display = 'none';
    },

    toggleSystem()
    {
        if (window.getComputedStyle(inspector.system, null).display === 'none')
            inspector.showInspector(inspector.system);
        else
            inspector.showInspector(null);
    }
}


log = {

    init()
    {

        log.view = document.querySelector('footer');
    },

    toggleView()
    {
        var s = window.getComputedStyle(log.view, null);
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


brainstudio = {

    init()
    {
        log.init();
        inspector.init();
        controller.init();
    }
}



/*
 *
 * Dialog Scrips
 *
 *
 */

dialog = {
    confirmOpen: function()
    {
        let sel = document.getElementById("openDialogItems");
        let text= sel.options[sel.selectedIndex].text;
        dialog.window.close(text);
            if(dialog.callback)
                dialog.callback(text);
    },
        
    cancelOpen: function()
    {
        dialog.window.close(null);
    },

    showOpenDialog: function (file_list, callback, message)
    {
        dialog.callback = callback;
        dialog.window = document.getElementById('openDialog');
        let sel = document.getElementById('openDialogItems');
        sel.innerHTML = '';
        if(file_list)
            for(i of file_list) // FIXME: TEST was .split(",")
            {
                var opt = document.createElement('option');
                opt.value = i;
                opt.innerHTML = i;
                document.getElementById('openDialogItems').appendChild(opt);
            }
            if(message)
            {
                document.getElementById('openDialogTitle').innerText = message;
            }
        dialog.window.showModal();
    }
}

