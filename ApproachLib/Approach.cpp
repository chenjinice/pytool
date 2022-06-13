#include "Approach.h"
#include <EwayFunc/FuncSdkLib.h>
#include <EwayMath/BasicMath.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>
#include <EwayMath/CoordinateConversion.h>
#include "TestUi.h"
#include "PosTransform.h"


Declare_FunSdk_Lib(Approach)
Declare_Function_ID("6e47a7c7-be00-4121-aad4-97eaac88b5d0")
Declare_Function_Priority(FunPri_Normal)

RegisterMessage_Start
RegisterMessage_End

BEGIN_FIFEMSG_MAP(Approach,CFuncInterface)
//MSG_FIFMAP(FunMsg_User+1,OnMessageProcess)
END_FIFMSG_MAP


using namespace std;
using namespace ewayos;
using namespace Eigen;


static const echar * ClassName          = "Approach";


Approach::Approach()
{
    m_sParam.dWheelDistance             = 1.05;
    m_sParam.dWheelL                    = 1.75;
    m_sParam.dWheelCircle               = 2.136;
    m_sParam.dWheelRadius               = m_sParam.dWheelCircle/(2*EPI);
    m_sParam.dShortArmTipX              = 1.3056;
    m_sParam.dShortArmTipY              = 0.77676;
    m_sParam.dLongArmJointX             = 0.69859;
    m_sParam.dLongArmJointY             = -0.252;
    m_sParam.dLongArmLength             = 1.0674;
    m_sParam.dMaxWheelAngle             = 45.0/180.0*EPI;

    m_sParam.dPaperRadiusOffset         = 0.03;
    m_sParam.dPaperValidTime            = 0.3;
    m_sParam.dPaperAllowedDistance      = 1.0;
    m_sParam.dPaperMinRelativeX         = 1.5;
    m_sParam.dRobotPosValidTime         = 0.3;
    m_sParam.dWheelAngleValidTime       = 0.3;
    m_sParam.dWheelAngleAllowedDiff     = 6.0/180.0*EPI;
    m_sParam.dDistanceToSlow            = 2.5;
    m_sParam.dDistanceToStop            = 0.03;
    m_sParam.dMaxSpeed                  = 0.5;
    m_sParam.dMinSpeed                  = 0.05;

    m_sParam.dStep1WheelAngle           = 15.0/180.0*EPI;
    m_sParam.dStep1Yn                   = 0;
}

Approach::~Approach()
{
}

eint Approach::Initialize(std::vector<eint> &vFeatureList,ebool &bLimbAuthority,edouble &dTimeOut)
{
    vFeatureList.clear();
    vFeatureList.push_back(Message_Code_PaperRollDect);
    vFeatureList.push_back(Message_Code_Slam_Localization);
    vFeatureList.push_back(Message_Code_WheelState);

    bLimbAuthority          = true;
    dTimeOut                = 3;

    if( (false == this->GetXnYn()) || (false == this->GetStep1Xn()) )
    {
        FunQuit(INIT_FAILED);
        return INIT_FAILED;
    }else {
        Log::inf(ClassName,"Xn=%.2f,Yn=%.2f,step1Xn=%.2f,step1Yn=%.2f",m_dXn,m_dYn,m_dStep1Xn,m_dStep1Yn);
        return ERR_NONE;
    }
}

eint Approach::Destroy()
{
    return ERR_NONE;
}

eint Approach::CheckMsgCode(eint nMsgCode)
{
    UNUSED(nMsgCode);
    return MsgProcCode_Record;
}

// 获取参数
eint Approach::ProcParentParam(CSetParamMsg *piMsg)
{
    uint nRet = ERR_NONE;
    edouble dApproachPaperRadius = 0 ,dCachedPaperRadius = 0 ,dPayload = 0;
    if(false == GetParamFloatVal(piMsg->m_strPara,"ObserveTargetRadius", dApproachPaperRadius))
    {
    }
    if(false == GetParamFloatVal(piMsg->m_strPara,"AppoarchTargetRadius", dCachedPaperRadius))
    {
    }
    if(false == GetParamFloatVal(piMsg->m_strPara,"Payload", dPayload))
    {
    }
    ewayos::Log::inf(ClassName,"get parameter,catchedPaperRadius=%.3f,approachPaperRadius=%.3f",dCachedPaperRadius,dApproachPaperRadius);
    if( (dApproachPaperRadius != m_dApproachPaperRadius) || (dCachedPaperRadius != m_dCatchPaperRadius)  )
    {
        if( (false == this->GetXnYn()) || (false == this->GetStep1Xn()) )
        {
            FunQuit(INIT_FAILED);
            nRet = INIT_FAILED;
        }else {
            Log::inf(ClassName,"Xn=%.2f,Yn=%.2f,step1Xn=%.2f,step1Yn=%.2f",m_dXn,m_dYn,m_dStep1Xn,m_dStep1Yn);
        }
    }
    return nRet;
}

void Approach::JobStart()
{
    SetTimer(100);
    return ;
}

void Approach::JobFailed(eint nErrCode)
{
    UNUSED(nErrCode);
    return ;
}

eint Approach::ProcTimer(edouble dTimeStamp)
{
    UNUSED(dTimeStamp);
    if(false == m_bStep1Finished)
    {
        this->ApproachStep1();
    }else {
        this->ApproachStep2();
    }
    //    this->ApproachStep2();
    return ERR_NONE;
}

// 计算叉子抓取纸卷直径时，圆心的坐标
ebool Approach::GetXnYn()
{
    edouble dR1 = m_dCatchPaperRadius * 2;
    edouble dD  = sqrt(pow(m_sParam.dShortArmTipX - m_sParam.dLongArmJointX, 2) + pow(m_sParam.dShortArmTipY - m_sParam.dLongArmJointY, 2));
    if(dD == 0)
    {
        Log::err(ClassName,"GetXnYn ,dD = 0");
        return false;
    }
    edouble dA  = dD/2.0 + (pow(dR1, 2) - pow(m_sParam.dLongArmLength, 2))/(2.0 * dD);
    if(fabs(dR1) < fabs(dA) )
    {
        Log::err(ClassName,"GetXnYn ,dR1(%f) < dA(%f)",dR1,dA);
        return false;
    }
    edouble dH = sqrt(pow(dR1, 2) - pow(dA, 2));
    m_dXn       = m_sParam.dShortArmTipX + dA/(2 * dD) * (m_sParam.dLongArmJointX - m_sParam.dShortArmTipX) + dH/(2 * dD) * (m_sParam.dShortArmTipY - m_sParam.dLongArmJointY);
    m_dYn       = m_sParam.dShortArmTipY + dA/(2 * dD) * (m_sParam.dLongArmJointY - m_sParam.dShortArmTipY) - dH/(2 * dD) * (m_sParam.dShortArmTipX - m_sParam.dLongArmJointX);
    return true;
}

// 算出步骤1的Xn
ebool Approach::GetStep1Xn()
{
    edouble dTheta  = m_sParam.dStep1WheelAngle;
    m_dStep1Yn      = m_sParam.dStep1Yn;
    if(dTheta == 0){
        Log::err(ClassName,"GetStep1Xn , WheelAngle = 0");
        return false;
    }
    if(dTheta > m_sParam.dMaxWheelAngle){
        Log::err(ClassName,"GetStep1Xn , WheelAngle(%.2f) > %.2f",dTheta,m_sParam.dMaxWheelAngle);
        return false;
    }
    edouble dTmp1   = 2*(m_dYn-m_dStep1Yn)*m_sParam.dWheelL/tan(m_sParam.dStep1WheelAngle);
    edouble dTmp2   = pow(m_dXn, 2) + pow(m_dYn, 2) - pow(m_dStep1Yn, 2);

    if( (dTmp1+dTmp2) < 0 ){
        Log::err(ClassName,"GetStep1Xn , can not reach");
        return false;
    }
    m_dStep1Xn        = sqrt(dTmp1+dTmp2);
    return true;
}

// 获取机器人位置数据
ebool Approach::GetRobotData()
{
    ebool               bResult         = false;
    edouble             dTime           = 0;
    CLocalizationMsg    iLocalization;
    if(false == GetMsgData(&iLocalization))
    {
        Log::err(ClassName,"get vps location failed");
    }else {
        m_sRobot.dTime                  = iLocalization.m_dTimeStamp;
        m_sRobot.dX                     = iLocalization.m_iCurrentPose.m_iPosition.m_dx;
        m_sRobot.dY                     = iLocalization.m_iCurrentPose.m_iPosition.m_dy;
        m_sRobot.dRz                    = iLocalization.m_iCurrentPose.m_iRotation.m_dRz;
        dTime                           = Time::GetCurrentTime() - m_sRobot.dTime;
        if(fabs(dTime) < m_sParam.dRobotPosValidTime)
        {
            Log::inf(ClassName,"robot pos,x=%.2f,y=%.2f,yaw=%.2f,dt=%.2f",m_sRobot.dX,m_sRobot.dY,m_sRobot.dRz,dTime);
            bResult                     = true;
        }else {
            Log::err(ClassName,"vps location too old, dt = %.2f",dTime);
        }
        TestUi::Ins()->SaveRobotPose(iLocalization.m_iCurrentPose);
    }
    return bResult;
}

// 获取后轮角度数据
ebool Approach::GetWheelData()
{
    ebool               bResult         = false;
    edouble             dTime           = 0;
    CJointStateMsg      iWheelState;
    if(false == GetWheelStateMsg(&iWheelState))
    {
        Log::err(ClassName,"get wheel state failed");
    }else{
        m_sWheel.dTime                  = iWheelState.m_dTimeStamp;
        m_sWheel.dAngle                 = iWheelState.m_viJointList[2].m_dPosition;
        dTime                           = Time::GetCurrentTime() - m_sWheel.dTime;
        if(fabs(dTime) < m_sParam.dWheelAngleValidTime)
        {
            bResult                     = true;
        }else {
            Log::err(ClassName,"wheel state too old,dt=%.2f",dTime);
        }
    }
    return bResult;
}

// 获取雷达中纸卷数据
ebool Approach::GetPaperData(edouble &dPaperX, edouble &dPaperY)
{
    ebool                   bResult     = false;
    CPaperRollMsg           iMsg;
    vector<PaperData>       vPapers;
    RobotData               sRobot      = m_sRobot;
    edouble                 dNow        = Time::GetCurrentTime();
    edouble                 dTime       = 0;
    eint                    nCount      = 0;
    if (false == GetMsgData(&iMsg))
    {
        Log::inf(ClassName,"get paper roll failed");
        return bResult;
    }
    nCount                              = iMsg.m_vPaperRollList.size();
    dTime                               = dNow - iMsg.m_dTimeStamp;
    for(int i=0;i<nCount;i++)
    {
        const CPaperRoll   &iPaperRoll  = iMsg.m_vPaperRollList[i];
        PaperData           sPaper;
        sPaper.bValid     = true;
        sPaper.dTime      = iMsg.m_dTimeStamp;
        sPaper.dRadius    = iPaperRoll.m_fDiameter;
        sPaper.dX         = iPaperRoll.m_iPose.m_iPosition.m_dx;
        sPaper.dY         = iPaperRoll.m_iPose.m_iPosition.m_dy;
        sPaper.dMapX      = sRobot.dX + sPaper.dX*cos(sRobot.dRz) - sPaper.dY*sin(sRobot.dRz);
        sPaper.dMapY      = sRobot.dY + sPaper.dX*sin(sRobot.dRz) + sPaper.dY*cos(sRobot.dRz);
        Log::inf(ClassName,"paper[%d]:x=%.2f,y=%.2f,mapX=%.2f,mapY=%.2f,radius=%.02f,dt=%.3f",
                 i,sPaper.dX,sPaper.dY,sPaper.dMapX,sPaper.dMapY,sPaper.dRadius,dTime);
        if(iPaperRoll.m_iPose.m_iPosition.m_dx < m_sParam.dPaperMinRelativeX) continue;
        if(fabs(iPaperRoll.m_fDiameter - m_dApproachPaperRadius) > m_sParam.dPaperRadiusOffset) continue;
        if(fabs(dTime) > m_sParam.dPaperValidTime) continue;
        vPapers.push_back(sPaper);
    }
    eint     nIndex         = -1;
    efloat   fDist          = 1e7;
    nCount                  = vPapers.size();
    if(false == m_sPaper.bValid)
    {
        for(int i=0;i<nCount;i++)
        {
            const PaperData &sRoll  = vPapers[i];
            efloat  fTmp            = sqrt(pow(sRoll.dX,2) + pow(sRoll.dY,2));
            if(fTmp < fDist)
            {
                fDist               = fTmp;
                nIndex              = i;
            }
        }
    }else{
        for(int i=0;i<nCount;i++)
        {
            const PaperData &sRoll  = vPapers[i];
            efloat  fTmp            = sqrt(pow(sRoll.dMapX-m_sPaper.dMapX,2) + pow(sRoll.dMapY-m_sPaper.dMapY,2));
            if( (fTmp < m_sParam.dPaperAllowedDistance) && (fTmp < fDist) )
            {
                nIndex              = i;
                fDist               = fTmp;
            }
        }
    }
    if( -1 != nIndex )
    {
        m_sPaper            = vPapers[nIndex];
    }
    if(m_sPaper.bValid)
    {
        dTime               = dNow - m_sPaper.dTime;
        if(fabs(dTime) < m_sParam.dPaperValidTime)
        {
            edouble dDeltaX = m_sPaper.dMapX - sRobot.dX;
            edouble dDeltaY = m_sPaper.dMapY - sRobot.dY;
            dPaperX         = dDeltaX*cos(sRobot.dRz)  + dDeltaY*sin(sRobot.dRz);
            dPaperY         = -dDeltaX*sin(sRobot.dRz) + dDeltaY*cos(sRobot.dRz);
            Log::inf(ClassName,"paperFromMap:x=%.2f,y=%.2f,radius=%.02f,dt=%.2f",dPaperX,dPaperY,m_sPaper.dRadius,dTime);
        }else{
            dPaperX         = m_sPaper.dX;
            dPaperY         = m_sPaper.dY;
            Log::inf(ClassName,"paperFromLidar:x=%.2f,y=%.2f,radius=%.02f,dt=%.2f",dPaperX,dPaperY,m_sPaper.dRadius,dTime);
        }
        bResult             = true;
        // ui显示
        CPose iPaperInLidar;
        iPaperInLidar.m_iPosition.m_dx  = dPaperX;
        iPaperInLidar.m_iPosition.m_dy  = dPaperY;
        TestUi::Ins()->SavePaperRadius(m_dApproachPaperRadius);
        TestUi::Ins()->SavePaperInLidar(iPaperInLidar);
    }
    TestUi::Ins()->Show();
    return bResult;
}

// 计算要发送的速度和轮子角度
eint Approach::CalculateSpeed(edouble dXn, edouble dYn, edouble dPaperX, edouble dPaperY, edouble &dSpeed, edouble &dWheelAngle, ebool &bStop)
{
    eint        nResult             = ERR_NONE;
    edouble     dLinearSpeed        = 0;
    edouble     dDistance           = sqrt(pow(dXn-dPaperX,2) + pow(dYn-dPaperY,2));
    bStop                           = false;
    dSpeed                          = 0;
    dWheelAngle                     = 0;

    Log::inf(ClassName,"calc speed,Xn=%.2f,Yn=%.2f,paperX=%.2f,paperY=%.2f,dist=%.2f",dXn,dYn,dPaperX,dPaperY,dDistance);

    if ((dDistance < m_sParam.dDistanceToStop) || (dXn >= dPaperX))
    {
        dLinearSpeed                = 0.0;
        bStop                       = true;
        Log::inf(ClassName,"cal speed end,dist=%.2f < %.2f",dDistance,m_sParam.dDistanceToStop);
        return nResult;
    }
    // 两点的中垂线与y轴的交点（转动圆心位置)，中垂线方程为y=kx+b,k=-(x1-x2)/(y1-y2),b=(x1-x2)(x1+x2)/(y1-y2)/2+(y1+y2)/2;
    edouble dB                      = (dXn-dPaperX)*(dXn+dPaperX)/(dYn-dPaperY)/2+(dYn+dPaperY)/2;
    dWheelAngle                     = -atan(m_sParam.dWheelL/dB);
    if(fabs(dWheelAngle) > m_sParam.dMaxWheelAngle)
    {
        Log::err(ClassName,"calc speed err,wheel angle=%.2f>%.2f(max)",dWheelAngle,m_sParam.dMaxWheelAngle);
        bStop                       = true;
        nResult                     = PLANNING_FAIL;
        return nResult;
    }
    if (dDistance < m_sParam.dDistanceToSlow)
    {
        dLinearSpeed                = MAX(m_sParam.dMinSpeed,m_sParam.dMaxSpeed*dDistance/m_sParam.dDistanceToSlow);
    }else{
        dLinearSpeed                = m_sParam.dMaxSpeed;
    }
    edouble dAngleDiff              = dWheelAngle-m_sWheel.dAngle;
    if(fabs(dAngleDiff) > m_sParam.dWheelAngleAllowedDiff)
    {
        Log::inf(ClassName,"calc speed,angleDiff=%.2f > %.2f,speed=0",dAngleDiff,m_sParam.dWheelAngleAllowedDiff);
        dLinearSpeed                = 0;
    }

    // 限制速度
    efloat fTmp                     = 0.1;
    if(dLinearSpeed > fTmp  )dLinearSpeed = fTmp;
    if(dLinearSpeed < -fTmp )dLinearSpeed = -fTmp;
    //    dLinearSpeed    = 0;
    //    dWheelAngle     = 0;

    dSpeed                          = dLinearSpeed/m_sParam.dWheelRadius;
    Log::inf(ClassName,"calc speed,linear speed=%.2f,angle=%.2f(current:%.2f,diff:%.2f,maxDiff:%.2f)",
             dLinearSpeed,dWheelAngle,m_sWheel.dAngle,dAngleDiff,m_sParam.dWheelAngleAllowedDiff);
    return nResult;
}

eint Approach::ApproachStep1()
{
    eint    nErr        = ERR_NONE;
    ebool   bStop       = false;
    euint   unCmdSN     = 0;
    edouble dPaperX     = 0;
    edouble dPaperY     = 0;
    edouble dSpeed      = 0;
    edouble dWheelAngle = 0;
    ebool   bRet        = this->GetRobotData() && this->GetWheelData() && this->GetPaperData(dPaperX,dPaperY);
    if(!bRet)
    {
        BaseSteerCtrl(dSpeed,dWheelAngle,unCmdSN);
        nErr            = LOCALIZATION_LOST;
        return nErr;
    }
    nErr                = this->CalculateSpeed(m_dStep1Xn,m_dStep1Yn,dPaperX,dPaperY,dSpeed,dWheelAngle,bStop);
    if(ERR_NONE != nErr)
    {
        ApproachEnd(nErr);
    }else if(bStop){
        m_bStep1Finished    = true;
        Log::inf(ClassName,"approach step1 finished");
        BaseSteerCtrl(dSpeed,dWheelAngle,unCmdSN);
    }else{
        BaseSteerCtrl(dSpeed,dWheelAngle,unCmdSN);
    }
    return nErr;
}

eint Approach::ApproachStep2()
{
    eint    nErr        = ERR_NONE;
    ebool   bStop       = false;
    euint   unCmdSN     = 0;
    edouble dPaperX     = 0;
    edouble dPaperY     = 0;
    edouble dSpeed      = 0;
    edouble dWheelAngle = 0;
    ebool   bRet        = this->GetRobotData() && this->GetWheelData() && this->GetPaperData(dPaperX,dPaperY);
    if(!bRet)
    {
        BaseSteerCtrl(dSpeed,dWheelAngle,unCmdSN);
        nErr            = LOCALIZATION_LOST;
        return nErr;
    }
    nErr                = this->CalculateSpeed(m_dXn,m_dYn,dPaperX,dPaperY,dSpeed,dWheelAngle,bStop);
    if(ERR_NONE != nErr)
    {
        ApproachEnd(nErr);
    }else if(bStop){
        ApproachEnd(nErr);
    }else{
        BaseSteerCtrl(dSpeed,dWheelAngle,unCmdSN);
    }
    return nErr;
}

eint Approach::ApproachEnd(eint nErr)
{
    euint unCmdSN                   = 0;
    std::string strParamStr;
    std::vector<CFunParamVal> vParamList;
    string str                      = to_string(nErr);
    Log::inf(ClassName,"approach finished,err=%d\n",nErr);
    BaseStop(1,unCmdSN);
    vParamList.resize(1);
    vParamList[0].m_strParamName    = "RetCode";
    vParamList[0].m_strParamValue   = str;
    SetParamStr(vParamList,strParamStr);
    SendNotifyToParent(strParamStr);
    FunQuit(nErr);
    return ERR_NONE;
}

void Approach::GetRobotPosInRealis(const Eigen::Matrix4d &iMatRobotInMark, const CPose &iMarkInRealis, CPosition &iRobotPosInRealis)
{
    Translation3d   iTransMarkInRealis(iMarkInRealis.m_iPosition.m_dx,iMarkInRealis.m_iPosition.m_dy,iMarkInRealis.m_iPosition.m_dz);
    AngleAxisd      iRx(iMarkInRealis.m_iRotation.m_dRx, Eigen::Vector3d::UnitX());
    AngleAxisd      iRy(iMarkInRealis.m_iRotation.m_dRy, Eigen::Vector3d::UnitY());
    AngleAxisd      iRz(iMarkInRealis.m_iRotation.m_dRz, Eigen::Vector3d::UnitZ());
    // mark坐标系到realis坐标系的变换矩阵，顺序z,y,x，矩阵不满足乘法交换律
    Affine3d        iAffMarkInRealis  = iTransMarkInRealis*iRz*iRy*iRx;
    // robot坐标系到maker坐标系的变换矩阵
    Affine3d        iAffRobotInMark(iMatRobotInMark);
    // robot坐标系到realis坐标系的变换矩阵
    Affine3d        iAffRobotInRealis   = iAffMarkInRealis * iAffRobotInMark;
    // 机器人原点在realis中的x，y，z
    iRobotPosInRealis.m_dx                 = iAffRobotInRealis.translation().x();
    iRobotPosInRealis.m_dy                 = iAffRobotInRealis.translation().y();
    iRobotPosInRealis.m_dz                 = iAffRobotInRealis.translation().z();
}

void Approach::GetPaperPosInRobot(const CPosition &iPaperPosInLidar, const CPose &iRobotInRealis, CPosition &iPaperPosInRealis)
{
    Translation3d   iTransMarkInRealis(iRobotInRealis.m_iPosition.m_dx,iRobotInRealis.m_iPosition.m_dy,iRobotInRealis.m_iPosition.m_dz);
    AngleAxisd      iRx(iRobotInRealis.m_iRotation.m_dRx, Eigen::Vector3d::UnitX());
    AngleAxisd      iRy(iRobotInRealis.m_iRotation.m_dRy, Eigen::Vector3d::UnitY());
    AngleAxisd      iRz(iRobotInRealis.m_iRotation.m_dRz, Eigen::Vector3d::UnitZ());
    // 机器人坐标系到realis坐标系的变换矩阵，顺序z,y,x，矩阵不满足乘法交换律
    Affine3d        iAffRobotInRealis  = iTransMarkInRealis*iRz*iRy*iRx;
    Vector4d        iResult            = iAffRobotInRealis.inverse()*Vector4d(iPaperPosInLidar.m_dx,iPaperPosInLidar.m_dy,iPaperPosInLidar.m_dz,1);
    iPaperPosInRealis.m_dx             = iResult(0);
    iPaperPosInRealis.m_dy             = iResult(1);
    iPaperPosInRealis.m_dz             = iResult(2);
}

void Approach::TestMatrix()
{
    CPose               iFittingResult;
    edouble             dArr[4][4]          = {{0.9999,-0.0130,0,1.4194},
                                               {0.0130,0.9990,0,0.0206},
                                               {0,0,1.0000,-1.9824},
                                               {0,0,0,1.0000}};

    Matrix4d    iMat;
    iMat <<     0.9999,-0.0130,0,1.4194,
            0.0130,0.9990,0,0.0206,
            0,0,1.0000,-1.9824,
            0,0,0,1.0000;


    CTransformationM    iM;
    CPose               iRobotInRealis,iPaperInRealis,iMarkInRealis,iRobotInTag,iPaperInRobot,iFittingResultInRealis;

    memcpy(iM.m_vdMat,dArr,sizeof (dArr));
    CCSysConv::TransformationMatrixToPose(iM,iRobotInTag);

    iMarkInRealis.m_iPosition.m_dx   = -1.391;
    iMarkInRealis.m_iPosition.m_dy   = -0.087;
    iMarkInRealis.m_iRotation.m_dRz  = -0.027;

    iPaperInRealis.m_iPosition.m_dx   = 4.677;
    iPaperInRealis.m_iPosition.m_dy   = 0.284;

    CCSysConv::CoordinateTransform2(iRobotInTag,iMarkInRealis,iRobotInRealis);
    Log::inf(ClassName,"markInRealis :x=%.3f,y=%.3f,z=%.3f,dRz=%.3f",
             iMarkInRealis.m_iPosition.m_dx,iMarkInRealis.m_iPosition.m_dy,iMarkInRealis.m_iPosition.m_dz,iMarkInRealis.m_iRotation.m_dRz);
    Log::inf(ClassName,"robotInRealis:x=%.3f,y=%.3f,z=%.3f,dRz=%.3f",
             iRobotInRealis.m_iPosition.m_dx,iRobotInRealis.m_iPosition.m_dy,iRobotInRealis.m_iPosition.m_dz,iRobotInRealis.m_iRotation.m_dRz);
    CCSysConv::CoordinateTransform(iPaperInRealis,iRobotInRealis,iPaperInRobot);
    Log::inf(ClassName,"paperInRealis:x=%.3f,y=%.3f,paperInRobot:x=%.3f,y=%.3f",
             iPaperInRealis.m_iPosition.m_dx,iPaperInRealis.m_iPosition.m_dy,iPaperInRobot.m_iPosition.m_dx,iPaperInRobot.m_iPosition.m_dy);


    // ---------------------------my----------------------------------
    Log::inf(ClassName,"-----------------------------------------------------");
    this->GetRobotPosInRealis(iMat,iMarkInRealis,iRobotInRealis.m_iPosition);
    // 机器人的航向角取Mark的，而不是机器人原点在realis下的欧拉角,机器人原点相对于Mark只是平移关系
    iRobotInRealis.m_iRotation          = iMarkInRealis.m_iRotation;
    // 根据paper在realis中位置以及机器人在realis中位置以及机器人的航向角，计算paper在机器人中位置
    this->GetPaperPosInRobot(iPaperInRealis.m_iPosition,iRobotInRealis,iPaperInRobot.m_iPosition);

    Log::inf(ClassName,"robotInRealis:x=%.3f,y=%.3f,z=%.3f,dRz=%.3f",
             iRobotInRealis.m_iPosition.m_dx,iRobotInRealis.m_iPosition.m_dy,iRobotInRealis.m_iPosition.m_dz,iRobotInRealis.m_iRotation.m_dRz);
    Log::inf(ClassName,"paperInRealis:x=%.3f,y=%.3f,paperInRobot:x=%.3f,y=%.3f",
             iPaperInRealis.m_iPosition.m_dx,iPaperInRealis.m_iPosition.m_dy,iPaperInRobot.m_iPosition.m_dx,iPaperInRobot.m_iPosition.m_dy);
}






