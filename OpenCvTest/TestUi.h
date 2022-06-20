#ifndef TESTUI_H
#define TESTUI_H


#include <EwayFunc/FuncInterface.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>


//
const cv::Scalar ScalarBlack(0,0,0);
const cv::Scalar ScalarWhite(255,255,255);
const cv::Scalar ScalarRed(0,0,255);
const cv::Scalar ScalarGreen(0,255,0);
const cv::Scalar ScalarBlue(255,0,0);
//
const cv::Scalar ScalarLidar    = ScalarRed;
const cv::Scalar ScalarRealis   = ScalarGreen;
const cv::Scalar ScalarXnYn     = ScalarBlue;
//
const echar      UiImagePath[]  = "/home/muniu/Desktop/chen/image/";


// ui paper
typedef struct{
    CPosition               iPos;
    edouble                 dRadius     = 0.1;
    cv::Scalar              iColor      = ScalarBlack;
}UiPaper;

// ui text
typedef struct{
    estring                 str;
    cv::Scalar              iColor      = ScalarBlack;
}UiText;


class TestUi
{
public:
    static TestUi *         Ins();
    ~TestUi();

    void                    Init();
    cv::Point               GetUiPoint(const CPosition &iPos);
    void                    SaveRobot(const CPose &iPos);
    void                    SaveXnYn(edouble dXn,edouble dYn);
    void                    SaveStep1XnYn(edouble dXn,edouble dYn);
    void                    SavePaper(const UiPaper &sPaper);
    void                    SaveText(estring strText,cv::Scalar iScalar = ScalarRed);
    void                    SavePaperInLidar(const CPose &iPos,edouble dRadius);
    void                    SavePaperInRealis(const CPose &iPos,edouble dRadius);
    void                    Show(ebool bMapView = false, ebool bRobotView = true);


private:
    TestUi();
    void                    DrawArrow(cv::Mat &iImg, cv::Scalar iScalar, const CPose &iPos);
    void                    ShowMapView();
    void                    ShowRobotView();

    ebool                   m_bHasRobotPos          = false;
    std::vector<UiPaper>    m_vPapers;
    std::vector<UiText>     m_vText;
    CPose                   m_iRobot;
    CPosition               m_iXnYn;
    CPosition               m_iStep1XnYn;
    cv::Mat                 m_iTrajImg;

//  参数
    eint                    m_nRes                  = 100;
    edouble                 m_dMapMinX              = -2.0;
    edouble                 m_dMapMaxX              = 8.0;
    edouble                 m_dMapMinY              = -5.0;
    edouble                 m_dMapMaxY              = 5.0;

    eint                    m_nImgCols              = (m_dMapMaxY-m_dMapMinY) * m_nRes;
    eint                    m_nImgRows              = (m_dMapMaxX-m_dMapMinX) * m_nRes;
    eint                    m_nWindowWidth          = 800;
    eint                    m_nWindowHeight         = m_nImgRows * m_nWindowWidth / m_nImgCols;

    cv::Point               m_iZeroPos;
};

#endif // TESTUI_H
