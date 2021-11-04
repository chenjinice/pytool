
import os, yaml


_cur_dir                    = os.path.join(os.path.abspath('.'),'config')

'''----------------------------------------'''
_cfgFilePath                = os.path.join(_cur_dir, 'cfg.yaml')
_cfgDefaultHostIp           = '127.0.0.1'
_cfgDefaultHostPort         = 80
_cfgDefaultSaveAsn          = False
_cfgDefaultSavePos          = False
_cfgDefaultObuFile          = 'obu.yaml'
_cfgDefaultObuInfo          = []

_cfgDefaultObuType          = 'obusy'
_cfgDefaultObuPort          = 30000
_cfgDefaultAsnType          = 'asn2020'


'''----------------------------------------'''
kCfgHostIp                  = 'host_ip'
kCfgHostPort                = 'host_port'
kCfgObuInfo                 = 'obu_info'
kCfgObuFile                 = 'obu_file'
kCfgSaveAsn                 = 'save_asn'
kCfgSavePos                 = 'save_pos'

kCfgObuType                 = 'obu_type'
kCfgAsnType                 = 'asn_type'
kCfgObuPort                 = 'obu_port'
kCfgObuArray                = 'array'


'''----------------------------------------'''
CfgData                     = {}
CfgData[kCfgHostIp]         = _cfgDefaultHostIp
CfgData[kCfgHostPort]       = _cfgDefaultHostPort
CfgData[kCfgSaveAsn]        = _cfgDefaultSaveAsn
CfgData[kCfgSavePos]        = _cfgDefaultSavePos
CfgData[kCfgObuFile]        = _cfgDefaultObuFile
CfgData[kCfgObuInfo]        = _cfgDefaultObuInfo


def getConfig():
    print('cfg_file :',_cfgFilePath)
    try:
        f       = open(_cfgFilePath, encoding='utf-8')
        data    = yaml.load(f, Loader=yaml.Loader)
        f.close()
    except Exception as e:
        print('read',_cfgFilePath,'error ,',e)
        return
    _getValue(CfgData, data, kCfgHostIp, _cfgDefaultHostIp)
    _getValue(CfgData, data, kCfgHostPort, _cfgDefaultHostPort)
    _getValue(CfgData, data, kCfgSaveAsn, _cfgDefaultSaveAsn)
    _getValue(CfgData, data, kCfgSavePos, _cfgDefaultSavePos)
    _getValue(CfgData, data, kCfgObuFile, _cfgDefaultObuFile)
    file_path = os.path.join(_cur_dir, CfgData[kCfgObuFile])
    _getObuList(file_path)


def _getValue(cfg,data,key,default):
    value = data.get(key)
    if not value:
        value       =   default
    cfg[key]        =   value
    return value


def _getObuList(file_path):
    print('obu_file :', file_path)
    try:
        f       = open(file_path, encoding='utf-8')
        data    = yaml.load(f, Loader=yaml.Loader)
        f.close()
    except Exception as e:
        print('read',file_path,'error ,',e)
        return
    obu                 = {}
    obu[kCfgObuArray]   = []
    _getValue(obu, data, kCfgObuType, _cfgDefaultObuType)
    _getValue(obu, data, kCfgObuPort, _cfgDefaultObuPort)
    _getValue(obu, data, kCfgAsnType, _cfgDefaultAsnType)
    obu_list = data.get(kCfgObuArray)
    if obu_list:
        for line in obu_list:
            arr = line.strip().replace(' ', '').split(',')
            ip = arr[0]
            if len(arr) > 1:
                name    = arr[1]
            else:
                name    = ''
            obu[kCfgObuArray].append((ip, name))
    CfgData[kCfgObuInfo].append(obu)






'''-------------------------------'''
getConfig()
for i in CfgData.keys():
    print(i,':',CfgData[i])

