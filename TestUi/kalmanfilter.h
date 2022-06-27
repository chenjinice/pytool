#ifndef KALMANFILTER_H
#define KALMANFILTER_H


#include <iostream>
#include "eigen3/Eigen/Dense"


class KalmanFilter
{
public:
    KalmanFilter(int stateSize_, int measSize_,int uSize_);
    ~KalmanFilter();
    void init(Eigen::VectorXd &x_, Eigen::MatrixXd& P_,Eigen::MatrixXd& R_, Eigen::MatrixXd& Q_);
    Eigen::VectorXd predict(Eigen::MatrixXd& A_);
    Eigen::VectorXd predict(Eigen::MatrixXd& A_, Eigen::MatrixXd &B_, Eigen::VectorXd &u_);
    void update(Eigen::MatrixXd& H_,Eigen::VectorXd z_meas);


private:
    int stateSize;
    int measSize;
    int uSize;
    Eigen::VectorXd x;
    Eigen::VectorXd z;
    Eigen::MatrixXd A;
    Eigen::MatrixXd B;
    Eigen::VectorXd u;
    Eigen::MatrixXd P;
    Eigen::MatrixXd H;
    Eigen::MatrixXd R;//measurement noise covariance
    Eigen::MatrixXd Q;//process noise covariance
};

#endif // KALMANFILTER_H
