import _thread
import time
import socket
import struct
import threading

from obu.obu_abstract import *


'''--------------------沈阳OBU，asn为新四跨版本-----------------------------'''
_kUdpBuffSize       = 4096
_kHeartInterval     = 2



class ObuSy(ObuAbstract):
    '''沈阳OBU'''
    s_thread_lock           = threading.Lock()
    s_thread                = None


    '创建一个所有obu发心跳的线程，只会创建一次'
    def __new__(cls, *args, **kwargs):
        p = ObuAbstract.__new__(cls)
        cls.s_thread_lock.acquire()
        if cls.s_thread == None:
            cls.s_thread = _thread.start_new_thread(cls.__heartThread, ())
        cls.s_thread_lock.release()
        return p


    def __init__(self, ip,port = 30000,asn_parser=None, html_sender=None):
        ObuAbstract.__init__(self,ip,port,asn_parser, html_sender)
        self.port           = port
        self.room_id        = self.getRoomId(self.ip, self.port)
        self.buffsize       = _kUdpBuffSize
        self.udp_fd         = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp_fd.bind( ('',0) )
        self.ready          = True
        _thread.start_new_thread(self._start,())


    '类方法，发送心跳线程'
    @classmethod
    def __heartThread(cls):
        while True:
            cls.s_cache_lock.acquire()
            for obu in cls.s_cache.values():
                obu.__sendHeart()
            cls.s_cache_lock.release()
            time.sleep(_kHeartInterval)


    def _start(self):
        while self.ready:
            try:
                data,addr = self.udp_fd.recvfrom(self.buffsize)
                if (len(data) <= 4):
                    continue
                if data[0] == ord('g') and data[1] == ord('p') and data[2] == ord('s') and data[3] == ord('0') and len(
                        data) >= 36:
                    dict = self.__parseGps(data)
                    if self.html_sender:
                        self.html_sender(dict,self.room_id)
                else:
                    if self.asn_parser and self.html_sender:
                        dict = self.asn_parser(data,self.ip)
                        self.html_sender(dict,self.room_id)
            except ConnectionResetError:
                continue
            except OSError:
                break


    def stop(self):
        self.ready      = False
        self.udp_fd.close()
        self.savePosEnd()


    def __sendHeart(self):
        self.udp_fd.sendto('hi'.encode(), (self.ip,self.port))


    def __parseGps(self, data):
        head,lng,lat,elev,speed,heading,num_st,hdop,model = struct.unpack("9i",data)
        self.lng        = lng * 1e-7
        self.lat        = lat * 1e-7
        self.elev       = elev * 1e-1
        self.speed      = speed * 1e-2
        self.heading    = heading * 1e-2
        self.num_st     = num_st
        self.hdop       = hdop  * 1e-1
        self.model      = model

        self.savePos()
        dict            = {}
        dict[oType]     = oPositionType
        dict[oIp]       = self.ip
        dict[oLng]      = self.lng
        dict[oLat]      = self.lat
        dict[oElev]     = self.elev
        dict[oSpeed]    = self.speed
        dict[oHeading]  = self.heading
        dict[oNumSt]    = self.num_st
        dict[oHdop]     = self.hdop
        dict[oModel]    = self.model
        return dict



'''------------添加到支持的obu-------------'''
obuAll[ObuSy.__name__.lower()] = ObuSy