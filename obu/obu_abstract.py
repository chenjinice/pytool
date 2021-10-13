
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

def checkIp(ip):
    if not ip:
        return False
    compile = re.compile('^(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|[1-9])\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)$')
    if compile.match(ip):
        return True
    else:
        return False


class ObuAbstract(metaclass=abc.ABCMeta):
    ''' obu 抽象类 '''
    s_lock                  = threading.Lock()
    s_cache                 = {}

    def __init__(self, ip, asn_parser=None, html_sender=None):
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
        self.port           = _kDefaultPort
        self.asn_parser     = asn_parser


    def __del__(self):
        self.stop()


    @abc.abstractmethod
    def stop():
        '''抽象方法,关闭obu连接，需要子类实现'''
        pass

    @classmethod
    def openDevice(cls, ip, asn_parser=None, html_sender=None):
        '''类方法，新建obu类'''
        tmp = cls(ip,asn_parser=asn_parser,html_sender=html_sender)
        cls.cacheAdd(tmp)

    @classmethod
    def closeDevice(cls,ip):
        '''类方法，删除obu设备'''
        cls.cacheDelete(ip)


    @classmethod
    def cacheAdd(cls, obu):
        '''类方法，把obu类添加到缓存'''
        if len(obu.ip) == 0:
            return
        cls.s_lock.acquire()
        if obu.ip in cls.s_cache:
            pass
        else:
            cls.s_cache[obu.ip] = obu
        print(cls.__name__ + ' : cache add , ip = ' + obu.ip + ' , count = ', len(cls.s_cache))
        cls.s_lock.release()

    @classmethod
    def cacheDelete(cls, ip):
        '''类方法，从缓存中删除'''
        cls.s_lock.acquire()
        if ip in cls.s_cache:
            cls.s_cache[ip].ready = False
            cls.s_cache[ip].stop()
            del cls.s_cache[ip]
            print(cls.__name__ + ' : cache delete , ip = ' + ip + ' , count = ', len(cls.s_cache))
        cls.s_lock.release()