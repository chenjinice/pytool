#include "TestUi.h"


using namespace std;
using namespace ewayos;



TestUi *TestUi::Ins()
{
    static TestUi instance;
    return &instance;
}

TestUi::TestUi()
{
    this->Init();

    m_iTrajImg              = cv::Mat(m_nImgRows,m_nImgCols,CV_8UC3,ScalarWhite);
}

TestUi::~TestUi()
{
    cv::destroyAllWindows();
}

void TestUi::Init()
{
    m_iZeroPos.x                 = -m_dMapMinY * m_nRes;
    m_iZeroPos.y                 = -m_dMapMinX * m_nRes;
}

cv::Point TestUi::GetUiPoint(const CPosition &iPos)
{
    cv::Point iPoint;
    iPoint.x    = iPos.m_dy * m_nRes + m_iZeroPos.x;
    iPoint.y    = iPos.m_dx * m_nRes + m_iZeroPos.y;
    return iPoint;
}

void TestUi::SaveRobot(const CPose &iPos)
{
    if(m_bHasRobotPos)
    {
        cv::Point iLastPos          = this->GetUiPoint(m_iRobot.m_iPosition);
        cv::Point iNowPos           = this->GetUiPoint(iPos.m_iPosition);
        cv::line(m_iTrajImg,iLastPos,iNowPos,ScalarRealis,3,cv::LINE_AA);
    }else{
        m_bHasRobotPos              = true;
    }
    m_iRobot                        = iPos;
}

void TestUi::SaveXnYn(edouble dXn, edouble dYn)
{
    m_iXnYn.m_dx                    = dXn;
    m_iXnYn.m_dy                    = dYn;
}

void TestUi::SaveStep1XnYn(edouble dXn, edouble dYn)
{
    m_iStep1XnYn.m_dx               = dXn;
    m_iStep1XnYn.m_dy               = dYn;
}

void TestUi::SavePaper(const UiPaper &sPaper)
{
    m_vPapers.push_back(sPaper);
}

void TestUi::SaveText(string strText, cv::Scalar iScalar)
{
    UiText sText;
    sText.str                   = strText;
    sText.iColor                = iScalar;
    m_vText.push_back(sText);
}

void TestUi::SavePaperInLidar(const CPose &iPos, edouble dRadius)
{
    UiPaper sPaper;
    sPaper.iPos                 = iPos.m_iPosition;
    sPaper.dRadius              = dRadius;
    sPaper.iColor               = ScalarLidar;
    this->SavePaper(sPaper);
}

void TestUi::SavePaperInRealis(const CPose &iPos, edouble dRadius)
{
    UiPaper sPaper;
    sPaper.iPos                 = iPos.m_iPosition;
    sPaper.dRadius              = dRadius;
    sPaper.iColor               = ScalarRealis;
    this->SavePaper(sPaper);
}

void TestUi::ShowMapView()
{
    size_t i;
    eint nCols                      = m_iTrajImg.cols;
    eint nRows                      = m_iTrajImg.rows;
    cv::Mat iImg(nRows,nCols,CV_8UC3,ScalarWhite);
    // 画轨迹
    m_iTrajImg.copyTo(iImg(cv::Rect(0,0,nCols,nRows)));
    // 画原点
    cv::drawMarker(iImg,m_iZeroPos,ScalarBlack,cv::MARKER_CROSS,100,2,cv::LINE_AA);
    // 画机器人位置
    this->DrawArrow(iImg,ScalarRealis,m_iRobot);
    // 画map中纸卷位置
    for(i=0;i<m_vPapers.size();i++){
        const UiPaper &sPaper       = m_vPapers[i];
        CPosition   iPos;
        edouble     dTheta          = m_iRobot.m_iRotation.m_dRz;
        iPos.m_dx                   = m_iRobot.m_iPosition.m_dx + sPaper.iPos.m_dx*cos(dTheta) - sPaper.iPos.m_dy*sin(dTheta);
        iPos.m_dy                   = m_iRobot.m_iPosition.m_dy + sPaper.iPos.m_dx*sin(dTheta) + sPaper.iPos.m_dy*cos(dTheta);
        eint nRadius                = sPaper.dRadius * m_nRes;
        cv::Point iPoint            = this->GetUiPoint(iPos);
        cv::circle(iImg,iPoint,nRadius,sPaper.iColor,3,cv::LINE_AA);
        cv::drawMarker(iImg,iPoint,sPaper.iColor,cv::MARKER_CROSS,50,2,cv::LINE_AA);
    }
    // 旋转x，y
    cv::flip(iImg,iImg,-1);
    // 重新设置画面大小
    cv::resize(iImg,iImg,cv::Size(m_nWindowWidth,m_nWindowHeight));
    // 画文字
    for(i=0;i<m_vText.size();i++){
        eint nY = (i+1)*35;
        const UiText &iText         = m_vText[i];
        cv::putText(iImg,iText.str,cv::Point(20,nY),cv::FONT_HERSHEY_COMPLEX,1,iText.iColor,2,cv::LINE_AA);
    }
    cv::imshow("map view", iImg);
    cv::waitKey(1);
}

void TestUi::ShowRobotView()
{
    size_t  i;
    eint nCols                      = m_iTrajImg.cols;
    eint nRows                      = m_iTrajImg.rows;
    cv::Mat iImg(nRows,nCols,CV_8UC3,ScalarWhite);
    // 画原点
    cv::drawMarker(iImg,m_iZeroPos,ScalarBlack,cv::MARKER_CROSS,500,2,cv::LINE_AA);
    // 画纸卷位置
    for(i=0;i<m_vPapers.size();i++){
        const UiPaper &sPaper       = m_vPapers[i];
        eint nRadius                = sPaper.dRadius * m_nRes;
        cv::Point iPoint            = this->GetUiPoint(sPaper.iPos);
        cv::circle(iImg,iPoint,nRadius,sPaper.iColor,3,cv::LINE_AA);
        cv::drawMarker(iImg,iPoint,sPaper.iColor,cv::MARKER_CROSS,50,2,cv::LINE_AA);
    }
    // 画Xn,Yn
    cv::drawMarker(iImg,this->GetUiPoint(m_iXnYn),ScalarXnYn,cv::MARKER_CROSS,50,2,cv::LINE_AA);
    // 画Step1的Xn，Yn
    cv::drawMarker(iImg,this->GetUiPoint(m_iStep1XnYn),ScalarXnYn,cv::MARKER_CROSS,50,2,cv::LINE_AA);
    // 旋转x，y
    cv::flip(iImg,iImg,-1);
    // 重新设置画面大小
    cv::resize(iImg,iImg,cv::Size(m_nWindowWidth,m_nWindowHeight));
    // 画文字
    for(i=0;i<m_vText.size();i++){
        eint nY = (i+1)*35;
        const UiText &iText         = m_vText[i];
        cv::putText(iImg,iText.str,cv::Point(20,nY),cv::FONT_HERSHEY_COMPLEX,1,iText.iColor,2,cv::LINE_AA);
    }
//    echar cFile[200];
//    edouble dNow = ewayos::Time::GetCurrentTime();
//    sprintf(cFile,"%s%3f.jpg",UiImagePath,dNow);
//    cv::imwrite(cFile,iImg);
    cv::imshow("robot view",iImg);
    cv::waitKey(1);
}

void TestUi::Show(ebool bMapView, ebool bRobotView)
{
    if(bMapView)
    {
        this->ShowMapView();
    }
    if(bRobotView)
    {
        this->ShowRobotView();
    }
    m_vPapers.clear();
    m_vText.clear();
}

void TestUi::DrawArrow(cv::Mat &iImg, cv::Scalar iScalar, const CPose &iPos)
{
    CPose       iEndPos;
    cv::Point   iStart,iEnd;
    edouble     dX = 0.5 ,  dY  = 0;
    edouble     dTheta          = iPos.m_iRotation.m_dRz;
    iEndPos.m_iPosition.m_dx    = iPos.m_iPosition.m_dx + dX*cos(dTheta) - dY*sin(dTheta);
    iEndPos.m_iPosition.m_dy    = iPos.m_iPosition.m_dy + dX*sin(dTheta) + dY*cos(dTheta);
    iStart                      = this->GetUiPoint(iPos.m_iPosition);
    iEnd                        = this->GetUiPoint(iEndPos.m_iPosition);
    cv::arrowedLine(iImg,iStart,iEnd,iScalar,4,cv::LINE_AA,0,0.5);
}

