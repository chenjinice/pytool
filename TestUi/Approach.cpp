#include "Approach.h"
#include <EwayFunc/FuncSdkLib.h>
#include <EwayMath/BasicMath.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>
#include <EwayMath/CoordinateConversion.h>

#include "TestUi.h"


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


static const echar *        ClassName           = "Approach";
// 左右轮子间距,米
static const efloat         WheelD              = 1.05;
// 前后轮间距,米
static const efloat         WheelL              = 1.75;
// 前轮周长,米
static const efloat         WheelC              = 2.136;
// 前轮半径,米
static const efloat         WheelR              = WheelC/(2*EPI);
// 叉子固定臂的x,y,米
static const efloat         ForkX1              = 1.280;
static const efloat         ForkY1              = 0.752;
// 叉子可移动臂的关节点x，y和长度(米)
static const efloat         ForkX2              = 0.663;
static const efloat         ForkY2              = -0.244;
static const efloat         ForkR2              = 1.047;

// 纸卷在机器人x轴多少米以内时，不用相对位置
static const efloat         RollMinX            = 3.0;
// 纸卷半径允许最大差距,米
static const efloat         RadiusRange         = 0.03;
// 缓存的纸卷位置相差多远算作同一个，米
static const efloat         TrackDistRange      = 1.0;
// 缓存的纸卷数据，在这个数据以内取相对位置，超过这个时间取缓存的全局位置，秒
static const efloat         TrackTimeRange      = 0.2;

// 轮子角度数据，最大允许多久之前的，秒
static const efloat         WheelTimeRange      = 0.4;
// 轮子当前角度与要设置的角度差距多大时，速度设为0，等它转过来，弧度
static const efloat         WheelAngleRange     = 3/180.0*EPI;
// 轮子最大允许转动角度，弧度
static const efloat         MaxSteerAngle       = 40.0/180.0*EPI;
// 根据轮子转弯的度数和步骤1的Yn，算出步骤1的Xn
// 主要是为了防止随意设置的Xn和Yn车到不了
static const efloat         ThetaStep1          = 30;
static const efloat         YnStep1             = 0;


Approach::Approach()
{
//    this->ShowGui();
//    this->TestLidar();
}

Approach::~Approach()
{
}

eint Approach::Initialize(std::vector<eint> &vFeatureList,ebool &bLimbAuthority,edouble &dTimeOut)
{
    vFeatureList.clear();
//    vFeatureList.push_back(Message_Code_PaperRollDect);
    vFeatureList.push_back(Message_Code_Slam_Localization);
    //vFeatureList.push_back(Message_Code_WheelState);
//    vFeatureList.push_back(Message_Code_CalibData);

    bLimbAuthority          = true;
    dTimeOut                = 3;

    this->GetXnYn(m_fTargetRadius,m_fXn,m_fYn);
    this->GetXnYnStep1(ThetaStep1);

    Log::inf(ClassName,"        xn=%.3f,yn=%.3f",m_fXn,m_fYn);
    Log::inf(ClassName,"step1 , xn=%.3f,yn=%.3f",m_fXnStep1,m_fYnStep1);
    return ERR_NONE;
}

eint Approach::Destroy()
{
    return ERR_NONE;
}

eint Approach::CheckMsgCode(eint nMsgCode)
{
    UNUSED(nMsgCode);
    eint ret    = MsgProcCode_Record;
    if(Message_Code_FeatureStatus == nMsgCode)
    {
        ret     = MsgProcCode_Notify;
    }
    return ret;
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
    if (false == m_bMotionReady)
    {
        eint    nErrorCode  = 0;
        euint   unCmdSN     = 0;
        // 前进档
        eint    mode        = CSetModeCmd::DriveMode_Drive;
        //                mode        = CSetModeCmd::DriveMode_Reverse;
        nErrorCode          = BaseSetDriveMode(mode,unCmdSN);
        if(ERR_NONE == nErrorCode)
        {
            m_bMotionReady  = true;
        }
    }

//    if(false == m_bStep1Finished)
//    {
//        this->ApproachStep1();
//    }else {
//        this->ApproachStep2();
//    }
//    this->ApproachStep2();

//    this->TestLidar();

//    this->ShowGui();

    this->Test();

    static int i = 0;
    CPose p , q , r,robot;
    robot.m_iPosition.m_dx  = i*0.001;
    robot.m_iPosition.m_dy  = i*0.002;
    robot.m_iRotation.m_dRz = i*0.1;
    p.m_iPosition.m_dx  = i*0.01;
    p.m_iPosition.m_dy  = i*0.02;
    p.m_iRotation.m_dRz = i*0.2;
    q.m_iPosition.m_dx  = 2.0;
    q.m_iPosition.m_dy  = 2.0;
    r.m_iPosition.m_dx  = 0.5 + i*0.01;
    r.m_iPosition.m_dy  = 0.2 + i*0.005;

    echar strText[200];
    sprintf(strText,"dx=%.3f,dy=%.3f,dt=%.3f",q.m_iPosition.m_dx-r.m_iPosition.m_dx,q.m_iPosition.m_dy-r.m_iPosition.m_dy,0.1);
    TestUi::Ins()->SaveText(strText);
    sprintf(strText,"dx=%.2f,dy=%.2f,dt=%.2f",q.m_iPosition.m_dx-r.m_iPosition.m_dx,q.m_iPosition.m_dy-r.m_iPosition.m_dy,0.1);
    TestUi::Ins()->SaveText(strText);
    TestUi::Ins()->SaveXnYn(m_fXn,m_fYn);
    TestUi::Ins()->SaveStep1XnYn(1,2);
    TestUi::Ins()->SaveRobot(robot);
    TestUi::Ins()->SavePaperInLidar(p,0.5);
    TestUi::Ins()->SavePaperInLidar(r,1);
    TestUi::Ins()->SavePaperInRealis(q,1);
    TestUi::Ins()->Show();
    i++;
    return ERR_NONE;
}

eint Approach::ProcFeatureStatus(edouble m_dTimeStamp, CFeatureStatusMsg *piMsg)
{
    UNUSED(m_dTimeStamp);
    UNUSED(piMsg);
    return ERR_NONE;
}

eint Approach::ApproachEnd(ebool flag)
{
    euint unCmdSN                   = 0;
    std::string strParamStr;
    std::vector<CFunParamVal> vParamList;
    string str                      = "1";
    if(false == flag)
    {
        str = "0";
    }
    Log::inf("TestApproach","ApproachEnd , flag = %d\n",flag);
    BaseStop(1,unCmdSN);
    vParamList.resize(1);
    vParamList[0].m_strParamName    = "RetCode";
    vParamList[0].m_strParamValue   = str;
    SetParamStr(vParamList,strParamStr);
    SendNotifyToParent(strParamStr);
    FunQuit(0);
    return ERR_NONE;
}

eint Approach::TrackPaperRoll(const CLocalizationMsg &iLocalization)
{
    CPaperRollMsg           iMsg;
    vector<CurPaperRoll>    vPaperRolls;
    CPose                   iPos        = iLocalization.m_iCurrentPose;
    edouble                 dTheta      = iPos.m_iRotation.m_dRz;
    edouble                 dNow        = Time::GetCurrentTime();
    eint                    nCount      = 0;

    Log::inf(ClassName,"robot pos,x=%.2f,y=%.2f,yaw=%.2f",iPos.m_iPosition.m_dx,iPos.m_iPosition.m_dy,dTheta);
    if (false == GetMsgData(&iMsg))
    {
        Log::inf(ClassName,"get paper roll failed\n");
        return ERR_NONE;
    }
    nCount                              = iMsg.m_vPaperRollList.size();
    for(int i=0;i<nCount;i++)
    {
        const CPaperRoll   &iPaperRoll  = iMsg.m_vPaperRollList[i];
        edouble             dt          = dNow - iMsg.m_dTimeStamp;
        CurPaperRoll        sData;
        sData.valid     = true;
        sData.time      = iMsg.m_dTimeStamp;
        sData.diameter  = iPaperRoll.m_fDiameter;
        sData.x         = iPaperRoll.m_iPose.m_iPosition.m_dx;
        sData.y         = iPaperRoll.m_iPose.m_iPosition.m_dy;
        sData.mapX      = iPos.m_iPosition.m_dx + sData.x*cos(dTheta) - sData.y*sin(dTheta);
        sData.mapY      = iPos.m_iPosition.m_dy + sData.x*sin(dTheta) + sData.y*cos(dTheta);

        Log::inf(ClassName,"roll[%d]:x=%.2f,y=%.2f,mapX=%.2f,mapY=%.2f,dia=%.02f,dt=%.3f",
                 i,sData.x,sData.y,sData.mapX,sData.mapY,sData.diameter,dt);

        if(iPaperRoll.m_iPose.m_iPosition.m_dx < RollMinX) continue;
        if(fabs(iPaperRoll.m_fDiameter - m_fTargetRadius) > RadiusRange) continue;
        if(fabs(dt) > TrackTimeRange) continue;
        vPaperRolls.push_back(sData);
    }
    eint     nIndex         = -1;
    efloat   fDist          = 1e7;
    nCount                  = vPaperRolls.size();
    if(false == m_sRoll.valid)
    {
        for(int i=0;i<nCount;i++)
        {
            const CurPaperRoll &sRoll = vPaperRolls[i];
            efloat  fTmp    = sqrt(pow(sRoll.x,2) + pow(sRoll.y,2));
            if(fTmp < fDist)
            {
                fDist       = fTmp;
                nIndex      = i;
            }
        }
    }else{
        for(int i=0;i<nCount;i++)
        {
            const CurPaperRoll &sRoll = vPaperRolls[i];
            efloat  fTmp    = sqrt(pow(sRoll.mapX-m_sRoll.mapX,2) + pow(sRoll.mapY-m_sRoll.mapY,2));
            if( (fTmp < TrackDistRange) && (fTmp < fDist) )
            {
                nIndex      = i;
                fDist       = fTmp;
            }
        }
    }
    if( -1 != nIndex )
    {
        m_sRoll = vPaperRolls[nIndex];
    }
    if(m_sRoll.valid)
    {
        Log::inf(ClassName,"roll   :x=%.2f,y=%.2f,mapX=%.2f,mapY=%.2f,dia=%.02f,dt=%.3f",
                 m_sRoll.x,m_sRoll.y,m_sRoll.mapX,m_sRoll.mapY,m_sRoll.diameter,(dNow-m_sRoll.time));
    }
    return ERR_NONE;
}

bool Approach::GetWheelState()
{
    bool                ret             = false;
    CJointStateMsg      iWheelState;

    if(false == GetWheelStateMsg(&iWheelState))
    {
        Log::err(ClassName,"get wheel state failed");
    }else{
        edouble     dNow                = Time::GetCurrentTime();
        edouble     dt                  = dNow - iWheelState.m_dTimeStamp;
        if(fabs(dt) < WheelTimeRange)
        {
            m_fWheelAngle               = iWheelState.m_viJointList[2].m_dPosition;
            ret                         = true;
        }else {
            Log::err(ClassName,"wheel state too old, dt = %.3f",dt);
        }
    }
    return ret;
}

eint Approach::GetXnYn(efloat radius, efloat &fXn, efloat &fYn)
{
    efloat fR1  = radius * 2;
    efloat fD   = sqrt(pow(ForkX1 - ForkX2, 2) + pow(ForkY1 - ForkY2, 2));
    if(fD == 0)
    {
        Log::err(ClassName,"GetXnYn ,fD = 0");
        ApproachEnd(false);
        return INIT_FAILED;
    }
    efloat fA   = fD/2.0 + (pow(fR1, 2) - pow(ForkR2, 2))/(2.0 * fD);
    if(fabs(fR1) < fabs(fA) )
    {
        Log::err(ClassName,"GetXnYn ,fR1(%f) < fA(%f)",fR1,fA);
        ApproachEnd(false);
        return INIT_FAILED;
    }
    efloat fH   = sqrt(pow(fR1, 2) - pow(fA, 2));
    fXn         = ForkX1 + fA/(2 * fD) * (ForkX2 - ForkX1) + fH/(2 * fD) * (ForkY1 - ForkY2);
    fYn         = ForkY1 + fA/(2 * fD) * (ForkY2 - ForkY1) - fH/(2 * fD) * (ForkX1 - ForkX2);
    return ERR_NONE;
}

eint Approach::GetXnYnStep1(efloat fTheta)
{
    m_fYnStep1  = YnStep1;
    efloat rad  = fTheta/180.0*EPI;
    efloat tmp1 = 2*(m_fYn-m_fYnStep1)*WheelL/tan(rad);
    efloat tmp2 = pow(m_fXn, 2) + pow(m_fYn, 2) - pow(m_fYnStep1, 2);
    m_fXnStep1  = sqrt(tmp1+tmp2);

    return ERR_NONE;
}

bool Approach::GetSpeed(efloat fXn,efloat fYn,efloat fRollX, efloat fRollY, efloat &fSpeed, efloat &fSteerAngle)
{
    ebool   bStop                   = false;
    efloat  fMaxSpeed               = 0.5;
    efloat  fDistToSlow             = 2.5;
    efloat  fDistToStop             = 0.03;
    efloat  fSpeedMin               = 0.05;
    efloat  fLinearSpeed            = fMaxSpeed;
    efloat  fDist                   = sqrt(pow(fXn-fRollX,2) + pow(fYn-fRollY,2));
    efloat  fSquaredError           = pow(fXn, 2) + pow(fYn, 2) - pow(fRollX, 2) - pow(fRollY, 2);

    fSteerAngle                     = atan(-2*(fYn-fRollY)*WheelL/fSquaredError);
    if (fSteerAngle > MaxSteerAngle)
    {
        fSteerAngle                 = MaxSteerAngle;
    }
    if (fSteerAngle < -MaxSteerAngle)
    {
        fSteerAngle                 = -MaxSteerAngle;
    }

    if (fDist < fDistToSlow)
    {
        fLinearSpeed                = MAX(fSpeedMin,fMaxSpeed*fDist/fDistToSlow);
    }else{
        fLinearSpeed                = fMaxSpeed;
    }
    if ((fDist < fDistToStop) || (fXn > fRollX))
    {
        fLinearSpeed                = 0.0;
        bStop                       = true;
    }
    if(fabs(fSteerAngle-m_fWheelAngle) > WheelAngleRange)
    {
        fLinearSpeed                = 0;
    }

    // 限制速度
    efloat fTmp                     = 0.1;
    if(fLinearSpeed > fTmp  )fLinearSpeed = fTmp;
    if(fLinearSpeed < -fTmp )fLinearSpeed = -fTmp;

    fSpeed                          = fLinearSpeed/WheelR;
    Log::inf(ClassName,"xn=%.2f,yn=%.2f,rollx=%.2f,rolly=%.2f,dist=%.2f,speed=%.2f,angle=%.2f(%.2f)",
             fXn,fYn,fRollX,fRollY,fDist,fLinearSpeed,fSteerAngle,m_fWheelAngle);
    return bStop;
}

eint Approach::ApproachStep1()
{
    euint               unCmdSN         = 0;
    efloat              fRollX          = 0;
    efloat              fRollY          = 0;
    efloat              fSpeed          = 0;
    efloat              fSteerAngle     = 0;
    CLocalizationMsg    iLocalization;
    if(false == GetMsgData(&iLocalization))
    {
        Log::err(ClassName,"get location failed");
        ApproachEnd(false);
        return LOCALIZATION_LOST;
    }
    if(false == this->GetWheelState())
    {
        ApproachEnd(false);
        return LOCALIZATION_LOST;
    }
    this->TrackPaperRoll(iLocalization);
    if(false == m_sRoll.valid)
    {
        return ERR_NONE;
    }
    if(fabs(Time::GetCurrentTime() - m_sRoll.time) < TrackTimeRange)
    {
        fRollX              = m_sRoll.x;
        fRollY              = m_sRoll.y;
    }else{
        efloat fDeltaX      = m_sRoll.mapX - iLocalization.m_iCurrentPose.m_iPosition.m_dx;
        efloat fDeltaY      = m_sRoll.mapY - iLocalization.m_iCurrentPose.m_iPosition.m_dy;
        efloat fTheta       = iLocalization.m_iCurrentPose.m_iRotation.m_dRz;
        fRollX              = fDeltaX*cos(fTheta)  + fDeltaY*sin(fTheta);
        fRollY              = -fDeltaX*sin(fTheta) + fDeltaY*cos(fTheta);
    }
    if(this->GetSpeed(m_fXnStep1,m_fYnStep1,fRollX,fRollY,fSpeed,fSteerAngle))
    {
        m_bStep1Finished    = true;
        Log::inf(ClassName,"step1 finished");
        BaseSteerCtrl(fSpeed,fSteerAngle,unCmdSN);
    }else {
        BaseSteerCtrl(fSpeed,fSteerAngle,unCmdSN);
    }
    return ERR_NONE;
}

eint Approach::ApproachStep2()
{
    euint               unCmdSN         = 0;
    efloat              fRollX          = 0;
    efloat              fRollY          = 0;
    efloat              fSpeed          = 0;
    efloat              fSteerAngle     = 0;
    CLocalizationMsg    iLocalization;
    if(false == GetMsgData(&iLocalization))
    {
        Log::err(ClassName,"get location failed");
        ApproachEnd(false);
        return LOCALIZATION_LOST;
    }
    if(false == this->GetWheelState())
    {
        ApproachEnd(false);
        return LOCALIZATION_LOST;
    }
    this->TrackPaperRoll(iLocalization);
    if(false == m_sRoll.valid)
    {
        return ERR_NONE;
    }
    if(fabs(Time::GetCurrentTime() - m_sRoll.time) < TrackTimeRange)
    {
        fRollX              = m_sRoll.x;
        fRollY              = m_sRoll.y;
    }else{
        efloat fDeltaX      = m_sRoll.mapX - iLocalization.m_iCurrentPose.m_iPosition.m_dx;
        efloat fDeltaY      = m_sRoll.mapY - iLocalization.m_iCurrentPose.m_iPosition.m_dy;
        efloat fTheta       = iLocalization.m_iCurrentPose.m_iRotation.m_dRz;
        fRollX              = fDeltaX*cos(fTheta)  + fDeltaY*sin(fTheta);
        fRollY              = -fDeltaX*sin(fTheta) + fDeltaY*cos(fTheta);
    }
    if(this->GetSpeed(m_fXn,m_fYn,fRollX,fRollY,fSpeed,fSteerAngle))
    {
        ApproachEnd(true);
    }else {
        BaseSteerCtrl(fSpeed,fSteerAngle,unCmdSN);
    }
    return ERR_NONE;
}


void Approach::ShowGui()
{
    const std::string name = "image";
    cv::namedWindow(name, cv::WINDOW_AUTOSIZE);
    int w = 500, h = 500;
    static int y = 0;

    y+=3;
//    // 白图黑线
//    cv::Mat img1(h, w, CV_8UC1, cv::Scalar(255,255,255)); // cv::Mat img = cv::Mat::zeros(w, h, CV_8UC1);
//    cv::line(img1, cv::Point(0, y), cv::Point(img1.cols, y), cv::Scalar(255), 2);

//    // 白图黑线
//    cv::Mat img2(h, w, CV_8UC1, cv::Scalar(255)); // cv::Mat img = cv::Mat::ones(w, h, CV_8UC1) * 255;
//    cv::line(img2, cv::Point(0, y), cv::Point(img2.cols, y), cv::Scalar(0), 2);

    // 蓝图绿线
    cv::Mat img3(h, w, CV_8UC3, cv::Scalar(255, 0, 0));
    cv::line(img3, cv::Point(-10, y), cv::Point(img3.cols, y), cv::Scalar(0, 255, 0), 1);

//    // 三张图片合并到一张图片上显示
    cv::Mat img(h, w, CV_8UC3);
//    cv::cvtColor(img1, img(cv::Rect(0, 0, w, h)), cv::COLOR_GRAY2BGR);
//    cv::cvtColor(img2, img(cv::Rect(0, h, w, h)), cv::COLOR_GRAY2BGR);
    img3.copyTo(img(cv::Rect(0, 0, w, h)));

    cv::imshow(name, img);
    cv::waitKey(2);
}

void Approach::TestLidar()
{
//    edouble             dNow                = 0;
//    eint                nCount              = 0;
//    CRigidBodyList      iRigid;
//    CPaperRollMsg       iMsg;
//    ebool               eFlag               = false;
//    CPose               iRoll,iCar,iRollLidar;
//    edouble             dt                  = 0;
//    edouble             dArr[4][4]          = {{0.9999,-0.0130,0,1.4194},
//                                               {0.0130,0.9990,0,0.0206},
//                                               {0,0,1.0000,-1.9824},
//                                               {0,0,0,1.0000}};
//    CTransformationM    iM;
//    CPose               iPosCar,iPosPaper,iPosTag,iPosTmp,iPosRel;

//    memcpy(iM.m_vdMat,dArr,sizeof (dArr));
//    CCSysConv::TransformationMatrixToPose(iM,iPosTmp);
//    // 获取雷达和摄像头中纸卷位置
//    if (false == GetMsgData(&iMsg))
//    {
//        Log::inf(ClassName,"get paper roll failed\n");
//        return;
//    }
//    dNow                                = Time::GetCurrentTime();
//    nCount                              = iMsg.m_vPaperRollList.size();
//    for(int i=0;i<nCount;i++)
//    {
//        const CPaperRoll   &iR          = iMsg.m_vPaperRollList[i];
//        edouble             dt          = dNow - iMsg.m_dTimeStamp;
//        if(fabs(iR.m_fDiameter - m_fTargetRadius) > RadiusRange) continue;
//        Log::inf(ClassName,"roll[%d]:radius=%.2f,x=%.2f,y=%.2f,dt=%.3f,now=%lf",
//                 i,iR.m_fDiameter,iR.m_iPose.m_iPosition.m_dx,iR.m_iPose.m_iPosition.m_dy,dt,dNow);
//        iRoll                           = iR.m_iPose;
//        eFlag                           = true;
//    }
//    // 获取纸卷和车的全局位置
//    if(false == GetMsgData(&iRigid))
//    {
//        Log::inf(ClassName,"get rigid body failed");
//        return;
//    }
//    nCount                              = iRigid.m_vItemList.size();
//    for(int i=0;i<nCount;i++)
//    {
//        echar * strName                 = iRigid.m_vItemList[i].m_szName;
//        string  str                     = strName;
//        CPose   iPos                    = iRigid.m_vItemList[i].m_iPose;
//        dt                              = dNow - iRigid.m_dTimeStamp;
//        if(str == "Mark"){
//            iPosTag                     = iPos;
//        }
//        if(str == "Paper"){
//            iPosPaper                   = iPos;
//        }
////        Log::inf(ClassName,"[%d]name=%s,x=%.3f,y=%.3f,Rz=%.3f",i,strName,iPos.m_iPosition.m_dx,iPos.m_iPosition.m_dy,iPos.m_iRotation.m_dRz);
//    }

////    CCSysConv::CoordinateTransform2(iPosTag,iPosTmp,iPosCar);
//    CCSysConv::CoordinateTransform2(iPosTmp,iPosTag,iPosCar);
//    Log::inf(ClassName,"mark:x=%.3f,y=%.3f,Rz=%.3f,robot:x=%.3f,y=%.3f,Rz=%.3f,dt=%.3f",
//             iPosTag.m_iPosition.m_dx,iPosTag.m_iPosition.m_dy,iPosTag.m_iRotation.m_dRz,
//             iPosCar.m_iPosition.m_dx,iPosCar.m_iPosition.m_dy,iPosCar.m_iRotation.m_dRz,dt);

//    CCSysConv::CoordinateTransform(iPosPaper,iPosCar,iPosRel);
//    Log::inf(ClassName,"paper:x=%.3f,y=%.3f,relative:x=%.3f,y=%.3f,dt=%.3f",
//             iPosPaper.m_iPosition.m_dx,iPosPaper.m_iPosition.m_dy,
//             iPosRel.m_iPosition.m_dx,iPosRel.m_iPosition.m_dy,dt);
//    if(eFlag){
//        edouble dX                          = iPosRel.m_iPosition.m_dx - iRoll.m_iPosition.m_dx;
//        edouble dY                          = iPosRel.m_iPosition.m_dy - iRoll.m_iPosition.m_dy;
//        Log::inf(ClassName,"diff ============ x=%.3f,y=%.3f",dX,dY);
//    }
}

void Approach::Test()
{
    edouble             dArr[4][4]          = {{0.9999,-0.0130,0,1.4194},
                                               {0.0130,0.9990,0,0.0206},
                                               {0,0,1.0000,-1.9824},
                                               {0,0,0,1.0000}};
    CTransformationM    iM;

    CPose  paperInLidar , robotInRealis ,paperInRealis , resultInRealis;

    paperInLidar.m_iPosition.m_dx   = 4.35;
    paperInLidar.m_iPosition.m_dy   = -0.90;

    paperInRealis.m_iPosition.m_dx  = 4.605;
    paperInRealis.m_iPosition.m_dy  = -0.107;

    robotInRealis.m_iPosition.m_dx  = -0.029;
    robotInRealis.m_iPosition.m_dy  = 0.394;
    robotInRealis.m_iRotation.m_dRz = 0.048;

    edouble dTheta                  = robotInRealis.m_iRotation.m_dRz;
    CPosition &iPos                 = paperInLidar.m_iPosition;
    resultInRealis.m_iPosition.m_dx = robotInRealis.m_iPosition.m_dx + iPos.m_dx*cos(dTheta) - iPos.m_dy*sin(dTheta);
    resultInRealis.m_iPosition.m_dy = robotInRealis.m_iPosition.m_dy + iPos.m_dx*sin(dTheta) + iPos.m_dy*cos(dTheta);
    edouble  dx                     = paperInRealis.m_iPosition.m_dx - resultInRealis.m_iPosition.m_dx;
    edouble  dy                     = paperInRealis.m_iPosition.m_dy - resultInRealis.m_iPosition.m_dy;

    printf("diff ================= dx=%.3f,dy=%.3f\n",dx,dy);
}





