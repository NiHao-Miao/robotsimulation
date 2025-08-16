#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include "sceneobject.h"
#include <QVector>
#include <gp_Pnt.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <AIS_Shape.hxx>

/**
@brief 代表场景中的一条3D轨迹。
Trajectory 本身也是一个 SceneObject，可以被场景统一管理。
它内部存储一系列相对于某个“参考坐标系”的局部坐标点。
它的 update() 方法会根据其参考坐标系拥有者的实时位姿，来更新自身在世界中的位置。
*/

class Trajectory : public SceneObject
{
    Q_OBJECT
public:
    explicit Trajectory(QObject* parent = nullptr);
    virtual ~Trajectory();

    // --- SceneObject 接口实现 ---
    virtual bool load(const Handle(AIS_InteractiveContext)& context, const QVariant& data) override;
    virtual void update() override;
    virtual QString getBaseName() const override;
    virtual void removeFromScene(const Handle(AIS_InteractiveContext)& context) override;


    // --- Trajectory 专属功能 ---

    /**
     * @brief 向轨迹中添加一个点。
     * @param local_point 要添加的点，其坐标是相对于本轨迹的参考坐标系的。
     */
    void addPoint(const gp_Pnt& local_point);

    /**
     * @brief 清空轨迹中的所有点。
     */
    void clear();

    /**
     * @brief 设置本轨迹的参考坐标系。
     * @param owner 定义该坐标系的 SceneObject 对象。轨迹将跟随这个对象移动。
     */
    void setReferenceFrame(SceneObject* owner);
    // 设置颜色的公共方法声明 ---
    void setTrajectoryColor(const Quantity_Color& color);

    //导入和导入
    QStringList exportPoints() const;
    void importPoints(const QStringList& points);

    SceneObject* getReferenceFrameOwner() const { return m_referenceFrameOwner; }

private:
    QVector<gp_Pnt> m_localPoints; // 存储相对于“参考坐标系”的局部坐标点
    SceneObject* m_referenceFrameOwner; // 指向定义本轨迹坐标系的那个物体
    // --- 视觉表现相关成员 ---
    TopoDS_Compound m_compound;         // OpenCASCADE的几何体集合，容纳所有轨迹点顶点
    BRep_Builder m_builder;             // 用于向集合中添加顶点的工具

};

#endif // TRAJECTORY_H
