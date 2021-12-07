/*
    chen.jin
    使用leaflet的接口开发的一些地图通用函数
    供其他模块调用
*/


// 地图类型
var MapType = {
    RemoteGoogleSatellite : 1,
    RemoteGoogleMap       : 2,

    LocalGoogleSatellite  : 101,
    LocalGoogleMap        : 102,
}

// 全局变量
var GKD = {
    arrow1_icon         : L.icon({ iconUrl:'style/image/arrow.png',iconSize: [15, 15]}),
    arrow2_icon         : L.icon({ iconUrl:'style/image/arrow2.png',iconSize: [15, 15]}),
    car_icon            : L.icon({ iconUrl:'style/image/car.png',iconSize: [25, 48],iconAnchor:[12.5,48] }),
    remote_car_icon     : L.icon({ iconUrl:'style/image/other_car.png',iconSize: [25, 48],iconAnchor:[12.5,48] }),

    colors              : ["#FF0000","#00FF00","#FFFF00"],
    color_index         : 0,

    online              : ifOnLine(),
    sockio              : null,
    sockio_ready        : false,

    map                 : null,
    // type                : MapType.RemoteGoogleSatellite,
    type                : MapType.LocalGoogleSatellite,
    center              : [28.1128547,112.8668242],
    zoom                : 17,
    min_zoom            : 3,
    max_zoom            : 22,
    group               : L.layerGroup(),
    cluser              : L.markerClusterGroup(),
}


// 是不是远程网页
function ifOnLine() {
    var str = window.location + "";
    if( str.indexOf("http") == 0 ){
        return true;
    }else{
        return false;
    }
}

// 获取url的参数
function getUrlParam(key){
    var query = window.location.search.substring(1);
    var vars = query.split("&");
    for (var i=0;i<vars.length;i++) {
        var pair = vars[i].split("=");
        if(pair[0] == key){return pair[1];}
    }
    console.log("getUrlParam "+key+" fail");
    return false;
}

// 获取地图网址
function getMapUrl(type){
    switch(type){
        case MapType.RemoteGoogleSatellite:
        case MapType.RemoteGoogleMap:
        case MapType.LocalGoogleSatellite:
        case MapType.LocalGoogleMap:
            GKD.type = type;
            break;
    }
    var url;
    switch(GKD.type){
        case MapType.RemoteGoogleSatellite:
            url = "http://{s}.google.com/vt/lyrs=s&x={x}&y={y}&z={z}";
            break;
        case MapType.RemoteGoogleMap:
            url = "http://{s}.google.com/vt/lyrs=m&x={x}&y={y}&z={z}";
            break;
        case MapType.LocalGoogleSatellite:
            url = "map/google_satellite/{z}/{x}/{y}.jpg";
            break;
        case MapType.LocalGoogleMap:
            url = "map/google_map/{z}/{x}/{y}.jpg";
            break;
        default:
            GKD.type = RemoteGoogleSatellite;
            url      = "http://{s}.google.com/vt/lyrs=s&x={x}&y={y}&z={z}";
    }
    return url;
}

// 初始化地图
function mapInit( type = 0 ){
    var url = getMapUrl(type);
    if (GKD.sockio == null){
        GKD.sockio = io();
        GKD.sockio.on('connect', function(){
            GKD.sockio_ready    = true;     console.log("sio connect");
        });
        GKD.sockio.on('disconnect', function(){
            GKD.sockio_ready    = false;    console.log("sio disconnect");
        });
    }
    if(GKD.map == null){
        GKD.map = L.map('googleMap',{zoomAnimation:true});
        GKD.map.setView(GKD.center,GKD.zoom);
        GKD.map.addLayer(GKD.group);
        GKD.map.on("move",function(event){
            var b       = GKD.map.getBounds();
            var zoom    = GKD.map.getZoom();
            GKD.sockio.emit("bounds",b.getSouth(),b.getNorth(),b.getWest(),b.getEast(),zoom,GKD.type);
        })
    }
    if(GKD.map.hasLayer(GKD.street))GKD.map.removeLayer(GKD.street);
    GKD.street = L.tileLayer(url,{ minZoom: GKD.min_zoom,maxZoom: GKD.max_zoom,subdomains:['mt0','mt1','mt2','mt3']});
    GKD.map.addLayer(GKD.street);
    GKD.map.addLayer(GKD.cluser);
}

// 地图清除所有 layers
function myClear(){
    GKD.group.clearLayers();
    GKD.cluser.clearLayers(); 
}

// 添加可移动的marker
function addMarker(lng,lat,pan=0){
    var marker = L.marker([lat,lng]).addTo(GKD.group);
    marker.dragging.enable();
    marker.bindPopup("lng : "+lng+"<br />lat : "+lat);
    marker.on('dragend',function(event){
        var pt = marker.getLatLng();
        var lat_new = pt.lat.toFixed(7);
        var lng_new = pt.lng.toFixed(7);
        marker.setPopupContent("lng : "+lng_new+"<br />lat : "+lat_new);
    });
    if(pan){
        GKD.map.panTo([lat,lng],{"animate":true,"duration":pan});
    }
    return marker;
}

// 添加不能动的marker
function addFixedMarker(lng,lat,str="",pan=0) {
    var marker = L.marker([lat,lng]).addTo(GKD.group);
    if (str){
        marker.bindPopup(str);
    }else{
        marker.bindPopup("lng : "+lng+"<br />lat : "+lat);
    }
    if(pan){
        GKD.map.panTo([lat,lng],{"animate":true,"duration":pan});
    }
    return marker;
}

// 在地图中心添加marker
function addMarkerAtCenter(){
    var center = GKD.map.getCenter();
    addMarker(center.lng.toFixed(7),center.lat.toFixed(7));
}


