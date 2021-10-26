
import os, yaml


cfgDefaultObuType   = 'obusy'
cfgDefaultObuPort   = 30000
cfgDefaultAsnType   = 'asn2020'


kCfgKHostIp         = 'host_ip'
kCfgKObuInfo        = 'obu_info'
kCfgKObuFile        = 'obu_file'
kCfgKObuType        = 'obu_type'
kCfgKAsnType        = 'asn_type'
kCfgKObuPort        = 'obu_port'
kCfgKObuArray       = 'array'



_cur_dir        = os.path.dirname(__file__)
_yaml_path      = os.path.join(_cur_dir, 'cfg.yaml')



CfgData         = {}



def _getObuList(file_path):
    print('obu_file :', file_path)
    obu = {}
    obu[kCfgKObuType]   =   cfgDefaultObuType
    obu[kCfgKAsnType]   =   cfgDefaultAsnType
    obu[kCfgKObuPort]   =   cfgDefaultObuPort
    obu[kCfgKObuArray]  =   []
    try:
        f       = open(file_path, encoding='utf-8')
        data    = yaml.load(f, Loader=yaml.Loader)
        f.close()
    except Exception as e:
        print('read',file_path,'error ,',e)
        return
    obu_type    = data.get(kCfgKObuType)
    obu_port    = data.get(kCfgKObuPort)
    asn_type    = data.get(kCfgKAsnType)
    obu_list    = data.get(kCfgKObuArray)
    if obu_type:
        obu[kCfgKObuType]   = obu_type
    if obu_port:
        obu[kCfgKObuPort]   = obu_port
    if asn_type:
        obu[kCfgKAsnType]   = asn_type
    if obu_list:
        for line in obu_list:
            arr = line.strip().replace(' ', '').split(',')
            ip = arr[0]
            if len(arr) > 1:
                name    = arr[1]
            else:
                name    = ''
            obu[kCfgKObuArray].append((ip, name))
    CfgData[kCfgKObuInfo].append(obu)
    # print(CfgData)


def getConfig():
    print('cfg_file :',_yaml_path)
    try:
        f       = open(_yaml_path, encoding='utf-8')
        data    = yaml.load(f, Loader=yaml.Loader)
        f.close()
    except Exception as e:
        print('read',_yaml_path,'error ,',e)
        return

    host_ip                     = data.get(kCfgKHostIp)
    obu_file                    = data.get(kCfgKObuFile)
    CfgData[kCfgKObuInfo]       = []
    if host_ip:
        CfgData[kCfgKHostIp]    = host_ip
    else :
        CfgData[kCfgKHostIp]    = '127.0.0.1'
    if obu_file:
        file_path = os.path.join(_cur_dir, obu_file)
        _getObuList(file_path)