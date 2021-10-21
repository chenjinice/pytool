
''' obu通用数据 '''

import re
import abc
import time
import threading


'''----'''
_kDefaultPort           = 10000


'''----------key----------'''
kPositionType           = 'host_pt'

kType                   = 'type'
kIp                     = 'ip'
kLng                    = 'lng'
kLat                    = 'lat'
kElev                   = 'elev'
kSpeed                  = 'speed'
kHeading                = 'heading'
kNumSt                  = 'num_st'
kHdop                   = 'hdop'
kModel                  = 'model'



def getTimeStr():
    str = time.strftime("[%Y-%m-%d %H:%M:%S]", time.localtime())
    return str

def getRoomId(ip, port):
    return ip+"-"+str(port)


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
    s_lock                  = threading.Lock()
    s_cache                 = {}

    def __init__(self, ip,port=None,asn_parser=None, html_sender=None):
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
        self.asn_parser     = asn_parser
        if port:
            self.port       = port
        else:
            self.port = _kDefaultPort
        self.room_id        = getRoomId(self.ip, self.port)


    def __del__(self):
        self.stop()


    @abc.abstractmethod
    def stop():
        '''抽象方法,关闭obu连接，需要子类实现'''
        pass

    @classmethod
    def openDevice(cls, ip,port=None,asn_parser=None, html_sender=None):
        '''类方法，新建obu类'''
        ret     = None
        room_id = getRoomId(ip,port)
        cls.s_lock.acquire()
        if room_id in cls.s_cache:
            ret = cls.s_cache[room_id]
            print(cls.__name__ + ':cache exist,id=' + room_id + ',cache len=', len(cls.s_cache))
        else:
            ret = cls(ip, port, asn_parser=asn_parser, html_sender=html_sender)
            cls.s_cache[room_id] = ret
            print(cls.__name__ + ':cache add,id=' + room_id + ',cache len=', len(cls.s_cache))
        cls.s_lock.release()
        return ret

    @classmethod
    def closeDevice(cls,ip):
        '''类方法，删除obu设备'''
        cls.cacheDelete(ip)


    @classmethod
    def cacheAdd(cls, obu):
        '''类方法，把obu类添加到缓存'''
        room_id = obu.room_id
        if len(room_id) == 0:
            return
        cls.s_lock.acquire()
        if room_id in cls.s_cache:
            pass
        else:
            cls.s_cache[room_id] = obu
        print(cls.__name__ + ':cache add,id=' + room_id + ',cache len=', len(cls.s_cache))
        cls.s_lock.release()

    @classmethod
    def cacheDelete(cls, room_id):
        '''类方法，从缓存中删除'''
        cls.s_lock.acquire()
        if room_id in cls.s_cache:
            cls.s_cache[room_id].ready = False
            cls.s_cache[room_id].stop()
            del cls.s_cache[room_id]
            print(cls.__name__ + ':cache delete,id=' + room_id + ',cache len=', len(cls.s_cache))
        cls.s_lock.release()