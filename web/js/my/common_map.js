/*
    chen.jin
    使用leaflet的接口开发的一些地图通用函数
    供其他模块调用
*/


// 地图类型
var MapType = {
    RemoteGoogleSatellite : 0,
    RemoteGoogleMap       : 1,

    LocalGoogleSatellite  : 100,
    LocalGoogleMap        : 101,
}

// 全局变量
var GKD = {
    arrow1_icon         : L.icon({ iconUrl:'style/image/arrow.png',iconSize: [15, 15]}),
    arrow2_icon         : L.icon({ iconUrl:'style/image/arrow2.png',iconSize: [15, 15]}),
    car_icon            : L.icon({ iconUrl:'style/image/car.png',iconSize: [25, 48],iconAnchor:[12.5,48] }),
    remote_car_icon     : L.icon({ iconUrl:'style/image/other_car.png',iconSize: [25, 48],iconAnchor:[12.5,48] }),

    colors              : ["#FF0000","#00FF00","#FFFF00"],
    color_index         : 0,

    sockio              : null,
    map                 : null,
    type                : 0,
    center              : [41.9016655,123.5177551],
    // center           : [28.1128547,112.8668242],
    zoom                : 17,
    min_zoom            : 3,
    max_zoom            : 22,
    layers              : [],
    group               : null,
}


// 获取url的参数
function getUrlParam(key){
    var query = window.location.search.substring(1);
    var vars = query.split("&");
    for (var i=0;i<vars.length;i++) {
        var pair = vars[i].split("=");
        if(pair[0] == key){return pair[1];}
     }
     return(false);
}

// 获取地图网址
function getMapUrl(type){
    var url;
    switch(type){
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
            url = "http://{s}.google.com/vt/lyrs=s&x={x}&y={y}&z={z}";
    }
    return url;
}

// 初始化地图
function mapInit( type = -1 ){
    if (type != -1)         GKD.type = type;
    if (GKD.sockio == null) GKD.sockio = io();
    var url = getMapUrl(GKD.type);
    if(GKD.map == null){
        GKD.map = L.map('googleMap',{zoomAnimation:true});
        GKD.map.setView(GKD.center,GKD.zoom);
        if(GKD.sockio != null){
            GKD.map.on("move",function(event){
                var b       = GKD.map.getBounds();
                var zoom    = GKD.map.getZoom();
                GKD.sockio.emit("bounds",b.getSouth(),b.getNorth(),b.getWest(),b.getEast(),zoom,GKD.type);
            })
        }
    }
    if(GKD.map.hasLayer(GKD.street))GKD.map.removeLayer(GKD.street);
    GKD.street = L.tileLayer(url,{ minZoom: GKD.min_zoom,maxZoom: GKD.max_zoom,subdomains:['mt0','mt1','mt2','mt3']});
    GKD.street.addTo(GKD.map);
    if(GKD.group == null){
        GKD.group = L.markerClusterGroup();
        GKD.map.addLayer(GKD.group);
    }
}

// 缓存 layer，方便删除
function cacheLayer(layer){
    GKD.layers.push(layer); 
}


// 地图清除所有 layers
function clearAll(){
    for(var i=0;i<GKD.layers.length;i++){
        GKD.map.removeLayer(GKD.layers[i]);
    }
    GKD.group.clearLayers(); 
}

// 添加可移动的marker
function addMarker(lng,lat,pan=false){
    lat = lat.toFixed(7);
    lng = lng.toFixed(7);
    var marker = L.marker([lat,lng]).addTo(GKD.map);
    cacheLayer(marker);
    marker.dragging.enable();
    marker.bindPopup("lng : "+lng+"</br>lat : "+lat);
    marker.on('dragend',function(event){
        var pt = marker.getLatLng();
        var lat_new = pt.lat.toFixed(7);
        var lng_new = pt.lng.toFixed(7);
        marker.setPopupContent("lng : "+lng_new+"</br>lat : "+lat_new);
    });
    if(pan == true){
        GKD.map.panTo([lat,lng],{"animate":true,"duration":1});
    }
    return marker;
}

// 添加不能动的marker
function addFixedMarker(lng,lat,str="",pan=false) {
    var marker = L.marker([lat,lng]).addTo(GKD.map);
    cacheLayer(marker);
    if (str.length){
        marker.bindPopup(str);
    }else{
        marker.bindPopup("lng : "+lng+"</br>lat : "+lat);
    }
    if(pan == true){
        GKD.map.panTo([lat,lng],{"animate":true,"duration":1});
    }
    return marker;
}

// 添加线段末端的箭头
function addLineArrow(latlng,angle,arrow=GKD.arrow1_icon)
{
    var marker = L.marker([latlng[0],latlng[1]],{icon:arrow,rotationAngle:angle,rotationOrigin:'center',}).addTo(GKD.map);
    var tmp_angle = angle;
    if(tmp_angle < 0)tmp_angle += 360.0;
    tmp_angle = tmp_angle.toFixed(2);
    marker.bindPopup("lng : "+latlng[1]+"</br>lat : "+latlng[0] + "</br>angle :"+tmp_angle);
    cacheLayer(marker);
}

// 添加带箭头线段
function addLineWithArrow(latlngs,str="",line_color="#FF0000",arrow=GKD.arrow1_icon){
    var len = latlngs.length ,angle = 0;
    if(len == 0 )return;
    for(var i=0;i<len;i++){
        if(i<len-1) angle = Math.atan2(latlngs[i+1][1]-latlngs[i][1],latlngs[i+1][0]-latlngs[i][0])*180/Math.PI;
        addLineArrow(latlngs[i],angle,arrow);
    }
    if(len>1){
        var polyline = L.polyline(latlngs, {color:line_color,weight:5}).addTo(GKD.map);
        if(str.length > 0) polyline.bindPopup(str);
        cacheLayer(polyline);
    }
}

