#include "TestUi.h"


using namespace std;
using namespace ewayos;


static const cv::Scalar ScalarLidar(0,0,255);
static const cv::Scalar ScalarRealis(0,255,0);


TestUi *TestUi::Ins()
{
    static TestUi instance;
    return &instance;
}

TestUi::TestUi()
{
    this->Init();

    m_iTrajImg              = cv::Mat(m_nImgRows,m_nImgCols,CV_8UC3,cv::Scalar(255,255,255));
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

cv::Point TestUi::GetPoint(const CPose &iPos)
{
    cv::Point iPoint;
    iPoint.x    = iPos.m_iPosition.m_dy * m_nRes + m_iZeroPos.x;
    iPoint.y    = iPos.m_iPosition.m_dx * m_nRes + m_iZeroPos.y;
    return iPoint;
}

void TestUi::SaveRobotInRealis(const CPose &iPos)
{
    if(m_bHasRobotPos)
    {
        cv::Point iLastPos      = this->GetPoint(m_iRobotPos);
        cv::Point iNowPos       = this->GetPoint(iPos);
        cv::line(m_iTrajImg,iLastPos,iNowPos,ScalarRealis,3,cv::LINE_AA);
    }else{
        m_bHasRobotPos          = true;
    }
    m_iRobotPos                 = iPos;
}

void TestUi::SavePaperRadius(efloat fRadius)
{
    m_fPaperRadius              = fRadius;
}

void TestUi::SavePaperInLidar(const CPose &iPos)
{
    m_bHasPaperInLidar          = true;
    m_iPaperInLidarPos          = iPos;
}

void TestUi::SavePaperInRealis(const CPose &iPos)
{
    m_bHasPaperInRealis         = true;
    m_iPaperInRealisPos         = iPos;
}

void TestUi::SaveText(string strText)
{
    m_strText                   = strText;
}

void TestUi::Show()
{
    eint nRadius                = m_fPaperRadius * m_nRes;
    eint nCols                  = m_iTrajImg.cols;
    eint nRows                  = m_iTrajImg.rows;
    cv::Mat iImg(nRows,nCols,CV_8UC3,cv::Scalar(255,255,255));
    m_iTrajImg.copyTo(iImg(cv::Rect(0,0,nCols,nRows)));

    // 画原点
    cv::drawMarker(iImg,m_iZeroPos,cv::Scalar(0,0,0),cv::MARKER_CROSS,100,2,cv::LINE_AA);
    // 画机器人位置
    cv::drawMarker(iImg,this->GetPoint(m_iRobotPos),cv::Scalar(0,0,0),cv::MARKER_STAR,15,2,cv::LINE_AA);
    // 画雷达中的纸卷位置
    if(m_bHasPaperInLidar){
        cv::Point iPaperInLidarPos  = this->GetPoint(m_iPaperInLidarPos);
        cv::circle(iImg,iPaperInLidarPos,nRadius,ScalarLidar,3,cv::LINE_AA);
        cv::drawMarker(iImg,iPaperInLidarPos,ScalarLidar,cv::MARKER_CROSS,15,2,cv::LINE_AA);
    }
    // 画realis中的纸卷位置
    if(m_bHasPaperInRealis){
        cv::Point iPaperInRealisPos = this->GetPoint(m_iPaperInRealisPos);
        cv::circle(iImg,iPaperInRealisPos,nRadius,ScalarRealis,3,cv::LINE_AA);
        cv::drawMarker(iImg,iPaperInRealisPos,ScalarRealis,cv::MARKER_CROSS,15,2,cv::LINE_AA);
    }
    // 旋转x，y
    cv::flip(iImg,iImg,-1);
    // 重新设置画面大小
    cv::resize(iImg,iImg,cv::Size(m_nWindowWidth,m_nWindowHeight));
    // 画文字
    if(m_bHasPaperInLidar && m_bHasPaperInRealis){
        cv::putText(iImg,m_strText,cv::Point(50,35),cv::FONT_HERSHEY_COMPLEX,1,cv::Scalar(0,0,255),2,cv::LINE_AA);
    }
    cv::imshow("image", iImg);
    cv::waitKey(1);

    m_bHasPaperInLidar          = false;
    m_bHasPaperInRealis         = false;
}

