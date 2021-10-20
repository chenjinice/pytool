import logging , os , time , _thread , threading

from flask              import Flask,request,render_template
from flask_socketio     import SocketIO,emit,join_room,leave_room
from engineio.payload   import Payload

from obu.obu_abstract   import getTimeStr,checkIp
from web.map_downloader import MapDownloader


'''去掉恶心的log'''
logging.getLogger('werkzeug').setLevel(logging.ERROR)
'''默认16,导致engineio老是打印说太少'''
Payload.max_decode_packets  = 50


_kPort                      = 80
_kObuType                   = 'obusy'
_kAsnType                   = 'asn2020'

_cur_dir                    = os.path.dirname(__file__)
_vpn_cfg                    = os.path.join(_cur_dir, 'vpn.cfg')
_index_html                 = os.path.join(_cur_dir, 'index.html')
_html_dir                   = os.path.join(_cur_dir, '')
_app                        = Flask('chen_server', static_url_path='', static_folder=_html_dir, template_folder=_html_dir)
_socketio                   = SocketIO(_app, ping_interval=10,ping_timeout=60*60*24)
_room                       = {}
_lock                       = threading.Lock()
_obus                       = {}
_asns                       = {}



'''-----------api------------'''
def serverFun():
    # _app.run(port=_kPort)
    _socketio.run(_app, port=_kPort)

def startServer(obus={},asns={},bg=False):
    global _obus
    global _asns
    _obus   = obus
    _asns   = asns
    if bg :
        thread = _thread.start_new_thread(serverFun,())
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
                dev_list += 'ip='+ip+'&n='+name+'&asn='+_kAsnType
                dev_list += '">'+str+'</a></li>'
    if len(dev_list) > 0:
        dev_list ='<ol>' + dev_list +'</ol>'
    with open(_index_html, 'r', encoding='utf-8') as f:
        html_text = f.read().replace('${device_list}',dev_list)
    return html_text


@_app.route('/device', methods=['GET'])
def device():
    ip = request.args.get('ip')
    if checkIp(ip):
        return render_template('device.html')
    else:
        return 'wrong ip'


@_app.route('/asn_upload', methods=['POST'])
def asnUpload():
    result  =   {}
    i=0
    for f in request.files.values():
        arr         = f.name.split('#')
        asn_type    = f.name.split('#')[0]
        asn         = _asns.get(asn_type)
        if not asn:
            print('unsupport asn type -> '+asn_type)
            continue
        else:
            data    = f.stream.read()
            dict    = asn.parseAsn(data)
            result[f.filename] = dict
    return result




'''---------------------flask_socketio-----------------'''
def sioSendObuData(data={},ip=''):
    _socketio.emit('sio_msg',data,to=ip)

def sioSendMapUpdateSiginal():
    _socketio.emit('sio_map_init')

@_socketio.on('connect')
def connect():
    pass

@_socketio.on('disconnect')
def disconnect():
    _lock.acquire()
    sid = request.sid
    for ip in _room.keys():
        while sid in _room[ip]:
            _room[ip].remove(sid)
            leave_room(ip, sid)
            print(getTimeStr(), 'room[' + ip + '] : ' + sid + ' leave , len =', len(_room[ip]))
            obu     = _obus.get(_kObuType)
            if ( len(_room[ip]) == 0 ) and obu:
                obu.closeDevice(ip)
    _lock.release()


@_socketio.on('hello')
def getHello(ip):
    if not checkIp(ip):
        print('wrong ip')
        return
    obu = _obus.get(_kObuType)
    asn = _asns.get(_kAsnType)
    if not obu:
        print('obu type not supported : ' + _kObuType)
        return
    if not asn:
        print('asn type not supported : ' + _kAsnType)
        return
    obu.openDevice(ip, asn_parser=asn.parseAsn, html_sender=sioSendObuData)
    _lock.acquire()
    if not _room.get(ip):
        _room[ip] = []
    _room[ip].append(request.sid)
    join_room(ip)
    print(getTimeStr(),'room[' + ip + '] : ' + request.sid + ' join , len =', len(_room[ip]))
    _lock.release()


@_socketio.on('bounds')
def getBounds(lat1,lat2,lng1,lng2,zoom,map_type):
    # print(lat1,lat2,lng1,lng2,zoom,map_type)
    MapDownloader.signal_sender = sioSendMapUpdateSiginal
    MapDownloader.getBounds(lat1,lat2,lng1,lng2,zoom,map_type)

