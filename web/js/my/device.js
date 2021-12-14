/*
    chen.jin
    device界面的js
*/

// 本文件变量
var _kDevUiId       = "ui_id";
var _kDevChild      = "children";
var _kDevMs         = "ms";

var Sdev ={
    ip          :   "127.0.0.1",
    port        :   "30000",
    obu_type    :   "obusy",
    asn_type    :   "asn2020",
    name        :   "",   
    host_car    : 	null,
    timer       : 	null,
    heart_ms    :   0,
    
    map         : 	[],
    rsi         : 	[],
    rsm         : 	[],
    bsm         : 	[],
	cfg			: 	{
                        rsi_max_count   : 30,
                        div_cfg         : true,
                        div_tool        : true,
                        follow          : true,
						bsm			    : true,
						map 		    : true,
						rsi 		    : false,
						rsm 		    : false,
						spat 		    : true,
					}
}

// 初始化函数
function deviceInitAll(){
    var ip                  = getUrlParam('ip');
    var port                = getUrlParam('port');
    var obu_type            = getUrlParam('obu');
    var asn_type            = getUrlParam('asn');
    var name                = getUrlParam('n');
    if(ip       != false)   Sdev.ip         = ip;
    if(port     != false)   Sdev.port       = port;
    if(obu_type != false)   Sdev.obu_type   = obu_type;
    if(asn_type != false)   Sdev.asn_type   = asn_type;
    if(name     != false)   Sdev.name       = decodeURI(name);
    $("title").html(Sdev.ip+" "+Sdev.name);
    mapInit();
    GKD.sockio.on('connect',function(){
        GKD.sockio.emit('hello',Sdev.ip,Sdev.port,Sdev.obu_type,Sdev.asn_type);
    })
    GKD.sockio.on('sio_map_init',function(){
        mapInit();
    })
    GKD.sockio.on('sio_err',function(err){
        console.log(err);
    })
    GKD.sockio.on('sio_obu_msg',function(data){
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
    GKD.sockio.on('sio_pt',function(data){
        var lng     = data.lng;
        var lat     = data.lat;
        var name    = data.name;
        var heading = data.heading;
        var str     = "name : "+name+"<br />lng : "+lng+"<br />lat : "+lat +"<br />heading : "+heading;
        addFixedMarker(lng,lat,str);
    });
    // timer
    if(Sdev.timer == null){
        Sdev.timer  = self.setInterval("intervalFun()",300);
    }
}

// 定时跑的函数
function intervalFun(){
    var now = Date.now();
    clearOlds(Sdev.rsm,now,1000);
    clearOlds(Sdev.bsm,now,1500);
    clearOldSpat(2000);
    sioHeartBeat(2000);
}

// 发送保活心跳
function sioHeartBeat(ms) {
    if(GKD.sockio_ready == false ) return;
    var now             = Date.now();
    var t = now - Sdev.heart_ms;
    if(t > ms){
        Sdev.heart_ms   = now;
        GKD.sockio.emit('heart_beat');
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
    var str     = "lng : "+lng+"<br />lat : "+lat;
    if(Sdev.host_car == null){
        Sdev.host_car = L.marker(pt,{icon:GKD.car_icon,rotationAngle:data.heading}).addTo(GKD.map);
        Sdev.host_car.bindPopup(str);
    }else{
        Sdev.host_car.setLatLng(pt);
        Sdev.host_car.setRotationAngle(heading);
        Sdev.host_car.setPopupContent(str);
    }
    if(Sdev.cfg.follow){
        GKD.map.panTo(pt,{"animate":true,"duration":0.5});
    }
    $("#car_pos").html(lng+","+lat+","+elev);
	$("#car_heading").html(heading);
	$("#car_speed").html(speed);
	$("#car_num_st").html(num_st);
	$("#car_hdop").html(hdop);
	$("#car_model").html(model);
}

// -------------------------------
function devClearAll() {
    myClear();
    Sdev.bsm.length     = 0;
    Sdev.map.length     = 0;
    Sdev.rsi.length     = 0;
    Sdev.rsm.length     = 0;
}

function checkAsnExist(arr,ui_id) {
    var item = null;
    for(var i=0;i<arr.length;i++){
        var m = arr[i];
        if(m[_kDevUiId] == ui_id){
            item = m;
            break;
        }
    }
    return item;
}

function arrDeleteFirstOne(arr) {
    if(arr.length == 0)return;
    var item    = arr[0];
    var child   = item[_kDevChild];
    if(child){
        for(var i=0;i<child.length;i++){
            GKD.group.removeLayer(child[i]);
        }
    }
    GKD.group.removeLayer(item);
    arr.splice(0,1);
}

function clearOlds(array,now=Date.now(),ms = 1500) {
    array.forEach(function(item, index, arr) {
        var t = now - item[_kDevMs];
        if(t > ms){
            GKD.group.removeLayer(item);
            arr.splice(index, 1);
        }
    });
}

function setDevCfg(key,value){
	Sdev.cfg[key] = value;
    if(GKD.online){
        $.cookie(Sdev.ip + "-" + key,value.toString(),{expires:365});
    }
}

function getDevCfg(){
    if(GKD.online){
        for(var key in Sdev.cfg){
            var value = $.cookie(Sdev.ip + "-" + key);
            if (typeof(value) != "undefined"){
                Sdev.cfg[key]   = JSON.parse(value);
            }
        }
    }
	return Sdev.cfg;
}


//-------------------------
// 添加车道线末端的箭头
function addAsnLaneArrow(latlng,angle,arr,arrow=GKD.arrow1_icon)
{
    var marker = L.marker(latlng,{icon:arrow,rotationAngle:angle,rotationOrigin:'center',}).addTo(GKD.group);
    var ptc_old_angle = angle;
    if(ptc_old_angle < 0)ptc_old_angle += 360.0;
    ptc_old_angle = ptc_old_angle.toFixed(2);
    marker.bindPopup("lng : "+latlng[1]+"<br />lat : "+latlng[0] + "<br />angle :"+ptc_old_angle);
    arr.push(marker);
}

// 添加带箭头车道线
function addAsnLane(latlngs,str="",arr,line_color="#FF0000",arrow=GKD.arrow1_icon){
    var len = latlngs.length ,angle = 0;
    if(len == 0 )return;
    for(var i=0;i<len;i++){
        if(i<len-1) angle = Math.atan2(latlngs[i+1][1]-latlngs[i][1],latlngs[i+1][0]-latlngs[i][0])*180/Math.PI;
        addAsnLaneArrow(latlngs[i],angle,arr,arrow);
    }
    if(len>1){
        var polyline = L.polyline(latlngs, {color:line_color,weight:5}).addTo(GKD.group);
        if(str.length > 0) polyline.bindPopup(str);
        arr.push(polyline);
    }
}

// ----------- map --------------
function parseAsnMap(data) {
    for(var i=0;i<data.nodes.length;i++){
        var node = data.nodes[i];
        var lng = node.refPos.lng.toFixed(7);
        var lat = node.refPos.lat.toFixed(7);
        var node_id     = node.id.id;
        var node_region = node.id.region;
        var ui_id       = "map_"+node.id.id+"_"+node.id.region;
        if( checkAsnExist(Sdev.map,ui_id) )continue;
        var str         = "id : "+node_id+"<br />region : "+node_region+"<br />lng : "+lng+"<br />lat : "+lat;
        var map         = addFixedMarker(lng,lat,str);
        map[_kDevUiId]  = ui_id;
        map[_kDevChild] = [];
        parseAsnMapLanes(node.lanes,map[_kDevChild]);
        Sdev.map.push(map);                
    }
}

function parseAsnMapLanes(lanes,arr){
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
        var str  = "laneID : "+lane.laneID + "<br />laneWidth : " + lane.laneWidth + " 米<br />";
            str += getLaneManeuverStr(lane.maneuvers,lane.movements); 
        addAsnLane(latlngs,str,arr);
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
        str += "<br />左转相位 : " + movements.left;
        if(value[1])str += yes_str;
    }
    if(movements.hasOwnProperty("straight")){
        str += "<br />直行相位 : " + movements.straight;
        if(value[0])str += yes_str;
    }
    if(movements.hasOwnProperty("right")){
        str += "<br />右转相位 : " + movements.right;
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
        var rte     = rtes[i];
        var lng     = rte.pos.lng.toFixed(7);
        var lat     = rte.pos.lat.toFixed(7);
        var des     = rte.description;
        var type    = rte.eventType;
        var ui_id   = "rte"+type+"_"+lng+"_"+lat;
        if( checkAsnExist(Sdev.rsi,ui_id))continue;                
        var center      = [lat,lng];
        var str         = "eventType : "+type+"<br />description : "+des+"<br />lng : "+lng+"<br />lat : "+lat;
        var path_str    = "eventType : "+type+"<br />description : "+des;
        var rsi         = addFixedMarker(lng,lat,str);
        rsi[_kDevUiId]  = ui_id;
        rsi[_kDevChild] = [];
        parseAsnRefpaths(rte.referencePaths,path_str,rsi[_kDevChild]);
        Sdev.rsi.push(rsi);
        if(Sdev.rsi.length > Sdev.cfg.rsi_max_count)arrDeleteFirstOne(Sdev.rsi);
    }
}

function parseAsnRtss(rtss){
    for(var i=0;i<rtss.length;i++){
        var rts     = rtss[i];
        var lng     = rts.pos.lng.toFixed(7);
        var lat     = rts.pos.lat.toFixed(7);
        var des     = rts.description;
        var type    = rts.signType;
        var ui_id   = "rts"+type+"_"+lng+"_"+lat;
        if( checkAsnExist(Sdev.rsi,ui_id))continue;             
        var center      = [lat,lng];
        var str         = "signType : "+type+"<br />description : "+des+"<br />lng : "+lng+"<br />lat : "+lat;
        var path_str    = "signType : "+type+"<br />description : "+des;
        var rsi         = addFixedMarker(lng,lat,str);
        rsi[_kDevUiId]  = ui_id;
        rsi[_kDevChild] = [];
        parseAsnRefpaths(rts.referencePaths,path_str,rsi[_kDevChild]);
        Sdev.rsi.push(rsi);
        if(Sdev.rsi.length > Sdev.cfg.rsi_max_count)arrDeleteFirstOne(Sdev.rsi);
    }
}


function parseAsnRefpaths(paths,str,arr){
   for(var i=0;i<paths.length;i++){
        var points = paths[i].activePath;
        var latlngs = [];
        for(var k=0;k<points.length;k++){
            var p = points[k];
            var plng = p.lng.toFixed(7);
            var plat = p.lat.toFixed(7);
            latlngs.push([plat,plng]);
        }
        str  = "radius : "+paths[i].pathRadius + "<br />" + str;
        addAsnLane(latlngs,str,arr);
    }
}
// ----------- rsi end --------------


// ----------- rsm start ------------
function parseAsnRsm(data) {
    var ms              = Date.now();
    for(var i=0;i<data.participants.length;i++) {
        var ptc         = data.participants[i];
        var lng         = ptc.pos.lng.toFixed(7);
        var lat         = ptc.pos.lat.toFixed(7);
        var id          = ptc.ptcId;
        var type        = ptc.ptcType;
        var heading     = ptc.heading;             
        var ui_id       = "rsm_" + type + "_" + id;
        var str         = "ptcId : "+id+"<br />type : "+type+"<br />lng : "+lng+"<br />lat : "+lat + "<br />heading : " + heading;
        var ptc_old     = checkAsnExist(Sdev.rsm,ui_id);
        if(ptc_old){
            ptc_old.setLatLng([lat,lng]);
            ptc_old.setPopupContent(str);
            ptc_old[_kDevMs] = ms;
        }else{
            var ptc_new         = L.marker([lat,lng]).addTo(GKD.group);
            ptc_new[_kDevUiId]  = ui_id;
            ptc_new[_kDevMs]    = ms;
            ptc_new.bindPopup(str);
            Sdev.rsm.push(ptc_new);
        }
    }
}
// ----------- rsm end   ------------


// ----------- bsm start ------------
function parseAsnBsm(data) {
    var flag        = false;
    var ms          = Date.now();
    var car         = null;
    var id          = data.id;
    var idStr       = data.idString;
    var lng         = data.pos.lng.toFixed(7);
    var lat         = data.pos.lat.toFixed(7);
    var speed       = data.speed.toFixed(2);
    var heading     = data.heading.toFixed(2); 
    var events      = data.events;
    var pt          = [lat,lng];
    var ui_id       = "bsm_" + id;
    var str         = "id : " + id + "<br />idString : "+ idStr + "<br />lng : " + lng + "<br />lat : " + lat 
    str             += "<br />speed : " + speed + " km/h<br />heading : " + heading + "<br />events : " + events;
    var bsm_old     = checkAsnExist(Sdev.bsm,ui_id);
    if(bsm_old){
        bsm_old.setLatLng([lat,lng]);
        bsm_old.setRotationAngle(heading);
        bsm_old.setPopupContent(str);
        bsm_old[_kDevMs] = ms;
    }else{
        var bsm_new         = L.marker([lat,lng],{icon:GKD.remote_car_icon,rotationAngle:heading}).addTo(GKD.group);
        bsm_new[_kDevUiId]  = ui_id;
        bsm_new[_kDevMs]    = ms;
        bsm_new.bindPopup(str);
        Sdev.bsm.push(bsm_new);
    }
}
// ----------- bsm end   ------------



// ----------- spat start -----------
function parseAsnSpat(data) {
    var now                  = Date.now();
    var div_spat             = $("#top");
    for(var i=0;i<data.intersections.length;i++) {
        var section         = data.intersections[i];
        var id              = section.id.id;
        var region          = section.id.region;
        var ui_id           = "spat_" + id + "_" + region;
        var phases_str      = "<div class='div_spat_id'>"+id+"-"+region+"</div>";
		var phase_count  	= 0;
        for(var m=0;m<section.phases.length;m++){
            var phase       = section.phases[m];
            var phase_id    = phase.phaseId;
			if(phase_id == 0)continue;
			phase_count++;
            var color       = phase.color;
            var t           = phase.leftTime.toFixed(1);
            // var tmp         = "id:"+id+","+region+"\nphaseId:"+phase.phaseId+"----"+color +" ---- "+t;
            // console.log(tmp); 
            phases_str      += '<div class="div_phase '+color+'">'+phase_id+'<br />'+t+'</div>';
        }
        var div = $("#"+ui_id);
        if( (div.length > 0) && (phase_count > 0) ){
            div.html(phases_str);
            div.attr(_kDevMs,now);    
        }else{
            div_spat.append("<div id='"+ui_id+"' "+_kDevMs+"="+now+">"+phases_str+"</div>");
        }
    }
}

function clearOldSpat(ms = 2000){
    var now                  = Date.now();
    var spats                = $("#top").children("div");

    for(var i=0;i<spats.length;i++){
        spat        = $("#"+spats[i].id)
        spat_ms     = spat.attr(_kDevMs);
        var t       = now - spat_ms;
        if(t > ms){
            spat.remove();
        }
    }
}
// ----------- spat end   -----------

