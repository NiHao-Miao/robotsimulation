#ifndef DYNMODELINTF_H
#define DYNMODELINTF_H

#include "../../Eigen/Dense"
#include <map>
#include <vector>

using namespace Eigen;
using namespace std;

namespace HcdynLib {

// 假设当前轴为i
// R1：轴0~i-1中存在不与轴i平行的轴
// R2：轴0~i-1都与轴i平行
// R3：轴0~i-1都与轴i重合
typedef enum JointFlagEnum {
    jfR1 = 1,
    jfR2,
    jfR3,
} JointFlag;

// 关节类型 0-旋转轴  1-直线轴
typedef enum JointTypeEnum {
    jtRotation = 0,
    jtTranslation,
} JointType;

typedef struct JointParam_ {
    double alpha; // MDH参数
    double a;
    double d;
    double theta;
    JointType joint_type;
    JointFlag joint_flag;
    double pos_p;   // 关节位置正极限
    double pos_n;   // 关节位置负极限
    double max_vel; // 关节最大速度
    double max_acc; // 关节最大加速度
} JointParam;

typedef struct LoadParam_ {
    double m; // 质量
    double x; // 质心位置
    double y;
    double z;
} LoadParam;

class DynModelInterface {
public:
    virtual ~DynModelInterface();

public:
    ///
    /// \brief Create 创建动力学模型
    /// \param jointParam 关节参数
    /// \return
    ///
    static DynModelInterface *Create(const vector<JointParam> &jointParam);

public:
    ///
    /// \brief CalMinParamSet 计算最小参数集
    /// \param q_map 关节角位移 <关节索引，角位移数据列表>
    /// \param dq_map 关节角速度
    /// \param ddq_map 关节角加速度
    /// \param tau_map 关节力矩
    /// \param minPSet 最小参数集
    /// \return
    ///
    virtual bool CalMinParamSet(const map<int, vector<double>> &q_map, const map<int, vector<double>> &dq_map,
                                const map<int, vector<double>> &ddq_map, const map<int, vector<double>> &tau_map,
                                VectorXd &minPSet) = 0;
    ///
    /// \brief CalTheTorque 计算理论模型力矩
    /// \param q 各个关节角位移的当前值
    /// \param dq 各个关节角速度的当前值
    /// \param ddq 各个关节角加速度的当前值
    /// \param tau 各个关节的理论模型力矩计算值
    /// \param isAdjustTauF 是否调整摩擦力矩补偿
    /// \return
    ///
    virtual bool CalTheTorque(const VectorXd &q, const VectorXd &dq, const VectorXd &ddq, VectorXd &tau,
                              bool isAdjustTauF = false) = 0;
    ///
    /// \brief GetObsMatrix 获取观测矩阵，可用于激励轨迹优化目标函数的设计、观测器的设计等
    /// \param q 各个关节角位移的当前值
    /// \param dq 各个关节角速度的当前值
    /// \param ddq 各个关节角加速度的当前值
    /// \param obsmat 观测矩阵
    /// \return
    ///
    virtual bool GetObsMatrix(const VectorXd &q, const VectorXd &dq, const VectorXd &ddq, MatrixXd &obsmat) = 0;
    ///
    /// \brief ModifyMinParamSet 更新最小参数集
    /// \param minPSet 最小参数集列表
    ///
    virtual void ModifyMinParamSet(const VectorXd &minPSet) = 0;
    ///
    /// \brief ModifyFTorqueRatio 调整摩擦力矩补偿比例
    /// \param ratio 摩擦力矩补偿比例列表 <关节，比例>
    ///
    virtual void ModifyFTorqueRatio(const map<int, float> &ratio) = 0;
    ///
    /// \brief CalEndLoadTorque 计算末端负载作用于各个关节的力矩
    /// \param q 各个关节角位移的当前值
    /// \param dq 各个关节角速度的当前值
    /// \param ddq 各个关节角加速度的当前值
    /// \param param 负载参数
    /// \param tau 末端负载作用于各个关节的力矩
    /// \return
    ///
    virtual bool CalEndLoadTorque(const VectorXd &q, const VectorXd &dq, const VectorXd &ddq, const LoadParam &param,
                                  VectorXd &tau) = 0;
};

} // namespace HcdynLib

#endif // DYNMODELINTF_H
