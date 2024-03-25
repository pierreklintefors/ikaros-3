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


  function toggleInArray(array, value) 
  {
    var index = array.indexOf(value);

    if (index == -1) 
    {
        array.push(value);
    } 
    else 
    {
        do 
        {
            array.splice(index, 1);
            index = array.indexOf(value);
        }
        while (index != -1);
    }
  }
  

  function toggleStrings(array, toggleItems)
  {
    for(let i of toggleItems)
        toggleInArray(array, i);
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



network = {
    network: null,
    dict: {},

    init(n)
    {
        network.network = n;
        network.buildDict(n, "");
    },

    buildDict(o, path)
    {
        network.dict[(path+'.'+o.name).substring(1)] = o;
        for(g of o.groups || [])
            network.buildDict(g, path+'.'+o.name)
        for(m of o.modules || [])
            network.dict[(path+'.'+o.name).substring(1)+'.'+m.name] = m;
        for(v of o.views || [])
            network.dict[(path+'.'+o.name).substring(1)+'/'+v.name] = v;

    }
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
            network.init(response);
            nav.populate(network.network);
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

breadcrumbs = {

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

    populate(net) 
    {
        nav.navigator.innerHTML = "<ul>"+nav.buildList(net, "")+"</ul>";
    }
}



inspector = {
    subview: {},
    current_t_body: null,

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
        if (window.getComputedStyle(inspector.component, null).display === 'none')        {
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

    addHeader(header)
    {
        current_t_body.innerHTML += `<tr><td colspan='2' class='header'>${header}</td></tr>`;
    },

    addAttributeValue(attribute, value) // SET ALSO VARIABLE AND LINK GROUP for EDITABLE
    {
        current_t_body.innerHTML += `<tr><td>${attribute}</td><td>${value}</td></tr>`;
    },

    showGroupBackground(item)
    {
        let g = network.dict[item];

        inspector.hideSubviews();
        inspector.setTable(inspector.subview.table);
        inspector.subview.table.style.display = 'block';
        inspector.addHeader("GROUP");
        inspector.addAttributeValue("name", g.name);
        inspector.addAttributeValue("subgroups", (g.groups || []).length);
        inspector.addAttributeValue("modules", (g.modules || []).length);
        inspector.addAttributeValue("connections", (g.connections || []).length);
    },

    showSingleSelection(c)
    {
        inspector.hideSubviews();
        inspector.setTable(inspector.subview.table);
        inspector.subview.table.style.display = 'block';
        inspector.addHeader("COMPONENT");
        inspector.addAttributeValue("name", c); 
    },

    showMultipleSelection(n)
    {
        inspector.hideSubviews();
        inspector.setTable(inspector.subview.table);
        inspector.subview.table.style.display = 'block';
        inspector.addHeader("MULTIPLE");
        inspector.addAttributeValue("selected", n); 
    }
}


log = {

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

/*
 *
 * Selector     -       select in 'navigator', 'breadcrums', 'inspector' and 'main'
 * 
 */

selector = {
    selected_foreground: [],
    selected_background: null,

    selectItems(foreground=[], background=null, toggle)
    {
        // Select background

        if(background != null)
            selector.selected_background = background;

        // Toggle foreground

        if(toggle)
            toggleStrings(selector.selected_foreground, foreground);
        else if(!selector.selected_foreground.includes(foreground[0]))
            selector.selected_foreground = foreground;

        if(selector.selected_background == null)
            return;

        if(selector.selected_foreground.length==0) // select background group
        {
            nav.selectItem(selector.selected_background);
            breadcrumbs.selectItem(selector.selected_background);    
            main.selectItem([], selector.selected_background);
            inspector.showGroupBackground(selector.selected_background); // FIXME: for edit *****
        }

        else // select forground components
        {
            main.selectItem(selector.selected_foreground, selector.selected_background);
            if(selector.selected_foreground.length > 1)
                inspector.showMultipleSelection(selector.selected_foreground.length);
            else
                inspector.showSingleSelection(selector.selected_foreground[0]);
        }
    },

    selectConnection(source, target)
    {
        alert(source+" => "+target);
    }
}

main = 
{
    view: null,
    grid: null,
    connections: "",
    grid_spacing: 25,

    init()
    {
        main.view = document.querySelector("#main_view");
        main.grid = document.querySelector("#main_grid");
        main.drawGrid();

        let g = network.dict["Brain.Amygdala.Central"];
        main.placeObjects(g);
    },

    drawGrid()
    {
        const ctx = main.grid.getContext("2d");
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

    addGroup(g,path)
    {
        let fullName = `${path}.${g.name}`;
        let s = "";
        s += `<div class='gi module' style='top:${g._y}px;left:${g._x}px;'  data-name='${fullName}'>`;
        s += `<table>`;
        s += `<tr><td class='title' colspan='3'>${g.name}</td></tr>`;

       for(let i of g.inputs || [])
           s += `<tr><td class='input'><div class='i_spot' id='${path}.${g.name}.${i.name}' onclick='alert(this.id)'></div></td><td>${i.name}</td><td></td></tr>`;
 
       for(let o of g.outputs || [])
           s += `<tr><td></td><td>${o.name}</td><td class='output'><div class='o_spot' id='${path}.${g.name}.${o.name}'></div></td></tr>`;

        s += `</table>`;
        s += `</div>`;
       main.view.innerHTML += s;
   },

    addInput(i,path)
    {
        main.view.innerHTML += `<div class='gi group_input' data-name='${path}.${i.name}' style='top:${i._y}px;left:${i._x}px;'>${i.name}<div class='o_spot' id='${path}.${i.name}'></div></div>`;
    },

    addOutput(o,path)
    {
        main.view.innerHTML += `<div class='gi group_output' data-name='${path}.${o.name}'  style='height:40px; width:160px;background-color:cyan;border:1px solid #666;position:absolute;top:${o._y}px;left:${o._x}px;'>${o.name}<div class='o_spot' id='${path}.${o.name}'></div>`; // ****NO COMPLETE
    },

    addModule(m,path)
    {
         let s = "";
         s += `<div class='gi module' style='top:${m._y}px;left:${m._x}px;'  data-name='${path}.${m.name}'>`;
         s += `<table>`;
         s += `<tr><td class='title' colspan='3'>${m.name}</td></tr>`;

        for(let i of m.inputs || [])
            s += `<tr><td class='input'><div class='i_spot' id='${path}.${m.name}.${i.name}' onclick='alert(this.id)'></div></td ><td>${i.name}</td><td class='output'></td></tr>`;
  
        for(let o of m.outputs || [])
            s += `<tr><td class='output'></td><td>${o.name}</td><td class='output'><div class='o_spot' id='${path}.${m.name}.${o.name}'></div></td></tr>`;

         s += `</table>`;
         s += `</div>`;
        main.view.innerHTML += s;
    },
    
    addWidget(w,path)
    {
        main.view.innerHTML += `<div class='gi widget' style='height:40px; width:160px;background-color:#dd0;border:1px solid #666;position:absolute;top:${w._y}px;left:${w._x}px;'>${w.name}</div>`;
    },
    
    addConnection(c,path)
    {
        let source_point = document.getElementById(`${path}.${c.source}`);
        let target_point = document.getElementById(`${path}.${c.target}`);

        if(source_point == undefined ||target_point == undefined)
            return;
        
        let ox = main.view.getBoundingClientRect().left;
        let oy = main.view.getBoundingClientRect().top;

        let x1 = source_point.getBoundingClientRect().left-ox+4.5;
        let y1 = source_point.getBoundingClientRect().top-oy+4.5;

        let x2 = target_point.getBoundingClientRect().left-ox+4.5;
        let y2 = target_point.getBoundingClientRect().top-oy+4.5;

        let cc = `<line x1='${x1}' y1='${y1}' x2='${x2}' y2='${y2}' class='gi connection_line' onclick='selector.selectConnection("${c.source}","${c.target}")'/> data-name='${c.source}$_{c.target}'`;
        main.connections += cc;
    },

    placeObjects(group, selectionList, path)
    {
        if(group == undefined)
            return;

        main.view.innerHTML = "";

        for(let i of group.inputs || [])
            main.addInput(i,path);

        for(let o of group.output || [])
                main.addOutput(o,path);

            for(let g of group.groups || [])
                main.addGroup(g,path);

        for(let m of group.modules || [])
            main.addModule(m,path);

        for(let w of group.widgets || [])
            main.addGroup(w,path, selectionList);

        main.connections = "<svg xmlns='http://www.w3.org/2000/svg'>";
        for(let c of group.connections || [])
            main.addConnection(c,path);
        main.connections += "</svg>";

        main.view.innerHTML += main.connections;

        // ADD CLICK AND SHIFT CLICK FUNCTIONS TO ALL OBJECTS

        for(let e of main.view.querySelectorAll(".group"))
            e.ondblclick = function(e) { selector.selectItems([], this.dataset.name); }; // Jump into group

        for(let e of main.view.querySelectorAll(".gi"))
        {
            e.onclick = function(e) { selector.selectItems([this.dataset.name], null, e.shiftKey); };
            if(selectionList.includes(e.dataset.name))
                e.classList.add("selected")
            else
                e.classList.remove("selected");
        }
    },

    toggleGrid()
    {
        let s = window.getComputedStyle(main.grid, null);
        if (s.display === 'none')
        main.grid.style.display = 'block';
        else 
        main.grid.style.display = 'none';
    },

    selectItem(foreground, background)
    {
        if(background != null)
        {
            group = network.dict[background];
            main.placeObjects(group, foreground, background);
        }
    }
}

brainstudio = {

    init()
    {
        log.init();
        inspector.init();
        controller.init();
        nav.init();
        breadcrumbs.init();
        main.init();
    }
}

