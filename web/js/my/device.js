/*
    chen.jin
    device界面的js
*/

// 本文件变量
var Sdev ={
    host_car    : 	null,
    timer       : 	null,
    pan         : 	true,
    map         : 	[],
    rsi         : 	[],
    rsm         : 	[],
    bsm         : 	[],
	cfg			: 	{
						bsm			: true,
						map 		: true,
						rsi 		: true,
						rsm 		: true,
						spat 		: true,
					}
}




// 初始化函数
function deviceInitAll(){
    mapInit(100);
    var ip      = getUrlParam('ip');
	var name 	= decodeURI(getUrlParam('n'));
    if (ip == false) { return; }
	$("title").html(ip+" "+name);
    GKD.sockio.on('connect', function(){
        GKD.sockio.emit('hello',ip);
    });
    GKD.sockio.on('sio_map_init',function(){
        mapInit();
    })
    GKD.sockio.on('sio_msg',function(data){
        if(data.type == "host_pt"){
            updateHostCar(data);
        }else if(data.type == "bsmFrame"){
			if(Sdev.cfg.bsm)parseAsnBsm(data);
        }else if(data.type == "rsmFrame"){
            if(Sdev.cfg.rsm)parseAsnRsm(data);
        }else if(data.type == "rsiFrame"){
            if(Sdev.cfg.rsi)parseAsnRsi(data);
        }else if(data.type == "mapFrame"){
            if(Sdev.cfg.map)parseAsnMap(data);
        }else if(data.type == "spatFrame"){
            if(Sdev.cfg.spat)parseAsnSpat(data);
        }
    });
    // timer
    if(Sdev.timer == null){
        Sdev.timer  = self.setInterval("intervalFun()",300);
    }
}

// 更新本车的位置
function updateHostCar(data){
    var lat     = data.lat.toFixed(7);
    var lng     = data.lng.toFixed(7);
    var elev    = data.elev.toFixed(2);
    var speed   = data.speed.toFixed(2);
    var heading = data.heading.toFixed(2);
    var num_st  = data.num_st;
    var hdop    = data.hdop.toFixed(1);
    var model   = data.model;
    var pt      = [lat,lng];
    var str     = "lng : "+lng+"</br>lat : "+lat+"</br>heading : "+heading + "</br>speed : " + speed + " km/h";
    if(Sdev.host_car == null){
        Sdev.host_car = L.marker(pt,{icon:GKD.car_icon,rotationAngle:data.heading}).addTo(GKD.map);
        Sdev.host_car.bindPopup(str);
    }else{
        Sdev.host_car.setLatLng(pt);
        Sdev.host_car.setRotationAngle(heading);
        Sdev.host_car.setPopupContent(str);
    }
    if(Sdev.pan){
        GKD.map.panTo(pt,{"animate":true,"duration":0.5});
    }
    $("#ui_pos").html(lng+","+lat+","+elev);
	$("#ui_heading").html(heading);
	$("#ui_speed").html(speed);
	$("#ui_num_st").html(num_st);
	$("#ui_hdop").html(hdop);
	$("#ui_model").html(model);
}

// 定时跑的函数
function intervalFun(){
    clearOldPtc(1000);
    clearOldBsm(1500);
}

function setDevCfg(key,value){
	Sdev.cfg[key] = value;
}

function getDevCfg(){
	return Sdev.cfg;
}


// ----------- map --------------
function parseAsnMap(data) {
    for(var i=0;i<data.nodes.length;i++){
        var node = data.nodes[i];
        var lng = node.refPos.lng.toFixed(7);
        var lat = node.refPos.lat.toFixed(7);
        var node_id     = node.id.id;
        var node_region = node.id.region;
        if( checkMapNodeExist(node_id,node_region) )continue;                
        var str = "id : "+node_id+"</br>region : "+node_region+"</br>lng : "+lng+"</br>lat : "+lat;
        addFixedMarker(lng,lat,str);
        parseAsnMapLanes(node.lanes);
    }
}

function checkMapNodeExist(id,region) {
    var flag = false;
    for(var i=0;i<Sdev.map.length;i++){
        var m = Sdev.map[i];
        if(m.id != id)continue;
        if(m.region != region)continue;
        flag = true;
    }
    if(!flag){
        var tmp     = {};
        tmp.id      = id;
        tmp.region  = region;
        Sdev.map.push(tmp);
    }
    return flag;
}

function parseAsnMapLanes(lanes){
    for(var i=0;i<lanes.length;i++){
        var lane = lanes[i];
        var points = lane.points;
        var latlngs = [];
        for(var k=0;k<points.length;k++){
            var p = points[k];
            var plng = p.lng.toFixed(7);
            var plat = p.lat.toFixed(7);
            latlngs.push([plat,plng]);
        }
        var str  = "laneID : "+lane.laneID + "</br>laneWidth : " + lane.laneWidth + " 米</br>";
            str += getLaneManeuverStr(lane.maneuvers,lane.movements); 
        addLineWithArrow(latlngs,str);
    }
}

function getLaneManeuverStr(maneuvers,movements) {
    var yes_str     = "<font color=\"#00ec00\"> ✔</font>"
    var str         = "maneuvers : "+ maneuvers + " ( ";
    var dirt        = ["直行","左转","右转","U型转"];
    var value       = [0,0,0,0];
    for(var i=0;i<4;i++){
        if ( (maneuvers >> i) & 0x01 ) {
            value[i]    =   1;
            str         +=  dirt[i] + " ";
        }
    }
    str += "  )"
    if(movements.hasOwnProperty("left")){
        str += "</br>左转灯 : " + movements.left;
        if(value[1])str += yes_str;
    }
    if(movements.hasOwnProperty("straight")){
        str += "</br>直行灯 : " + movements.straight;
        if(value[0])str += yes_str;
    }
    if(movements.hasOwnProperty("right")){
        str += "</br>右转灯 : " + movements.right;
        if(value[2])str += yes_str;
    }
    return str;
}
// ----------- map end --------------


// ----------- rsi start ------------
function parseAsnRsi(data) {
    parseAsnRtes(data.rtes);
    parseAsnRtss(data.rtss);
}

function parseAsnRtes(rtes){
    for(var i=0;i<rtes.length;i++){
        var rte = rtes[i];
        var lng = rte.pos.lng.toFixed(7);
        var lat = rte.pos.lat.toFixed(7);
        var des = rte.description;
        var type = rte.eventType;
        if( checkRsiExist("rte",type,lng,lat) )continue;                
        var center = [lat,lng];
        var str      = "eventType : "+type+"</br>description : "+des+"</br>lng : "+lng+"</br>lat : "+lat;
        var path_str = "eventType : "+type+"</br>description : "+des;
        addFixedMarker(lng,lat,str);
        parseAsnRefpaths(rte.referencePaths,path_str);
    }
}

function parseAsnRtss(rtss){
    for(var i=0;i<rtss.length;i++){
        var rts = rtss[i];
        var lng = rts.pos.lng.toFixed(7);
        var lat = rts.pos.lat.toFixed(7);
        var des = rts.description;
        var type = rts.signType;
        if( checkRsiExist("rts",type,lng,lat) )continue;                
        var center = [lat,lng];
        var str      = "signType : "+type+"</br>description : "+des+"</br>lng : "+lng+"</br>lat : "+lat;
        var path_str = "signType : "+type+"</br>description : "+des;
        addFixedMarker(lng,lat,str);
        parseAsnRefpaths(rts.referencePaths,path_str);
    }
}

function checkRsiExist(rsi_type,alert_type,lng,lat){
    var flag = false;
    for(var i=0;i<Sdev.rsi.length;i++){
        var rsi = Sdev.rsi[i];
        if(rsi.rsi_type != rsi_type)continue;
        if(rsi.alert_type != alert_type)continue;
        if(rsi.lng != lng)continue;
        if(rsi.lat != lat)continue;
        flag = true;
    }
    if(!flag){
        var tmp = {};
        tmp.rsi_type    = rsi_type;
        tmp.alert_type  = alert_type;
        tmp.lng         = lng;
        tmp.lat         = lat;
        Sdev.rsi.push(tmp);
        if(Sdev.rsi.length > 50){
            var tmp = Sdev.rsi.pop();
            GKD.map.removeLayer(tmp);
        }
    }
    return flag;
}

function parseAsnRefpaths(paths,str){
   for(var i=0;i<paths.length;i++){
        var points = paths[i].activePath;
        var latlngs = [];
        for(var k=0;k<points.length;k++){
            var p = points[k];
            var plng = p.lng.toFixed(7);
            var plat = p.lat.toFixed(7);
            latlngs.push([plat,plng]);
        }
        str  = "radius : "+paths[i].pathRadius + "</br>" + str;
        addLineWithArrow(latlngs,str);
    }
}
// ----------- rsi end --------------


// ----------- rsm start ------------
function parseAsnRsm(data) {
    if(Sdev.timer == null){
        Sdev.timer  = self.setInterval("clearOldPtc()",1000);
    }
    for(var i=0;i<data.participants.length;i++) {
        var ptc     = data.participants[i];
        var lng     = ptc.pos.lng.toFixed(7);
        var lat     = ptc.pos.lat.toFixed(7);
        var id      = ptc.ptcId;
        var type    = ptc.ptcType;
        var heading = ptc.heading;             
        var str     = "ptcId : "+id+"</br>type : "+type+"</br>lng : "+lng+"</br>lat : "+lat + "</br>heading : " + heading;
        updatePtc(id,lat,lng,str);
    }
}

function updatePtc(ptc_id,lng,lat,str) {
    var flag = false;
    var ptc  = null;
    var ms   = Date.now();
    for(var i=0;i<Sdev.rsm.length;i++){
        var rsm       = Sdev.rsm[i];
        if(rsm.ptcId != ptc_id)continue;
        ptc  = rsm;
        flag = true;
    }
    if(flag){
        ptc.setLatLng([lat,lng]);
        ptc.setPopupContent(str);
        ptc.ms = ms;
    }else{
        var ptc_new   = L.marker([lat,lng]).addTo(GKD.map);
        ptc_new.ptcId = ptc_id;
        ptc_new.ms    = ms;
        ptc_new.bindPopup(str);
        cacheLayer(ptc_new);
        Sdev.rsm.push(ptc_new);
    }
}

function clearOldPtc(ms = 1000) {
    var now   = Date.now();
    Sdev.rsm.forEach(function(item, index, arr) {
        var t = now - item.ms;
        if(t > ms){
            GKD.map.removeLayer(item);
            arr.splice(index, 1);
        }
    });
}
// ----------- rsm end   ------------


// ----------- bsm start ------------
function parseAsnBsm(data) {
    var flag    = false;
    var ms      = Date.now();
    var car     = null;
    var id      = data.id;
    var idStr   = data.idString;
    var lng     = data.pos.lng.toFixed(7);
    var lat     = data.pos.lat.toFixed(7);
    var speed   = data.speed.toFixed(2);
    var heading = data.heading.toFixed(2); 
    var events  = data.events;
    var pt      = [lat,lng];
    var str     = "id : " + id + "</br>id : "+ idStr + "</br>lng : " + lng + "</br>lat : " + lat 
    str        += "</br>speed : " + speed + " km/h</br>heading : " + heading + "</br>events : " + events;

    for(var i=0;i<Sdev.bsm.length;i++){
        var bsm       = Sdev.bsm[i];
        if(bsm.id != id)continue;
        car  = bsm;
        flag = true;
    }
    if(flag){
        bsm.setLatLng([lat,lng]);
        bsm.setRotationAngle(heading);
        bsm.setPopupContent(str);
        bsm.ms = ms;
    }else{
        var car_new   = L.marker([lat,lng],{icon:GKD.remote_car_icon,rotationAngle:heading}).addTo(GKD.map);
        car_new.id    = id;
        car_new.ms    = ms;
        car_new.bindPopup(str);
        cacheLayer(car_new);
        Sdev.bsm.push(car_new);
    }
}

function clearOldBsm(ms = 1500) {
    var now   = Date.now();
    Sdev.bsm.forEach(function(item, index, arr) {
        var t = now - item.ms;
        if(t > ms){
            GKD.map.removeLayer(item);
            arr.splice(index, 1);
        }
    });
}
// ----------- bsm end   ------------


// ----------- spat start -----------
function parseAsnSpat(data) {
    for(var i=0;i<data.intersections.length;i++) {
        var section = data.intersections[i];
        for(var m=0;m<section.phases.length;m++){
            var phase = section.phases[m];
            var str   = "id:"+section.id.id+","+section.id.region+"\nphaseId:"+phase.phaseId+"----"+phase.color +" ---- "+phase.leftTime.toFixed(1);
            // console.log(str); 
        }
    }
}
// ----------- spat end   -----------

