




// --------------------------采集的路径相关-------------------------------------
// 过滤条件,按距离取点
var _sJsonPt = L.latLng(0,0);
function pathPointFilter(latlng) {
	var dist 	  	= 2.0;		// 按两点之间距离决定，单位米
	var tmp 		= L.latLng(latlng[0],latlng[1]);
	if (tmp.distanceTo(_sJsonPt) > dist){
		_sJsonPt = tmp;
		return true;
	}else{
		return false;
	}
}

// 画线
function addPathJsonLine(latlngs,color) {
	var len = latlngs.length , angle = 0;
	var line_color = color;
    if(len>1){
        var polyline = L.polyline(latlngs, {color:line_color,weight:5}).addTo(GKD.group);
    }
    //画点
    for(var i=0;i<len;i++){
    	var latlng = latlngs[i];
    	if( !pathPointFilter(latlng) )continue;
    	var marker = L.marker(latlng,{icon:GKD.arrow1_icon,rotationAngle:latlng[2],rotationOrigin:'center',}).addTo(GKD.cluser);
		// marker.bindPopup("{\"lng\":"+latlng[1]*1e7+",\"lat\":"+latlng[0]*1e7+"}");
		marker.bindPopup("lng : "+latlng[1]+"</br>lat : "+latlng[0]+"</br>angle : "+latlng[2]);
    }
}

// 采集的路径json解析
function parsePathJson(filename) {
	$.getJSON(filename,function(data){
		var points = data.points;
		var len = points.length , k;
		var index = 0;
		var latlngs = [];
		var colors 	= ["#FF0000","#00FF00","#FFFF00"];
		for(k=0;k<len;k++){
			var p = points[k];
			if( (p.length < 3) || (k == len-1) ){
				addPathJsonLine(latlngs,colors[index++%colors.length]);
				latlngs = [];
			}else{
				var lng = p[0].toFixed(7);
				var lat = p[1].toFixed(7);
				var angle = p[2].toFixed(2);
				latlngs.push([lat,lng,angle]);
			}
		}
    }).error(function() {alert("解析文件失败 : "+filename);});
}


