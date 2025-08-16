#ifndef DYNAMICOBJECT_H
#define DYNAMICOBJECT_H

#include "sceneobject.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <AIS_Shape.hxx>
/**
@brief 代表场景中具有自主动态行为的物体。
这种物体在每帧更新时，会根据自身逻辑改变其变换矩阵 m_transform。
典型的例子是一个自转的物体、一个往复运动的平台等。
*/
class DynamicObject : public SceneObject
{
    Q_OBJECT
public:
    explicit DynamicObject(QObject* parent = nullptr);
    virtual ~DynamicObject();
    /**
     * @brief 加载一个立方体模型。
     *
     * @param context OpenCASCADE的显示上下文。
     * @param data (未使用) 为了与基类接口保持一致。
     * @return 总是返回true。
     */
    virtual bool load(const Handle(AIS_InteractiveContext)& context, const QVariant& data) override;

    /**
     * @brief 每帧更新状态，使其绕自身的Z轴旋转。
     */
    virtual void update() override;

    virtual QString getBaseName() const override;

private:
    double m_angle; // 用于记录当前旋转角度
};

#endif // DYNAMICOBJECT_H
