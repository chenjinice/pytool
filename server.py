import logging , os , time , _thread , threading

from flask                  import Flask,request,render_template
from flask_socketio         import SocketIO,emit,join_room,leave_room
from engineio.payload       import Payload
from engineio.async_drivers import gevent

from obu.obu                import *
from asn.asn                import *
from config                 import *
from map_downloader         import MapDownloader


class HtmlSender():
    def __init__(self):
        self.asn_send = None
        self.err_send = None
        self.pt_send  = None


'''去掉恶心的log'''
logging.getLogger('werkzeug').setLevel(logging.ERROR)
'''默认16,导致engineio老是打印说太少'''
Payload.max_decode_packets  = 50


_kClientTimeout             = 10.0


# _cur_dir                    = os.path.dirname(__file__)
_cur_dir                    = os.path.join(os.path.abspath('.'),'web')
_index_html                 = os.path.join(_cur_dir, 'index.html')
_html_dir                   = os.path.join(_cur_dir, '')
_app                        = Flask('chen_server', static_url_path='', static_folder=_html_dir, template_folder=_html_dir)
_socketio                   = SocketIO(_app,ping_interval=10,ping_timeout=60*60*24)
_rooms                      = {}
_lock                       = threading.Lock()
_obus                       = obuAll
_asns                       = asnAll

_html_sender                = HtmlSender()



'''-----------api------------'''
def serverFun():
    # _app.run(port=_kPort)
    host_ip                     = CfgData[kCfgHostIp]
    host_port                   = CfgData[kCfgHostPort]
    MapDownloader.signal_sender = sioSendMapUpdateSiginal
    MapDownloader.map_dir       = _cur_dir
    _socketio.run(_app,host=host_ip,port=host_port)

def startServer(bg=False):
    if bg :
        _thread.start_new_thread(serverFun,())
    else:
        serverFun()



'''-----------------------flask---------------------'''
@_app.route('/')
def index():
    dev_list = ""
    for obu_info in CfgData[kCfgObuInfo]:
        obu_type = obu_info[kCfgObuType]
        obu_port = obu_info[kCfgObuPort]
        asn_type = obu_info[kCfgAsnType]
        for obu in obu_info[kCfgObuArray]:
            tmp_str = ""
            ip      = obu[0]
            name    = obu[1]
            if len(ip) > 0 :
                tmp_str+=ip
            if len(name) > 0:
                tmp_str+='———'+name
            if len(tmp_str) > 0:
                dev_list += '<li><a target="_blank" href="device?'
                dev_list += 'ip='+ip+'&port='+str(obu_port)+'&obu='+obu_type+'&asn='+asn_type+'&n='+name
                dev_list += '">'+tmp_str+'</a></li>'
    if len(dev_list) > 0:
        dev_list ='<ol>' + dev_list +'</ol>'
    with open(_index_html, 'r', encoding='utf-8') as f:
        html_text = f.read().replace('${device_list}', dev_list)
    return html_text


@_app.route('/device', methods=['GET'])
def device():
    return render_template('device.html')


@_app.route('/asn_upload', methods=['POST'])
def asnUpload():
    result  =   {}
    i=0
    for f in request.files.values():
        arr         = f.name.split('#')
        asn_type    = f.name.split('#')[0]
        asn         = _asns.get(asn_type)
        if not asn:
            print('/asn_upload : unsupport asn type -> '+asn_type)
            continue
        else:
            data    = f.stream.read()
            dict    = asn.parseAsn(data)
            result[f.filename] = dict
    return result


@_app.route('/pt_upload', methods=['POST'])
def ptUpload():
    result          = {}
    result[oPts]    = []
    i=0
    for f in request.files.values():
        while True:
            line = f.stream.readline().decode(encoding='utf-8').strip()
            if not line:
                break
            tmp = line.split(']:')
            if len(tmp) < 2:
                continue
            arr = tmp[1].split(',')
            if len(arr) < 4:
                continue
            pt              = {}
            pt[oLng]        = arr[0]
            pt[oLat]        = arr[1]
            pt[oHeading]    = arr[2]
            pt[oName]       = arr[3]
            result[oPts].append(pt)
    return result



'''---------------------flask_socketio-----------------'''
def sioSendObuData(data={},ip=''):
    _socketio.emit('sio_obu_msg',data,to=ip)

def sioSendMapUpdateSiginal():
    _socketio.emit('sio_map_init')

def sioSendErr(err='',sid=''):
    _socketio.emit('sio_err',err,to=sid)

def sioSendPt(data={},sid=''):
    _socketio.emit('sio_pt',data,to=sid)


_html_sender.asn_send    = sioSendObuData
_html_sender.err_send    = sioSendErr
_html_sender.pt_send     = sioSendPt


@_socketio.on('connect')
def connect():
    pass

@_socketio.on('disconnect')
def disconnect():
    pass

@_socketio.on('hello')
def hello(ip,port,obu_type,asn_type):
    err = []
    sid = request.sid
    if not checkIp(ip):
        err.append('ip格式错误:' + str(ip))
    if not checkPort(port):
        err.append('端口格式错误:' + str(port))
    if not _obus.get(obu_type):
        err.append('不支持obu类型:' + str(obu_type))
    if not _asns.get(asn_type):
        err.append('不支持asn类型:' + str(asn_type))
    if len(err) > 0:
        print(err)
        sioSendErr(err, sid)
        return
    port_int    = int(port)
    obu_class   = _obus.get(obu_type)
    asn_class   = _asns.get(asn_type)
    obu         = obu_class.openDevice(ip,port_int,asn_parser=asn_class.parseAsn, html_sender=_html_sender,sid=sid)
    join_room(obu.room_id)

@_socketio.on('heart_beat')
def heartBeat():
    sid = request.sid
    now = time.time()
    for k in obuAll:
        obuAll[k].updateAllClient(sid,now)

@_socketio.on('bounds')
def bounds(lat1,lat2,lng1,lng2,zoom,map_type):
    # print(lat1,lat2,lng1,lng2,zoom,map_type)
    MapDownloader.getBounds(lat1,lat2,lng1,lng2,zoom,map_type)

@_socketio.on('save_current_pt')
def saveCurrentPt(name):
    sid = request.sid
    for k in obuAll:
        obuAll[k].saveCurrentPt(sid, name)

