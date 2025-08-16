#ifndef STATICOBJECT_H
#define STATICOBJECT_H

#include "sceneobject.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <AIS_Shape.hxx>

/**
@brief 代表场景中的静态物体。
这种物体被加载后，其位置和姿态不会随时间自动改变。
典型的例子是地面、桌子、墙壁等。
*/
class StaticObject : public SceneObject
{
    Q_OBJECT
public:
    explicit StaticObject(QObject* parent = nullptr);
    virtual ~StaticObject();

    /**
     * @brief 加载一个表示桌子的长方体模型。
     *
     * @param context OpenCASCADE的显示上下文。
     * @param data (未使用) 为了与基类接口保持一致。
     * @return 总是返回true。
     */
    virtual bool load(const Handle(AIS_InteractiveContext)& context, const QVariant& data) override;

    /**
     * @brief 更新状态。对于静态物体，此方法为空。
     */
    virtual void update() override;

    virtual QString getBaseName() const override;

};

#endif // STATICOBJECT_H
