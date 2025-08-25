var xhttp = new XMLHttpRequest();
var amenu = "main";

window.onload = function() {
    menuSel("main");
};

function postMsg(cmd, val){
    xhttp.open("POST", "/comm", true);
    xhttp.setRequestHeader("Content-Type", "application/json");
    xhttp.send(JSON.stringify({"cmd":cmd, "val":val}));
}

function hideAll(){
    document.getElementById('tbMain').style.display = 'none';;
    document.getElementById('tbSensor').style.display = 'none';;
    document.getElementById('tbNetwork').style.display = 'none';;
    document.getElementById('tbMqtt').style.display = 'none';;
    document.getElementById('btnSave').style.display = 'none';;
}

function menuSel(menu){
    amenu = menu;
    hideAll();
    if(menu == "main"){
        document.getElementById('tbMain').style.display = 'table';
    } else {
        document.getElementById('btnSave').style.display = 'block';;
        if(menu == "sensor"){document.getElementById('tbSensor').style.display = 'table';}
        if(menu == "network"){document.getElementById('tbNetwork').style.display = 'table';}
        if(menu == "mqtt"){document.getElementById('tbMqtt').style.display = 'table';}
    }
    postMsg("sel", menu);
}

function saveSett(){
    xhttp.open("POST", "/comm", true);
    xhttp.setRequestHeader("Content-Type", "application/json");

    if(amenu == "mqtt"){
        let _server = document.getElementById("inMqttServer").value;
        let _user = document.getElementById("inMqttUser").value;
        let _pass = document.getElementById("inMqttPass").value;
        let _ptop = document.getElementById("inMqttPtop").value;
        let _stop = document.getElementById("inMqttStop").value;
        let _int = document.getElementById("inMqttInt").value;
        let _iint = parseInt(_int);
        xhttp.send(JSON.stringify({"cmd":"save", "menu":amenu, "server":_server, "user":_user, "pass":_pass, "ptop":_ptop, "stop":_stop, "interval":_iint}));
    }

    if(amenu == "sensor"){
        let _limit = document.getElementById("inLimit").value ;
        let _id = document.getElementById("inId").value;
        let _name = document.getElementById("inName").value;
        let _loc = document.getElementById("inLoc").value;
        let _flimit = parseFloat(_limit);
        let _iid = parseInt(_id);
        xhttp.send(JSON.stringify({"cmd":"save", "menu":amenu, "limit":_flimit, "id":_iid, "name":_name, "loc":_loc}));
    }

    if(amenu == "network"){
        let inip = document.getElementById("inIp").value;
        let ips = inip.split(".");
        let ip1 = parseInt(ips[0]);
        let ip2 = parseInt(ips[1]);
        let ip3 = parseInt(ips[2]);
        let ip4 = parseInt(ips[3]);

        let ingw = document.getElementById("inGateway").value;
        let gws = ingw.split(".");
        let gw1 = parseInt(gws[0]);
        let gw2 = parseInt(gws[1]);
        let gw3 = parseInt(gws[2]);
        let gw4 = parseInt(gws[3]);
        
        let insn = document.getElementById("inSubnet").value;
        let sns = insn.split(".");
        let sn1 = parseInt(sns[0]);
        let sn2 = parseInt(sns[1]);
        let sn3 = parseInt(sns[2]);
        let sn4 = parseInt(sns[3]);
       
        xhttp.send(JSON.stringify({"cmd":"save", "menu":amenu, "ip1":ip1,"ip2":ip2,"ip3":ip3,"ip4":ip4,"gw1":gw1,"gw2":gw2,"gw3":gw3,"gw4":gw4,"sn1":sn1,"sn2":sn2,"sn3":sn3,"sn4":sn4}));
    }
}


if (!!window.EventSource) {
    var source = new EventSource('/events');

    source.addEventListener('uimain', function(e) {
        let data = e.data;
        let sdata= data.split(",");
        document.getElementById("lblTemp").innerHTML = sdata[0];
        document.getElementById("lblAlm").innerHTML = sdata[1];
        document.getElementById("lblSensor").innerHTML = sdata[2];
        document.getElementById("lblUptime").innerHTML = sdata[3];
        document.getElementById("lblStorage").innerHTML = sdata[4];
        document.getElementById("lblUsed").innerHTML = sdata[5];
        document.getElementById("lblFree").innerHTML = sdata[6];
        document.getElementById("lblFirmware").innerHTML = sdata[7];
    }, false);

    source.addEventListener('uimqtt', function(e) {
        let data = e.data;
        let sdata= data.split(",");
        document.getElementById("inMqttServer").value = sdata[0];
        document.getElementById("inMqttUser").value = sdata[1];
        document.getElementById("inMqttPass").value = sdata[2];
        document.getElementById("inMqttPtop").value = sdata[3];
        document.getElementById("inMqttStop").value = sdata[4];
        document.getElementById("inMqttInt").value = sdata[5];
        document.getElementById("inMqttCert").value = sdata[6];
    }, false);

    source.addEventListener('uisensor', function(e) {
        let data = e.data;
        let sdata= data.split(",");
        document.getElementById("inLimit").value = sdata[0];
        document.getElementById("inId").value = sdata[1];
        document.getElementById("inName").value = sdata[2];
        document.getElementById("inLoc").value = sdata[3];
    }, false);

    source.addEventListener('uinetwork', function(e) {
        let data = e.data;
        let sdata= data.split(",");
        document.getElementById("inIp").value = sdata[0];
        document.getElementById("inGateway").value = sdata[1];
        document.getElementById("inSubnet").value = sdata[2];
    }, false);

}

async function uploadFile() {
    var file = document.getElementById("upfile").files[0];
    if(file == null){
        alert("please select valid ssl.crt file..!");
        return;
    }

    var filename = file["name"];
    if(filename !== "ssl.crt"){
        alert("please select valid ssl.crt file..!");
        return;
    }

    let formData = new FormData();   
    formData.append("file", file);

    const ctrl = new AbortController()    // timeout
    setTimeout(() => ctrl.abort(), 5000);
    
    try {
       let r = await fetch('/upload', 
         {method: "POST", body: formData, signal: ctrl.signal}); 
    } catch(e) {
       console.log('upload error ', e);
    }
    alert('The ssl certificate has been uploaded successfully.');
}