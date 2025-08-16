
#include "dynamicobject.h"
#include <gp_Ax1.hxx>
#include <AIS_InteractiveContext.hxx>

DynamicObject::DynamicObject(QObject* parent)
    : SceneObject(parent), m_angle(0.0)
{
}

DynamicObject::~DynamicObject()
{
}

bool DynamicObject::load(const Handle(AIS_InteractiveContext)& context, const QVariant& data)
{
    Q_UNUSED(data);

    // 1. 创建一个 200x200x200 的立方体，中心在原点
    TopoDS_Shape boxShape = BRepPrimAPI_MakeBox(200, 200, 200).Shape();
    Handle(AIS_Shape) aisBox = new AIS_Shape(boxShape);

    // 2. 赋值给基类成员
    m_aisObject = aisBox;
    m_aisObject->SetColor(Quantity_NOC_CORAL);

    // 3. 将初始位置设置在场景的 (1500, 0, 100) 处
    m_transform.SetTranslation(gp_Vec(1500, 0, 100));
    // 调用基类的 setTransform 来正确应用变换
    this->setTransform(m_transform);

    // 4. 显示
    context->Display(m_aisObject, Standard_True);

    return true;
}

void DynamicObject::update()
{
    // 动态行为：每帧都更新旋转角度
    m_angle += 0.01; // 旋转速度
    if (m_angle > 2 * M_PI) {
        m_angle -= 2 * M_PI;
    }

    // 1. 获取当前的平移部分
    gp_Vec currentLocation = m_transform.TranslationPart();

    // 2. 创建一个新的变换矩阵，只包含旋转
    gp_Trsf rotationPart;
    // 绕物体自身的Z轴 (gp_Dir(0, 0, 1)) 旋转
    rotationPart.SetRotation(gp_Ax1(gp::Origin(), gp::DZ()), m_angle);

    // 3. 重新组合成最终的变换矩阵：先旋转，再平移
    m_transform = rotationPart; // 应用旋转
    m_transform.SetTranslationPart(currentLocation); // 再设置回平移

    // 4. 将更新后的变换应用到AIS对象上，使其在屏幕上运动
    if (!m_aisObject.IsNull()) {
        m_aisObject->SetLocalTransformation(this->getTransform());
    }
}

QString DynamicObject::getBaseName() const
{
    return "DynamicCube"; // 返回一个默认的基础名字
}


