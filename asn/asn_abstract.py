import os
import abc
import asn1tools
import threading
import binascii


'''----------------'''


'''------------常量-----------------'''
lcRoadStraight          = 1


''''------本地dictionary 的 key---------'''
lIp                     = 'ip'
lType                   = 'type'
lNodes                  = 'nodes'
lRefPos                 = 'refPos'
lId                     = 'id'
lRegion                 = 'region'
lLaneWidth              = 'laneWidth'
lMovements              = 'movements'
lLanes                  = 'lanes'
lPhaseId                = 'phaseId'
lLaneID                 = 'laneID'
lPoints                 = 'points'
lPhaseLeft              = 'left'
lPhaseStraight          = 'straight'
lPhaseRight             = 'right'
lPhaseU                 = 'u'
lLon                    = 'lng'
lLat                    = 'lat'
lManeuvers              = 'maneuvers'
lDescription            = 'description'

'''rsi'''
lRtes                   = 'rtes'
lRtss                   = 'rtss'
lEventType              = 'eventType'
lSignType               = 'signType'
lPos                    = 'pos'
lRefPaths               = 'referencePaths'
lActivePath             = 'activePath'
lPathRadius             = 'pathRadius'

'''rsm'''
lParticipants           = 'participants'
lPtcId                  = 'ptcId'
lPtcType                = 'ptcType'
lSource                 = 'source'
lSpeed                  = 'speed'
lHeading                = 'heading'

'''bsm'''
lEvents                 = 'events'
lIdStr                  = 'idString'

'''spat'''
lIntersections          = 'intersections'
lPhases                 = 'phases'
lPhaseId                = 'phaseId'
lLeftTime               = 'leftTime'
lLightColor             = 'color'
lLightBlack             = 'black'
lLightRed               = 'red'
lLightGreen             = 'green'
lLightYellow            = 'yellow'




'''-----------------'''
class AsnAbstract(metaclass=abc.ABCMeta):

    @classmethod
    def asnInit(cls,version):
        dir = os.path.join(os.path.dirname(__file__), version)
        asn_files = []
        for root, dirs, files in os.walk(dir):
            for name in files:
                asn_files.insert(0, root + '/' + name)
        asn = asn1tools.compile_files(asn_files, 'uper')
        print(version + ' init , dir = ', dir)
        return asn

    @classmethod
    def parseAsn(cls,data,ip=''):
        tmp = cls()
        return tmp.parse(data,ip)

    @abc.abstractmethod
    def parse(self,data,ip=''):
        pass
