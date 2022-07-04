#include "PosTransform.h"
#include "eigen3/Eigen/Dense"


using namespace std;
using namespace Eigen;




void tfTest()
{
    Matrix4d    m , ant;
    Vector4d    v;
    CPose       iTagInRealis ,iRobot;
    iTagInRealis.m_iPosition.m_dx   = 0;
    iTagInRealis.m_iPosition.m_dy   = 0;
    iTagInRealis.m_iPosition.m_dz   = 0;
    iTagInRealis.m_iRotation.m_dRx  = 0;
    iTagInRealis.m_iRotation.m_dRy  = 0;
    iTagInRealis.m_iRotation.m_dRz  = -0.4;

    m <<    0.9999,-0.0130,0,0,
            0.0130,0.9990,0,-1,
            0,0,1.0000,0,
            0,0,0,1.0000;

    v   <<  iTagInRealis.m_iPosition.m_dx,iTagInRealis.m_iPosition.m_dy,iTagInRealis.m_iPosition.m_dz,1;
    double  t = M_PI/2;
    ant <<  cos(t),-sin(t),0,1,
            sin(t),cos(t),0,1,
            0,0,1,0,
            0,0,0,1;

    Translation3d iTranslate(iTagInRealis.m_iPosition.m_dx,iTagInRealis.m_iPosition.m_dy,iTagInRealis.m_iPosition.m_dz);
    AngleAxisd iRx(iTagInRealis.m_iRotation.m_dRx, Eigen::Vector3d::UnitX());
    AngleAxisd iRy(iTagInRealis.m_iRotation.m_dRy, Eigen::Vector3d::UnitY());
    AngleAxisd iRz(iTagInRealis.m_iRotation.m_dRz, Eigen::Vector3d::UnitZ());
    Affine3d   iAffMarkerInRealis = iTranslate*iRz*iRy*iRx;

    Affine3d   iAnt = iAffMarkerInRealis * Affine3d(m);

    cout << iAnt.translation() << endl;
    cout << iAnt.rotation().eulerAngles(2,1,0) << endl;
    return;

    cout << iAffMarkerInRealis.matrix() << endl;
    cout << iAffMarkerInRealis.rotation().eulerAngles(2,1,0) << endl;

    Affine3d   iAffRobotInMarker(m);
    Affine3d   iAffRobotInRealis    = iAffMarkerInRealis * iAffRobotInMarker;
    iRobot.m_iPosition.m_dx         = iAffRobotInRealis.translation().x();


//    cout << iAffMarkerInRealis * Vector4d(1,2,4,1) << endl;
//    cout << (iTranslate*iRx*iRy*iRz).matrix() << endl;
//    cout << "-------------------"<< endl;
//    cout << (iTranslate*iRz*iRy*iRz).matrix() << endl;


//    AngleAxisd aa = AngleAxisd(M_PI, Eigen::Vector3d::UnitZ());
//    cout << iAffRobotInRealis.rotation().eulerAngles(2,1,0) << endl;
//    cout << iAffRobotInRealis.rotation().eulerAngles(0,1,2) << endl;
}
