#ifndef Approach_H
#define Approach_H

#include <EwayFunc/FuncInterface.h>

Declare_FunLibSymble


typedef struct
{
    ebool           valid       = false;    // 数据是否有效
    edouble         time        = 0;        // 数据记录的时间
    edouble         x           = 0;        // 相对位置x
    edouble         y           = 0;
    edouble         mapX        = 0;        // 绝对位置，地图中的位置
    edouble         mapY        = 0;
    efloat          diameter    = 0;        // 半径
}CurPaperRoll;



class Approach:public CFuncInterface
{
public:
    Approach();
    virtual ~Approach();
    virtual eint    Initialize(std::vector<eint> &vFeatureList,ebool &bLimbAuthority,edouble &dTimeOut);
    virtual eint    Destroy();
    virtual eint    CheckMsgCode(eint nMsgCode);

    virtual void    JobStart();
    virtual void    JobFailed(eint nErrCode);

    virtual eint    ProcTimer(edouble dTimeStamp);
    virtual eint    ProcFeatureStatus(edouble m_dTimeStamp,CFeatureStatusMsg*piMsg);


    DECLEAR_FIFMESSAGE_MAP


private:
    eint            ApproachEnd(ebool flag);
    eint            TrackPaperRoll(const CLocalizationMsg &iLocalization);
    bool            GetWheelState();
    eint            GetXnYn(efloat radius,efloat &fXn,efloat &fYn);
    eint            GetXnYnStep1(efloat fTheta);
    bool            GetSpeed(efloat fXn, efloat fYn, efloat fRollX, efloat fRollY, efloat &fSpeed, efloat &fSteerAngle);
    eint            ApproachStep1();
    eint            ApproachStep2();
    void            ShowGui();
    void            TestLidar();
    void            Test();


    ebool           m_bMotionReady              = false;            // 电机是否挂挡
    efloat          m_fTargetRadius             = 0.6;              // 叉车要接近的纸卷半径
    CurPaperRoll    m_sRoll;                                        // 记录当前观察的纸卷数据
    efloat          m_fWheelAngle               = 0;                // 轮子角度，弧度

    ebool           m_bStep1Finished            = false;
    efloat          m_fXn                       = 0;
    efloat          m_fYn                       = 0;
    efloat          m_fXnStep1                  = 0;
    efloat          m_fYnStep1                  = 0;

};

#endif // Approach_H
