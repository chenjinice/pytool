
from asn.asn_abstract import *


'''asn为新四跨版本'''
_kAsn2020Dir            = 'asn2020'


'''--------asn分辨率-------'''
_kcResLatLng            = 1e-7
_kcResLaneWidth         = 1e-2
_kcResPathRadius        = 1e-1
_kcResSpeed             = 0.072
_kcResHeading           = 0.0125
_kcResTiming            = 0.1

'''----- asn 常量 -----'''
_kNodes                 = 'nodes'
_kRefPos                = 'refPos'
_kId                    = 'id'
_kRegion                = 'region'
_kInlinks               = 'inLinks'
_kLinkWidth             = 'linkWidth'
_kUpstreamNodeId        = 'upstreamNodeId'
_kMovement              = 'movements'
_kLanes                 = 'lanes'
_kLaneWidth             = 'laneWidth'
_kRemoteIntersection    = 'remoteIntersection'
_kConnectsTo            = 'connectsTo'
_kPhaseId               = 'phaseId'
_kLaneID                = 'laneID'
_kPoints                = 'points'
_kPosOffset             = 'posOffset'
_kOffsetLL              = 'offsetLL'
_kLon                   = 'lon'
_kLat                   = 'lat'
_kRefLon                = 'long'
_kRefLat                = 'lat'
_kPositionLatLon        = 'position-LatLon'
_kManeuvers             = 'maneuvers'
_kDescription           = 'description'
_kTextString            = 'textString'

'''rsi'''
_kRtes                  = 'rtes'
_kRtss                  = 'rtss'
_kEventType             = 'eventType'
_kEventPos              = 'eventPos'
_kSignType              = 'signType'
_kSignPos               = 'signPos'
_kRefPaths              = 'referencePaths'
_kActivePath            = 'activePath'
_kPathRadius            = 'pathRadius'

'''rsm'''
_kParticipants          = 'participants'
_kPtcId                 = 'ptcId'
_kPtcType               = 'ptcType'
_kSource                = 'source'
_kPos                   = 'pos'
_kSpeed                 = 'speed'
_kHeading               = 'heading'

'''bsm'''
_kSafetyExt             = 'safetyExt'
_kEvents                = 'events'

'''spat'''
_kIntersections         = 'intersections'
_kIntersectionId        = 'intersectionId'
_kPhases                = 'phases'
_kPhaseStates           = 'phaseStates'
_kLight                 = 'light'
_kTiming                = 'timing'
_kCounting              = 'counting'
_kStartTime             = 'startTime'
_kLikelyEndTime         = 'likelyEndTime'




class Asn2020(AsnAbstract):
    s_lock                  = threading.Lock()
    s_asn                   = None

    def __new__(cls, *args, **kwargs):
        cls.s_lock.acquire()
        if not cls.s_asn:
            cls.s_asn = cls.asnInit(_kAsn2020Dir)
        cls.s_lock.release()
        return AsnAbstract.__new__(cls)

    def __init__(self):
        self.__ref_lng       = 0
        self.__ref_lat       = 0

    def parse(self,data,ip=''):
        dict = {}
        dict[lType] = 'none'
        dict[lIp]   = ip
        try:
            decoded = self.s_asn.decode('MessageFrame', data)
            msg_type = decoded[0]
            dict[lType] = msg_type
            # print(ip, msg_type)
            if msg_type == 'bsmFrame':
                self.__parseBsm(decoded[1],dict)
            elif msg_type == 'mapFrame':
                self.__parseMap(decoded[1], dict)
            elif msg_type == 'rsmFrame':
                self.__parseRsm(decoded[1],dict)
            elif msg_type == 'spatFrame':
                self.__parseSpat(decoded[1],dict)
            elif msg_type == 'rsiFrame':
                self.__parseRsi(decoded[1],dict)
            else:
                pass
        except asn1tools.errors.DecodeError:
            print('asn decode failed')
        return dict


    '''--------------------'''
    def __parseNodeId(self,node_id, l):
        l[lId] = node_id[_kId]
        region = node_id.get(_kRegion)
        if not region:
            region = 0
        l[lRegion] = region

    def __parseRefPos(self,ref_pos, l):
        l[lLon] = ref_pos[_kRefLon] * _kcResLatLng
        l[lLat] = ref_pos[_kRefLat] * _kcResLatLng
        self.__ref_lng  = l[lLon]
        self.__ref_lat  = l[lLat]

    def __parsePosOffset(self,offset,l_pt):
        p_type  = offset[_kOffsetLL][0]
        pos     = offset[_kOffsetLL][1]
        if p_type == _kPositionLatLon:
            l_pt[lLon] = (pos[_kLon]) * _kcResLatLng
            l_pt[lLat] = (pos[_kLat]) * _kcResLatLng
        else:
            l_pt[lLon] = (pos[_kLon] + self.__ref_lng) * _kcResLatLng
            l_pt[lLat] = (pos[_kLat] + self.__ref_lat) * _kcResLatLng

    def __parseDescription(self,des):
        ret = ''
        if not des:
            return ret
        if des[0] == _kTextString:
            ret  =  des[1]
        else:
            ret  =  binascii.b2a_hex(des[1]).decode('utf-8')
        return ret

    def __parseBitString(self, bit_string):
        '''bit string转int,涉及到位反转和字节倒序,比较恶心'''
        ret = 0
        if not bit_string:
            return ret
        b_new = bytearray()
        for i in bit_string[0]:
            value = 0
            for k in range(8):
                value = value << 1
                value |= (i >> k) & 0x01
            b_new.append(value)
        ret = int.from_bytes(b_new, byteorder='little')
        return ret

    '''------------------map---------------------'''
    def __parseMap(self,map, l_map):
        l_map[lNodes] = []
        for node in map[_kNodes]:
            l_node = {}
            l_node[lId] = {}
            l_node[lRefPos] = {}
            self.__parseNodeId(node[_kId], l_node[lId])
            self.__parseRefPos(node[_kRefPos], l_node[lRefPos])
            l_node[lLanes] = []
            self.__parseInlinks(node.get(_kInlinks), l_node[lLanes])
            l_map[lNodes].append(l_node)

    def __parseInlinks(self,links, l_lanes):
        '''解析links'''
        if not links:
            return
        for link in links:
            link_width = link[_kLinkWidth]
            movements = {}
            self.__parseMovements(link.get(_kMovement), movements)
            self.__parseLanes(link[_kLanes], l_lanes, link_width, movements)
            if not l_lanes:
                l_lane = {}
                l_lane[lLaneWidth] = link_width
                l_lane[lMovements] = movements
                l_lane[lManeuvers] = lcRoadStraight
                l_lane[lPoints]    = []
                self.__parsePoints(link.get(_kPoints), l_lane[lPoints])

    def __parseLanes(self,lanes, l_lanes, link_width, movements):
        '''解析lanes'''
        conects_to = {}
        '''先把一个link对应的3个灯(不一定是3个)都解析出来'''
        for lane in lanes:
            maneuvers = self.__parseBitString(lane.get(_kManeuvers))
            self.__parseConnectsTo(lane.get(_kConnectsTo), maneuvers, conects_to)
        '''然后解析lane'''
        for lane in lanes:
            l_lane = {}
            l_lane[lPoints] = []
            self.__parsePoints(lane.get(_kPoints), l_lane[lPoints])
            if not l_lane[lPoints]:
                continue
            if lane.get(_kLaneWidth):
                l_lane[lLaneWidth] = lane[_kLaneWidth] * _kcResLaneWidth
            else:
                l_lane[lLaneWidth] = link_width * _kcResLaneWidth
            if conects_to:
                l_lane[lMovements] = conects_to
            else:
                l_lane[lMovements] = movements
            l_lane[lLaneID] = lane[_kLaneID]
            maneuvers       = self.__parseBitString(lane.get(_kManeuvers))
            if maneuvers == 0 :
                maneuvers   = lcRoadStraight
            l_lane[lManeuvers] = maneuvers
            l_lanes.append(l_lane)

    def __parseMovements(self,m, l_m):
        '''解析link里的movements'''
        if not m:
            return
        direct = [lPhaseStraight, lPhaseLeft, lPhaseRight, lPhaseU]
        m_len = min(len(direct), len(m))
        for i in range(m_len):
            phase_id = m[i].get(_kPhaseId)
            if phase_id:
                l_m[direct[i]] = phase_id

    def __parseConnectsTo(self,conects, maneuvers, l_conencts):
        '''解析lane里的connectsTo'''
        if not conects:
            return
        direct = [lPhaseStraight, lPhaseLeft, lPhaseRight, lPhaseU]
        d_exist = []
        for i in range(len(direct)):
            if (maneuvers >> i) & 0x01:
                d_exist.append(direct[i])
        min_len = min(len(d_exist), len(conects))
        for i in range(min_len):
            phase_id = conects[i].get(_kPhaseId)
            if phase_id:
                l_conencts[d_exist[i]] = phase_id
        # print(min_len,d_exist,l)

    def __parsePoints(self,pts, l_pts):
        '''解析lane上的经纬度点'''
        if not pts:
            return
        for pt in pts:
            l_pt = {}
            self.__parsePosOffset(pt[_kPosOffset],l_pt)
            l_pts.append(l_pt)
    '''---------------map end-----------------'''


    '''---------------rsi start-----------------'''
    def __parseRsi(self,rsi, l_rsi):
        l_rsi[lRefPos]  = {}
        self.__parseRefPos(rsi[_kRefPos], l_rsi[lRefPos])
        l_rsi[lRtes]    = []
        l_rsi[lRtss]    = []
        self.__parseRtes(rsi.get(_kRtes),l_rsi[lRtes])
        self.__parseRtss(rsi.get(_kRtss),l_rsi[lRtss])

    def __parseRtes(self,rtes,l_rtes):
        if not rtes:
            return
        for rte in rtes:
            l_rte               = {}
            l_rte[lRefPaths]    = []
            l_rte[lEventType]   = rte[_kEventType]
            l_rte[lDescription] = self.__parseDescription(rte.get(_kDescription))
            pos =               rte.get(_kEventPos)
            if pos:
                l_rte[lPos]     = {}
                self.__parsePosOffset(pos,l_rte[lPos])
            self.__parseReferencePaths(rte.get(_kRefPaths), l_rte[lRefPaths])
            l_rtes.append(l_rte)

    def __parseRtss(self,rtss,l_rtss):
        if not rtss:
            return
        for rts in rtss:
            l_rts               = {}
            l_rts[lRefPaths]    = []
            l_rts[lSignType]    = rts[_kSignType]
            l_rts[lDescription] = self.__parseDescription(rts.get(_kDescription))
            pos                 = rts.get(_kSignPos)
            if pos:
                l_rts[lPos] = {}
                self.__parsePosOffset(pos, l_rts[lPos])
            self.__parseReferencePaths(rts.get(_kRefPaths), l_rts[lRefPaths])
            l_rtss.append(l_rts)

    def __parseReferencePaths(self,paths,l_paths):
        if not paths:
            return
        for path in paths:
            l_path              = {}
            l_path[lActivePath] = []
            l_path[lPathRadius] = path[_kPathRadius] * _kcResPathRadius
            for pt in path[_kActivePath]:
                l_pt            = {}
                self.__parsePosOffset(pt,l_pt)
                l_path[lActivePath].append(l_pt)
            l_paths.append(l_path)
    '''---------------rsi end---------------'''


    '''---------------rsm start-------------'''
    def __parseRsm(self,rsm,l_rsm):
        l_rsm[lRefPos]          = {}
        l_rsm[lParticipants]    = []
        self.__parseRefPos(rsm[_kRefPos],l_rsm[lRefPos])
        for ptc in rsm[_kParticipants]:
            l_ptc               = {}
            l_ptc[lRsuId]       = binascii.b2a_hex(rsm[_kId]).decode('utf-8')
            l_ptc[lPos]         = {}
            l_ptc[lPtcId]       = ptc[_kPtcId]
            l_ptc[lPtcType]     = ptc[_kPtcType]
            l_ptc[lSpeed]       = ptc[_kSpeed]   * _kcResSpeed
            l_ptc[lHeading]     = ptc[_kHeading] * _kcResHeading
            self.__parsePosOffset(ptc[_kPos],l_ptc[lPos])
            l_rsm[lParticipants].append(l_ptc)
    '''---------------rsm end---------------'''


    '''---------------bsm start-------------'''
    def __parseBsm(self, bsm, l_bsm):
        l_bsm[lPos]     = {}
        l_bsm[lId]      = binascii.b2a_hex(bsm[_kId]).decode('utf-8')
        # l_bsm[lIdStr]   = bsm[_kId].decode('ascii')
        l_bsm[lIdStr]   = ''
        l_bsm[lSpeed]   = bsm[_kSpeed]   * _kcResSpeed
        l_bsm[lHeading] = bsm[_kHeading] * _kcResHeading
        l_bsm[lEvents]  = 0
        self.__parseRefPos(bsm[_kPos],l_bsm[lPos])
        safety          = bsm.get(_kSafetyExt)
        if safety :
            l_bsm[lEvents]  = self.__parseBitString(safety.get(_kEvents))
        # print(l_bsm)
    '''---------------bsm end---------------'''


    '''---------------spat start-------------'''
    def __parseSpat(self,spat,l_spat):
        l_spat[lIntersections]  = []
        for intersection in spat[_kIntersections]:
            l_section           = {}
            l_section[lId]      = {}
            l_section[lPhases]  = []
            self.__parseNodeId(intersection[_kIntersectionId],l_section[lId])
            self.__parsePhases(intersection[_kPhases],l_section[lPhases])
            l_spat[lIntersections].append(l_section)

    def __parsePhases(self,phases,l_phases):
        for phase in phases:
            l_phase                 = {}
            l_phase[lPhaseId]       = phase[_kId]
            self.__parseLightLeftTime(phase.get(_kPhaseStates),l_phase)
            l_phases.append(l_phase)

    def __parseLightLeftTime(self,phase_states,l_phase):
        l_phase[lLightColor]        = lLightBlack
        l_phase[lLeftTime]          = 0
        if not phase_states:
            return
        for state in phase_states:
            color                   = state[_kLight]
            timing                  = state.get(_kTiming)
            if not timing:
                continue
            if timing[0] == _kCounting:
                if timing[1][_kStartTime] != 0:
                    continue
                l_phase[lLeftTime]          = timing[1][_kLikelyEndTime] * _kcResTiming
                if (color == 'red') or (color == 'flashing-red'):
                    l_phase[lLightColor]    = lLightRed
                elif (color == 'green') or (color == 'flashing-green') or (color == 'permissive-green') or (color == 'protected-green'):
                    l_phase[lLightColor]    = lLightGreen
                elif (color == 'yellow') or (color == 'flashing-yellow'):
                    l_phase[lLightColor]    = lLightYellow
                else:
                    l_phase[lLightColor]    = lLightBlack
    '''---------------spat end  -------------'''




'''------------添加到支持的asn-------------'''
asnAll[Asn2020.__name__.lower()] = Asn2020
