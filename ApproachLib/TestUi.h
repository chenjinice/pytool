#ifndef TESTUI_H
#define TESTUI_H


#include <EwayFunc/FuncInterface.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>


class TestUi
{
public:
    static TestUi *     Ins();
    ~TestUi();

    void                Init();
    cv::Point           GetPoint(const CPose &iPos);
    void                SaveRobotInRealis(const CPose &iPos);
    void                SavePaperRadius(efloat fRadius);
    void                SavePaperInLidar(const CPose &iPos);
    void                SavePaperInRealis(const CPose &iPos);
    void                SaveText(estring strText);
    void                Show();

private:
    TestUi();

    ebool               m_bHasRobotPos          = false;
    ebool               m_bHasPaperInLidar      = false;
    ebool               m_bHasPaperInRealis     = false;
    efloat              m_fPaperRadius          = 0.6;
    CPose               m_iPaperInLidarPos;
    CPose               m_iPaperInRealisPos;
    CPose               m_iRobotPos;
    estring             m_strText;
    cv::Mat             m_iTrajImg;

//  参数
    eint                m_nRes                  = 100;
    edouble             m_dMapMinX              = -10.0;
    edouble             m_dMapMaxX              = 10.0;
    edouble             m_dMapMinY              = -10.0;
    edouble             m_dMapMaxY              = 10.0;

    eint                m_nImgCols              = (m_dMapMaxY-m_dMapMinY) * m_nRes;
    eint                m_nImgRows              = (m_dMapMaxX-m_dMapMinX) * m_nRes;
    eint                m_nWindowWidth          = 800;
    eint                m_nWindowHeight         = m_nImgRows * m_nWindowWidth / m_nImgCols;

    cv::Point           m_iZeroPos;
};

#endif // TESTUI_H
