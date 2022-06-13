#ifndef Approach_H
#define Approach_H

#include <EwayFunc/FuncInterface.h>
#include "eigen3/Eigen/Dense"

Declare_FunLibSymble


// 纸卷的位置数据
typedef struct
{
    ebool           bValid                  = false;    // 数据是否有效
    edouble         dTime                   = 0;        // 数据的时间,秒
    edouble         dX                      = 0;        // 相对于机器人的x,米
    edouble         dY                      = 0;        // 相对于机器人的y,米
    edouble         dMapX                   = 0;        // 绝对位置，地图中的x,米
    edouble         dMapY                   = 0;        // 绝对位置，地图中的y,米
    edouble         dRadius                 = 0;        // 半径,米
}PaperData;

// 机器人的位置数据
typedef struct
{
    edouble         dTime                   = 0;        // 数据的时间,秒
    edouble         dX                      = 0;        // 在地图中的x,米
    edouble         dY                      = 0;        // 在地图中的y,米
    edouble         dRz                     = 0;        // 在地图中的偏航角,弧度
}RobotData;

// 后轮角度数据
typedef  struct
{
    edouble         dTime                   = 0;        // 数据的时间,秒
    edouble         dAngle                  = 0;        // 后轮角度，弧度
}WheelData;

// 配置
typedef struct
{
    // 固定配置
    edouble         dWheelDistance          = 0;        // 前面两个轮间距,米
    edouble         dWheelL                 = 0;        // 前后轮间距,米
    edouble         dWheelCircle            = 0;        // 前轮周长,米
    edouble         dWheelRadius            = 0;        // 前轮半径,米
    edouble         dShortArmTipX           = 0;        // 叉子短臂端点x,米
    edouble         dShortArmTipY           = 0;        // 叉子短臂端点y,米
    edouble         dLongArmJointX          = 0;        // 叉子长臂的转动点x,米
    edouble         dLongArmJointY          = 0;        // 叉子长臂的转动点y,米
    edouble         dLongArmLength          = 0;        // 叉子长臂的长度,米
    edouble         dMaxWheelAngle          = 0;        // 轮子最大允许角度，弧度

    // 自定义的参数
    edouble         dPaperRadiusOffset      = 0;        // 纸卷半径允许最大差距,米
    edouble         dPaperValidTime         = 0;        // 纸卷数据有效时间，在这个时间以内取相对位置，超过这个时间取地图中的位置,秒
    edouble         dPaperAllowedDistance   = 0;        // 保存纸卷位置时，相差多远以内算作同一个,米
    edouble         dPaperMinRelativeX      = 0;        // 纸卷相对于机器人的x大于这个值,使用相对位置，否则使用地图中位置,米
    edouble         dRobotPosValidTime      = 0;        // 机器人位置数据有效时间,秒
    edouble         dWheelAngleValidTime    = 0;        // 轮子角度数据有效时间,秒
    edouble         dWheelAngleAllowedDiff  = 0;        // 轮子当前角度与要设置的角度差值超过这个值时，速度设为0(等它转过来),弧度
    edouble         dDistanceToSlow         = 0;        // 减速的距离,米
    edouble         dDistanceToStop         = 0;        // 停止的距离,米
    edouble         dMaxSpeed               = 0;        // 最大速度 m/s
    edouble         dMinSpeed               = 0;        // 最小速度

    // 根据轮子的角度和步骤1的Yn，算出步骤1的Xn，主要是为了防止随意设置的Xn和Yn车到不了
    edouble         dStep1WheelAngle        = 0;        // 步骤1的轮子角度
    edouble         dStep1Yn                = 0;        // 步骤1的Yn

}ApproachParameter;




class Approach:public CFuncInterface
{
public:
    Approach();
    virtual             ~Approach();
    virtual eint        Initialize(std::vector<eint> &vFeatureList,ebool &bLimbAuthority,edouble &dTimeOut);
    virtual eint        Destroy();
    virtual eint        CheckMsgCode(eint nMsgCode);

    virtual eint        ProcParentParam(CSetParamMsg* piMsg);
    virtual void        JobStart();
    virtual void        JobFailed(eint nErrCode);

    virtual eint        ProcTimer(edouble dTimeStamp);

    DECLEAR_FIFMESSAGE_MAP


private:
    ebool               GetXnYn();
    ebool               GetStep1Xn();
    ebool               GetRobotData();
    ebool               GetWheelData();
    ebool               GetPaperData(edouble &dPaperX,edouble &dPaperY);
    eint                CalculateSpeed(edouble dXn, edouble dYn, edouble dPaperX, edouble dPaperY, edouble &dSpeed, edouble &dWheelAngle,ebool &bStop);
    eint                ApproachStep1();
    eint                ApproachStep2();
    eint                ApproachEnd(eint nErr);

    void                GetRobotPosInRealis(const Eigen::Matrix4d &iMatRobotInMark, const CPose &iMarkInRealis, CPosition &iRobotPosInRealis);
    void                GetPaperPosInRobot(const CPosition &iPaperPosInLidar,const CPose &iRobotInRealis,CPosition &iPaperPosInRealis);
    ebool               TestLidarAndRealis();
    void                TestMatrix();


    ebool               m_bMotionReady              = false;            // 电机是否挂挡
    ebool               m_bStep1Finished            = false;
    edouble             m_dApproachPaperRadius      = 0.5;              // 要靠近的纸卷半径
    edouble             m_dCatchPaperRadius         = 0.5;              // 要抓取或已经抓取的纸卷半径

    ApproachParameter   m_sParam;
    RobotData           m_sRobot;
    PaperData           m_sPaper;
    WheelData           m_sWheel;

    edouble             m_dXn                       = 0;
    edouble             m_dYn                       = 0;
    edouble             m_dStep1Xn                  = 0;
    edouble             m_dStep1Yn                  = 0;

};

#endif // Approach_H



