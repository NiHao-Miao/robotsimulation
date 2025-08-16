#include "staticobject.h"
#include <AIS_InteractiveContext.hxx>

StaticObject::StaticObject(QObject* parent)
    : SceneObject(parent)
{
    // 可以在这里设置默认ID等
}

StaticObject::~StaticObject()
{
}

bool StaticObject::load(const Handle(AIS_InteractiveContext)& context, const QVariant& data)
{
    Q_UNUSED(data); // 明确表示我们在这个实现中不使用data参数

    // 1. 创建一个几何实体，例如一个代表桌面的长方体
    //    尺寸：长2000，宽1000，高50
    //    位置：中心点在 (0, 0, -25)，使其上表面与Z=0平面齐平
    TopoDS_Shape boxShape = BRepPrimAPI_MakeBox(gp_Pnt(-1000, -500, -50), 2000, 1000, 50).Shape();

    // 2. 将几何实体封装成一个可交互、可显示的 AIS_Shape
    Handle(AIS_Shape) aisBox = new AIS_Shape(boxShape);

    // 3. 将这个 AIS_Shape 赋值给基类的 m_aisObject 成员
    m_aisObject = aisBox;
    m_aisObject->SetColor(Quantity_NOC_BURLYWOOD); // 给桌子一个木头颜色

    // 4. 应用初始变换（虽然是单位矩阵，但这是良好实践）
    m_aisObject->SetLocalTransformation(m_transform);

    // 5. 将对象显示在场景中
    context->Display(m_aisObject, Standard_True);

    return true;
}

void StaticObject::update()
{
    // 静态物体，每帧不需要做任何更新
}

QString StaticObject::getBaseName() const
{
    return "Table"; // 返回一个默认的基础名字
}

