import logging , os , time , _thread , threading

from flask              import Flask,request,render_template
from flask_socketio     import SocketIO,emit,join_room,leave_room
from engineio.payload   import Payload

from obu.obu            import *
from asn.asn            import *
from web.map_downloader import MapDownloader



'''去掉恶心的log'''
logging.getLogger('werkzeug').setLevel(logging.ERROR)
'''默认16,导致engineio老是打印说太少'''
Payload.max_decode_packets  = 50


_kClientTimeout             = 10.0
_kPort                      = 80
_kObuType                   = 'obusy'
_kAsnType                   = 'asn2020'

_cur_dir                    = os.path.dirname(__file__)
_vpn_cfg                    = os.path.join(_cur_dir, 'vpn.cfg')
_index_html                 = os.path.join(_cur_dir, 'index.html')
_html_dir                   = os.path.join(_cur_dir, '')
_app                        = Flask('chen_server', static_url_path='', static_folder=_html_dir, template_folder=_html_dir)
_socketio                   = SocketIO(_app, ping_interval=10,ping_timeout=60*60*24)
_rooms                       = {}
_lock                       = threading.Lock()
_obus                       = obuAll
_asns                       = asnAll



'''-----------api------------'''
def serverFun():
    # _app.run(port=_kPort)
    _socketio.run(_app, port=_kPort)

def startServer(bg=False):
    if bg :
        _thread.start_new_thread(serverFun,())
    else:
        serverFun()



'''-----------------------flask---------------------'''
@_app.route('/')
def index():
    html_text=""
    dev_list=""
    with open(_vpn_cfg, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            arr=line.strip().replace(' ','').split(',')
            str=""
            ip      = arr[0]
            name    = ''
            if len(ip) > 0 :
                str+=ip
            if len(arr) > 1  and  len(arr[1]) > 0:
                name    = arr[1]
                str+='--'+arr[1]
            if len(str) > 0:
                dev_list += '<li><a target="_blank" href="device?'
                dev_list += 'ip='+ip+'&port='+'30000'+'&obu='+_kObuType+'&asn='+_kAsnType+'&n='+name
                dev_list += '">'+str+'</a></li>'
    if len(dev_list) > 0:
        dev_list ='<ol>' + dev_list +'</ol>'
    with open(_index_html, 'r', encoding='utf-8') as f:
        html_text = f.read().replace('${device_list}',dev_list)
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




'''---------------------flask_socketio-----------------'''

def sioSendObuData(data={},ip=''):
    _socketio.emit('sio_obu_msg',data,to=ip)

def sioSendMapUpdateSiginal():
    _socketio.emit('sio_map_init')

def sioSendErr(err='',sid=''):
    _socketio.emit('sio_err',err,to=sid)



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
        sioSendErr(err, sid)
        return
    port_int    = int(port)
    obu_class   = _obus.get(obu_type)
    asn_class   = _asns.get(asn_type)
    obu         = obu_class.openDevice(ip,port_int,asn_parser=asn_class.parseAsn, html_sender=sioSendObuData,sid=sid)
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
    MapDownloader.signal_sender = sioSendMapUpdateSiginal
    MapDownloader.getBounds(lat1,lat2,lng1,lng2,zoom,map_type)

