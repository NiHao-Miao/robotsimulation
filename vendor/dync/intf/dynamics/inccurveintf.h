#ifndef INCCURVEINTF_H
#define INCCURVEINTF_H

#include <map>
#include <vector>

using namespace std;

#define PI 3.14159265358979323846
#define A2R 0.0174532925199432957692369076848
#define R2A 57.295779513082320876798154814105

namespace HcdynLib {

class IncCurveInterface {
public:
    virtual ~IncCurveInterface();

public:
    ///
    /// \brief Create 构造激励轨迹对象
    /// \return
    ///
    static IncCurveInterface *Create();

public:
    ///
    /// \brief SetCurveCoff 设置激励轨迹系数
    /// \param vec 激励轨迹系数列表
    /// \return
    ///
    virtual bool SetCurveCoff(const vector<double> &vec) = 0;
    ///
    /// \brief CreateCurve 创建激励轨迹
    /// \param T 周期
    /// \param isMatch 是否需要头尾拼接曲线使位置、速度、加速度初末都为0
    /// \return
    ///
    virtual bool CreateCurve(const int T = 10, bool isMatch = true) = 0;
    ///
    /// \brief GetCurvePos  获取激励轨迹角位移
    /// \return <关节索引，角位移列表>
    ///
    virtual map<int, vector<double>> &GetCurvePos() = 0;
    ///
    /// \brief GetCurveVel  获取激励轨迹角速度
    /// \return <关节索引，角速度列表>
    ///
    virtual map<int, vector<double>> &GetCurveVel() = 0;
    ///
    /// \brief GetCurveAcc  获取激励轨迹角加速度
    /// \return <关节索引，角加速度列表>
    ///
    virtual map<int, vector<double>> &GetCurveAcc() = 0;
};

} // namespace HcdynLib

#endif // INCCURVEINTF_H
