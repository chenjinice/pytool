
''' obu通用数据 '''
import os,re,abc,time,_thread,threading
from config                 import *
from asn.asn_abstract       import *


_log_dir                = os.path.join(os.path.abspath('.'),'log')


'''------支持的所有obu-------'''
obuAll      = {}


'''----'''
_kClearInterval         = 5
_kClientTimeout         = 8.0


'''----------key----------'''
oPositionType           = 'host_pt'
oType                   = 'type'
oIp                     = 'ip'
oLng                    = 'lng'
oLat                    = 'lat'
oElev                   = 'elev'
oSpeed                  = 'speed'
oHeading                = 'heading'
oNumSt                  = 'num_st'
oHdop                   = 'hdop'
oModel                  = 'model'


oPts                    = 'pts'
oName                   = 'name'


'''-----'''

def getTimeStr():
    str = time.strftime('[%Y-%m-%d %H:%M:%S]', time.localtime())
    return str

def checkIp(ip):
    if not ip:
        return False
    compile = re.compile('^(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|[1-9])\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)$')
    if compile.match(ip):
        return True
    else:
        return False

def checkPort(port):
    if not port:
        return False
    tmp_port = 0
    if   isinstance(port,int):
        tmp_port = port
    elif isinstance(port,str):
        tmp_port = int(port)
    else:
        return False
    if( tmp_port <= 0 ) or ( tmp_port > 65535):
        return False
    else:
        return True




class ObuAbstract(metaclass=abc.ABCMeta):
    ''' obu 抽象类 '''
    s_clear_thread          = None
    s_clear_thread_lock     = threading.Lock()
    s_cache                 = {}
    s_cache_lock            = threading.Lock()
    s_pt_file               = _log_dir + '/pt.dat'


    def __new__(cls, *args, **kwargs):
        return object.__new__(cls)


    def __init__(self, ip,port=9527,asn_parser=None, html_sender=None):
        self.html_sender    = html_sender
        self.lng            = 0
        self.lat            = 0
        self.elev           = 0
        self.heading        = 0
        self.speed          = 0
        self.hdop           = 999
        self.model          = 0
        self.num_st         = 0
        self.ip             = ip
        self.port           = port
        self.asn_parser     = asn_parser
        self.room_id        = self.getRoomId(self.ip, self.port)
        self.clients        = {}
        self.ready          = False

        self.pos_log_flag   = CfgData[kCfgSavePos]
        self.asn_log_flag   = CfgData[kCfgSaveAsn]

        self.log_dir        = _log_dir + '/' + self.ip
        if not os.path.exists(self.log_dir):
            os.makedirs(self.log_dir)
        self.pos_log_file   = self.log_dir+'/'+time.strftime('/%Y%m%d_%H%M%S.dat', time.localtime())


    def __del__(self):
        self.stop()


    @abc.abstractmethod
    def stop():
        '''抽象方法,关闭obu连接，需要子类实现'''
        pass


    @classmethod
    def getRoomId(cls,ip,port):
        return cls.__name__+'#'+ip + "#" + str(port)


    @classmethod
    def openDevice(cls, ip,port=9527,asn_parser=None,html_sender=None,sid='x'):
        cls.s_clear_thread_lock.acquire()
        if cls.s_clear_thread == None:
            cls.s_clear_thread = _thread.start_new_thread(cls.clearAllThread, ())
        cls.s_clear_thread_lock.release()
        room_id     = cls.getRoomId(ip, port)
        cls.s_cache_lock.acquire()
        if room_id in cls.s_cache:
            obu                     = cls.s_cache[room_id]
            print(getTimeStr(),cls.__name__ + ' exist, room_id = ' + room_id + ' , sid = '+sid+ ' , obu num = ', len(cls.s_cache))
        else:
            obu                     = cls(ip, port, asn_parser=asn_parser, html_sender=html_sender)
            cls.s_cache[room_id]    = obu
            print(getTimeStr(),cls.__name__ + ' open, room_id = ' + room_id + ' , sid = '+sid+ ' , obu num = ', len(cls.s_cache))
        obu.setClient(sid, time.time())
        cls.s_cache_lock.release()
        return obu


    @classmethod
    def clearAllThread(cls):
        while True:
            cls.s_cache_lock.acquire()
            for k in list(cls.s_cache):
                obu = cls.s_cache[k]
                l   = obu.removeOldClient()
                if l == 0:
                    obu.savePosEnd()
                    obu.stop()
                    del cls.s_cache[k]
                else:
                    print(getTimeStr(),'room_id = ' + obu.room_id + ' , client num = ',l)
            cls.s_cache_lock.release()
            time.sleep(_kClearInterval)


    @classmethod
    def updateAllClient(cls,sid,t):
        cls.s_cache_lock.acquire()
        for k in cls.s_cache:
            obu = cls.s_cache[k]
            if obu.findClient(sid):
                obu.setClient(sid, t)
        cls.s_cache_lock.release()


    @classmethod
    def saveCurrentPt(cls,sid,name):
        cls.s_cache_lock.acquire()
        for k in cls.s_cache:
            obu = cls.s_cache[k]
            if obu.findClient(sid):
                obu.saveSinglePoint(sid,name)
        cls.s_cache_lock.release()


    def findClient(self,sid):
        if sid in self.clients.keys():
            return True
        else:
            return False


    def setClient(self, sid, t):
        if sid:
            self.clients[sid]    = t


    def removeOldClient(self):
        now = time.time()
        for k in list(self.clients):
            t = now - self.clients[k]
            if t > _kClientTimeout:
                del self.clients[k]
                print(getTimeStr(),'room_id = '+self.room_id+' , del '+k+' , client num = ',len(self.clients))
        return len(self.clients)


    def savePos(self):
        if not self.pos_log_flag:
            return
        pre     = '{\n  "points":[\n'
        line    = '[' + format(self.lng, '.7f') + ',' + format(self.lat, '.7f') + ',' + format(self.heading,'.2f')
        line    += ',' + format(self.speed, '.2f') + ',' + format(self.hdop, '.2f')
        line    += ',"' + time.strftime('%Y:%m:%d-%H:%M:%S', time.localtime()) + '"]'
        f = open(self.pos_log_file,'a')
        if f.tell() <= len(pre):
            f.write(pre+line)
        else:
            f.write(',\n'+line)
        f.close()


    def savePosEnd(self):
        if not self.pos_log_flag:
            return
        if not os.path.exists(self.pos_log_file):
            return
        f = open(self.pos_log_file, 'a')
        if f.tell() != 0:
            f.write('\n]}\n')
        f.close()


    def saveSinglePoint(self,sid,name):
        line    = time.strftime('[%Y:%m:%d-%H:%M:%S]:', time.localtime())
        lng     = format(self.lng, '.7f')
        lat     = format(self.lat, '.7f')
        heading = format(self.heading,'.2f')
        line    += lng + ',' + lat + ',' + heading + ',' + name + '\n'
        f = open(self.s_pt_file, 'a',encoding='UTF-8')
        f.write(line)
        f.close()

