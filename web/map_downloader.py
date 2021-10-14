'''
    原来的Google地图地址，被禁了，貌似
        卫星图:http://mt1.google.com/vt/lyrs=s&x={x}&y={y}&z={z}
        普通图:http://mt1.google.com/vt/lyrs=m&x={x}&y={y}&z={z}
    后来的Google地图地址:
        卫星图:https://khms1.google.com/kh/v=908?x={x}&y={y}&z={z}
        普通图:
'''

import os , math , time , threading , _thread , requests
from urllib     import request , parse
from ping3      import ping



_kMaxZoom           = 22
_kMaxCount          = 100

_kType              = 'type'
_kAddr              = 'addr'
_kUrl               = 'url'
_kDir               = 'dir'

_cur_dir            = os.path.dirname(__file__)


class ImgInfo():
    def __init__(self):
        self.addr       = ''
        self.url        = ''
        self.dir        = ''
        self.fname      = ''


class MapDownloader(object):
    map_cfg         = []
    ping_status     = {}
    list            = []
    init_lock       = threading.Lock()
    lock            = threading.Lock()

    map_cfg.append( {_kType: [0, 100], _kUrl: 'https://khms1.google.com/kh/v=908?x={x}&y={y}&z={z}', _kDir: _cur_dir + '/map/google_satellite', _kAddr: ''} )
    map_cfg.append( {_kType: [1, 101], _kUrl: 'http://mt1.google.com/vt/lyrs=m&x={x}&y={y}&z={z}',   _kDir: _cur_dir + '/map/google_map',       _kAddr: ''} )
    for cfg in map_cfg:
        addr                = parse.urlparse(cfg[_kUrl]).netloc
        cfg[_kAddr]         = addr
        ping_status[addr]   = False

    ready                   = False
    down_thread             = None
    ping_thread             = None
    instance                = None
    signal_sender           = None
    update_time             = time.time()


    def __new__(cls, *args, **kwargs):
        cls.init_lock.acquire()
        cls.ready           = True
        if not cls.instance:
            cls.instance    = super().__new__(cls)
        if not cls.down_thread:
            cls.down_thread = _thread.start_new_thread(cls.__downThread,())
        if not cls.ping_thread:
            cls.ping_thread = _thread.start_new_thread(cls.__pingThread,())
        cls.init_lock.release()
        return cls.instance


    def __init__(self):
        pass


    @classmethod
    def __downThread(cls):
        while cls.ready:
            img             = None
            cls.lock.acquire()
            if len(cls.list) > 0:
                img         = cls.list[0]
            cls.lock.release()
            if img:
                cls.downloadImage(img)
                list_len = cls.liseDelete(img)
                cls.sendUpdateSignal(list_len)
            else:
                time.sleep(1)

    @classmethod
    def __pingThread(cls):
        while cls.ready:
            for addr in cls.ping_status.keys():
                ret = ping(addr)
                if ret:
                    cls.ping_status[addr]   = True
                    print('ping %s ok, %.4f s' %(addr,ret))
                else:
                    cls.ping_status[addr]   = False
                    print('ping %s fail'  %addr)
            time.sleep(10)


    @classmethod
    def getBounds(cls,lat1,lat2,lng1,lng2,zoom,map_type):
        if zoom > _kMaxZoom:
            print('zoom > ',_kMaxZoom)
            return
        ins     = cls()
        x_range = ins.getX(lng1,lng2,zoom)
        y_range = ins.getY(lat1,lat2,zoom)
        # print(x_range,y_range)
        ins.addAllImages(x_range, y_range, zoom, map_type)


    def getX(self,lng1,lng2,zoom):
        ret     = []
        max     = pow(2,zoom)
        x1      = pow(2,zoom-1)*(lng1/180.0+1)
        x2      = pow(2,zoom-1)*(lng2/180.0+1)
        x1      = int(x1)
        x2      = int(x2)
        x1      %= max
        x2      %= max
        if x1 > x2:
            ret.append(x2)
            ret.append(x1)
        else:
            ret.append(x1)
            ret.append(x2)
        return ret


    def getY(self,lat1,lat2,zoom):
        ret     = []
        lat_max = 85
        if      lat1 > lat_max  : lat1  = lat_max
        elif    lat1 < -lat_max : lat1  = -lat_max
        if      lat2 > lat_max  : lat2  = lat_max
        elif    lat2 < -lat_max : lat2  = -lat_max
        tmp1    = math.tan(math.pi * lat1 / 180.0) + 1.0 / math.cos(math.pi * lat1 / 180.0)
        tmp2    = math.tan(math.pi * lat2 / 180.0) + 1.0 / math.cos(math.pi * lat2 / 180.0)
        y1      = pow(2, zoom - 1) * (1 - math.log(tmp1) / math.pi)
        y2      = pow(2, zoom - 1) * (1 - math.log(tmp2) / math.pi)
        y1      = int(y1)
        y2      = int(y2)
        if y1 > y2:
            ret.append(y2)
            ret.append(y1)
        else:
            ret.append(y1)
            ret.append(y2)
        return ret


    def addAllImages(self, x_range, y_range, zoom, map_type):
        for x in range(x_range[0],x_range[1]+1):
            for y in range(y_range[0],y_range[1]+1):
                img     = self.getImgInfo(x, y, zoom, map_type)
                if self.ping_status[img.addr] and (not os.path.exists(img.fname)):
                    self.listAdd(img)
        pass


    @classmethod
    def listAdd(cls,img):
        exist               = False
        cls.lock.acquire()
        for value in cls.list:
            if value.url == img.url:
                exist       = True
                break
        if not exist:
            cls.list.insert(0, img)
        if len(cls.list) > _kMaxCount:
            cls.list.pop()
        cls.lock.release()


    @classmethod
    def liseDelete(cls,img):
        cls.lock.acquire()
        for value in cls.list[::-1]:
            if value.url == img.url:
                cls.list.remove(value)
        ret = len(cls.list)
        cls.lock.release()
        return ret


    def getMapCfg(self,map_type):
        dict = self.map_cfg[0]
        for cfg in self.map_cfg:
            if map_type in cfg[_kType]:
                dict = cfg
                break
        return dict


    def getImgInfo(self, x, y, z, map_type):
        img             = ImgInfo()
        x               = str(x)
        y               = str(y)
        z               = str(z)
        cfg             = self.getMapCfg(map_type)
        img.addr        = cfg[_kAddr]
        img.url         = cfg[_kUrl].replace('{x}',x).replace('{y}',y).replace('{z}',z)
        img.addr        = cfg[_kAddr]
        img.dir         = cfg[_kDir] + '/'+z + '/' + x
        img.fname       = cfg[_kDir] + '/'+z + '/' + x + '/' + y + '.jpg'
        # print(img.addr,img.url,img.dir,img.fname)
        return img


    @classmethod
    def downloadImage(cls,img):
        try:
            r   = requests.get(img.url,timeout=3)
            if r.status_code == 200:
                if not os.path.exists(img.dir):
                    os.makedirs(img.dir)
                with open(img.fname,'wb') as f:
                    f.write(r.content)
                    print(img.url, 'download ok')
            else:
                print(img.url,'download failed ,',r.status_code)
        except Exception as e:
            print(img.url,'download error')


    @classmethod
    def sendUpdateSignal(cls,list_len):
        if not cls.signal_sender:
            return
        now     = time.time()
        t       = now -cls.update_time
        if (t > 3) or (list_len == 0):
            cls.update_time = now
            cls.signal_sender()

