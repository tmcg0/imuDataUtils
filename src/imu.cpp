// a simple class to hold IMU data and various methods
#include "imu.h"
#include "datapkgr.h"


using Eigen::EigenSolver;

imu::imu(std::vector<double> time, std::vector<double> gxin, std::vector<double> gyin, std::vector<double> gzin, std::vector<double> axin, std::vector<double> ayin, std::vector<double> azin, std::vector<double> mxin, std::vector<double> myin, std::vector<double> mzin):
        measTime(time), gx(gxin), gy(gyin), gz(gzin), ax(ayin), ay(ayin), az(azin), mx(mxin), my(myin), mz(mzin) {
// construct!
}

imu::imu(std::string filePath, int readIdx){
    //todo: make this
}

imu::imu(std::string filePath, std::string labelName){
    imudata id=datapkgr::readSingleImuDataFromApdmOpalH5FileByLabel(filePath, labelName);
    this->ax=id.ax; this->ay=id.ay; this->az=id.az;
    this->gx=id.gx; this->gy=id.gy; this->gz=id.gz;
    this->mx=id.mx; this->my=id.my; this->mz=id.mz;
    this->measTime=id.measTime;
    this->label=id.label;
    if(id.orientation.size()>0){ // if orientation exists
        this->orientation=id.orientation;
    }
    this->m_deltaT=id.measTime[1]-id.measTime[0];
} // end constructor

void imu::print_sensor_maxmin(){
// simple print the max and min for each sensor
    std::cout<<"max and min of each sensor:"<<std::endl;
    std::cout<<"ax: max="<<*max_element(this->ax.begin(),this->ax.end())<<" min="<<*min_element(this->ax.begin(),this->ax.end())<<std::endl;
    std::cout<<"ay: max="<<*max_element(this->ay.begin(),this->ay.end())<<" min="<<*min_element(this->ay.begin(),this->ay.end())<<std::endl;
    std::cout<<"az: max="<<*max_element(this->az.begin(),this->az.end())<<" min="<<*min_element(this->az.begin(),this->az.end())<<std::endl;
    std::cout<<"mx: max="<<*max_element(this->mx.begin(),this->mx.end())<<" min="<<*min_element(this->mx.begin(),this->mx.end())<<std::endl;
    std::cout<<"my: max="<<*max_element(this->my.begin(),this->my.end())<<" min="<<*min_element(this->my.begin(),this->my.end())<<std::endl;
    std::cout<<"mz: max="<<*max_element(this->mz.begin(),this->mz.end())<<" min="<<*min_element(this->mz.begin(),this->mz.end())<<std::endl;
    std::cout<<"gx: max="<<*max_element(this->gx.begin(),this->gx.end())<<" min="<<*min_element(this->gx.begin(),this->gx.end())<<std::endl;
    std::cout<<"gy: max="<<*max_element(this->gy.begin(),this->gy.end())<<" min="<<*min_element(this->gy.begin(),this->gy.end())<<std::endl;
    std::cout<<"gz: max="<<*max_element(this->gz.begin(),this->gz.end())<<" min="<<*min_element(this->gz.begin(),this->gz.end())<<std::endl;
}

unsigned long imu::length() const {
    return this->ax.size();
}

gtsam::Rot3 imu::q_method(Eigen::Vector3d accelBody, Eigen::Vector3d magBody, Eigen::Vector3d accelNWU, Eigen::Vector3d magNWU, double wa, double wm){
    // Davenport's q method
    // Davenport, P., “A vector approach to the algebra of rotations with applications,” NASA, X-546-65-437, 1965.
    // This method serves to compute attitude matrix A[nav->body] s.t.,
    // A*r=b
    // the attitude determination problem is finding the orthogonal matrix A that minimizes Wahba's loss function:
    // L(A)=1/2*SUM_i=1:n(w*|b(i)-A*r(i)|^2)
    // this optimal solution is given by the eigenvector (representing a quaternion q=[x y z w]) corresponding to the largest
    //      eigenvector of the 4x4 matrix K:
    // K=[B+B'-trace(B)*eye(3), z; z', trace(B)];
    //    where:  B=SUM_OVER_MEASUREMENTS(w*b*r') [3x3] -- there are two measurements here (accel and mag)
    //            z=[B(2,3)-B(3,2); B(3,1)-B(1,3); B(1,2)-B(2,1)];  [3x1]
    // weights must sum to unity
    // measurements b and r are [3x1]
    // optional: print data
    //std::cout<<"accelVec=["<<accelBody.transpose()<<"] | magVec=["<<magBody.transpose()<<"]"<<std::endl;
    // Step (1): make sure weights sum to unity
    wa=wa/(wa+wm); wm=wm/(wa+wm);
    // (2) construct matrix B
    Eigen::Matrix3d B;
    Eigen::Matrix3d Kul3; // the upper left 3x3 matrix entry in K
    Eigen::Vector3d z;
    Eigen::MatrixXd K(4,4);
    B=wa*accelBody*accelNWU.transpose()+wm*magBody*magNWU.transpose();
    // (3) construct upper left 3x3 entry in K and z
    Kul3=B+B.transpose()-Eigen::MatrixXd::Identity(3,3)*B.trace();
    z[0]=B(1,2)-B(2,1); z[1]=B(2,0)-B(0,2); z[2]=B(0,1)-B(1,0);
    // (4) construct 4x4 K matrix
    K(0,0)=Kul3(0,0); K(0,1)=Kul3(0,1); K(0,2)=Kul3(0,2);
    K(1,0)=Kul3(1,0); K(1,1)=Kul3(1,1); K(1,2)=Kul3(1,2);
    K(2,0)=Kul3(2,0); K(2,1)=Kul3(2,1); K(2,2)=Kul3(2,2);
    K(0,3)=z[0]; K(1,3)=z[1]; K(2,3)=z[2];
    K(3,0)=z[0]; K(3,1)=z[1]; K(3,2)=z[2];
    K(3,3)=B.trace();
    // (5) find the 4 eigenvalues of K
    Eigen::EigenSolver<Eigen::MatrixXd> es(K);
    Eigen::VectorXcd eigvals=K.eigenvalues();
    Eigen::MatrixXcd eigvecs=es.eigenvectors();
    // (6) now find largest eigenvalue
    int maxIdx=0; double maxVal=0;
    for (int i=0;i<4;i++){
        // verify that eigenvalues are all real
        double absImag=abs(eigvals[i].imag()); // abs value of imaginary part
        if(absImag>1e-5) {
            std::cout<<"K=\n" << K <<std::endl;
            std::cout<<"eigenvalues(K)=\n" << eigvals <<std::endl;
            std::cout<<"eigenvectors(K)=\n" << eigvecs <<std::endl;
            assert(absImag < 1e-5);
        }
        if(eigvals[i].real()>maxVal){
            maxVal=eigvals[i].real();
            maxIdx=i;
        }
    }
    // (7) now pull out the eigenvector corresponding to maxVal
    // NOTE: this comes out with a scalar-last quaternion. switch to scalar-first for gtsam usage.
    Eigen::Vector4d q_opt;
    q_opt[0]=eigvecs(3,maxIdx).real();
    q_opt[1]=eigvecs(0,maxIdx).real();
    q_opt[2]=eigvecs(1,maxIdx).real();
    q_opt[3]=eigvecs(2,maxIdx).real();

    return gtsam::Rot3(q_opt[0],q_opt[1],q_opt[2],q_opt[3]);
}

void imu::setOrientation(std::vector<gtsam::Rot3> r){
    this->orientation=r;
}

std::vector<gtsam::Rot3> imu::getOrientation(){
    return this->orientation;
}

void imu::relativeAngularVelocity(imu imuB){
    // 'this' imu is imuA, pass in imuB.
    // calculate the relative angular velocity as [B-A] in frame A
    
}

imu cutImuByIdx(int startIdx, int stopIdx){
    // cut imu data down and return the chopped imu--note that this copies the imu so the original input imu is not affected

}

gtsam::Vector3 imu::accel_Vector3(int idx){
    // return a Vector3 of acclerometer measurements at index idx
    assert(idx>=0); assert(idx<this->length());
    gtsam::Vector3 acc(this->ax[idx],this->ay[idx],this->az[idx]);
    return acc;
}

gtsam::Vector3 imu::mags_Vector3(int idx){
    // return a Vector3 of magnetometer measurements at index idx
    assert(idx>=0); assert(idx<this->length());
    gtsam::Vector3 mags(this->mx[idx],this->my[idx],this->mz[idx]);
    return mags;
}

gtsam::Vector3 imu::gyros_Vector3(int idx){
    // return a Vector3 of gyro measurements at index idx
    assert(idx>=0); assert(idx<this->length());
    gtsam::Vector3 gyros(this->gx[idx],this->gy[idx],this->gz[idx]);
    return gyros;
}

std::map<std::string,imu> imu::getImuMapFromDataFile(std::string filestr){
    // this static method constructs an imu map where the keys are the label std::strings of the imu
    std::vector<std::string> allLabels=datapkgr::getAllImuLabelsInDataFile(filestr);
    std::map<std::string,imu> ImuMap;
    for(int i=0;i<allLabels.size();i++){ // now create map
        imu tempImu(filestr,allLabels[i]);
        ImuMap.insert(std::pair<std::string,imu>(allLabels[i],tempImu));
    }
    return ImuMap;
}

void imu::printLabelsInFile(std::string datafilestr){
    // prints all imu labels from given file
    std::vector<std::string> allLabels=datapkgr::getAllImuLabelsInDataFile(datafilestr);
    for(int i=0;i<allLabels.size();i++){ // now create map
        std::cout<<"imu label "<<i<<": "<<allLabels[i]<<std::endl;
    }
}