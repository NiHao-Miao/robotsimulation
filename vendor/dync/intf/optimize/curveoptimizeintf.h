#ifndef CURVEOPTIMIZEINTF_H
#define CURVEOPTIMIZEINTF_H

#include "../../intf/dynamics/dynmodelintf.h"
#include "../../intf/dynamics/inccurveintf.h"

namespace HcdynLib {

class CurveOptimizeInterface {
public:
    virtual ~CurveOptimizeInterface();

public:
    ///
    /// \brief Create 构建激励轨迹优化对象
    /// \param group_scale 种群规模
    /// \param crossover 交叉概率
    /// \param mutation 变异概率
    /// \return
    ///
    static CurveOptimizeInterface *Create(const int group_scale = 50, const float crossover = 0.6,
                                          const float mutation = 0.01);

public:
    ///
    /// \brief SetIncCurveParam 设置激励轨迹相关参数
    /// \param param
    ///
    virtual void SetIncCurveParam(const vector<JointParam> &param) = 0;
    ///
    /// \brief Genetic 遗传算法迭代流程
    /// \param max_gens 迭代次数
    /// \return 1-迭代完成
    ///
    virtual int Genetic(const int max_gens = 500) = 0;
    ///
    /// \brief GetOptimizeRet 获取优化迭代结果
    /// \param coff 激励轨迹系数列表
    ///
    virtual void GetOptimizeRet(vector<double> &coff) = 0;
};

} // namespace HcdynLib

#endif // CURVEOPTIMIZEINTF_H
