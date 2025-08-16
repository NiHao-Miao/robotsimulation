#include "trajectory.h"
#include <AIS_InteractiveContext.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>

Trajectory::Trajectory(QObject* parent)
: SceneObject(parent),
m_referenceFrameOwner(nullptr) // 默认不跟随任何物体
{
    // 初始化时，创建一个空的几何集合
    m_builder.MakeCompound(m_compound);
    // 将其封装在一个 AIS_Shape 中，并赋值给基类的 m_aisObject
    // 这样，基类的 setTransform 等方法就能操作我们的视觉对象了
    m_aisObject = new AIS_Shape(m_compound);
    m_aisObject->SetColor(Quantity_NOC_GREEN); // 给新轨迹一个醒目的默认颜色
}

Trajectory::~Trajectory()
{
}

bool Trajectory::load(const Handle(AIS_InteractiveContext)& context, const QVariant& data)
{
    Q_UNUSED(data);
    // Trajectory 的加载很简单，就是把自己显示出来
    if (!context.IsNull() && !context->IsDisplayed(m_aisObject)) {
        context->Display(m_aisObject, Standard_False); // Standard_False 表示轨迹默认不可选中
    }
    return true;
}

void Trajectory::update()
{
    // 轨迹的 update 任务，就是计算自己应该显示在哪里
    if (m_referenceFrameOwner != nullptr) {
        // 获取参考坐标系拥有者当前在世界中的位姿
        gp_Trsf ref_transform = m_referenceFrameOwner->getTransform();
        // 将这个位姿应用到我自己的视觉对象上
        m_aisObject->SetLocalTransformation(ref_transform);
    }
    // 如果 m_referenceFrameOwner 是 nullptr，它就保持在世界坐标系原点，其变换是单位矩阵
}

QString Trajectory::getBaseName() const
{
    return "Trajectory";
}



void Trajectory::removeFromScene(const Handle(AIS_InteractiveContext)& context)
{
    // 直接使用基类的默认实现即可，因为它会移除 m_aisObject
    SceneObject::removeFromScene(context);
}

void Trajectory::addPoint(const gp_Pnt& local_point)
{
    m_localPoints.append(local_point);
    m_builder.Add(m_compound, BRepBuilderAPI_MakeVertex(local_point).Vertex());

    // 1. 尝试将通用的 AIS_InteractiveObject 句柄转换为具体的 AIS_Shape 句柄
    Handle(AIS_Shape) trajectoryShape = Handle(AIS_Shape)::DownCast(m_aisObject);

    // 2. 检查转换是否成功，如果不为 Null，则可以安全调用其特有方法
    if (!trajectoryShape.IsNull()) {
        trajectoryShape->SetShape(m_compound);

        // Redisplay 也可以用转换后的句柄，更清晰
        if (!trajectoryShape->GetContext().IsNull()) {
            trajectoryShape->GetContext()->Redisplay(trajectoryShape, true);
        }
    }
}

void Trajectory::clear()
{
    m_localPoints.clear();
    m_builder.MakeCompound(m_compound); // 重新创建一个空的集合
    Handle(AIS_Shape) trajectoryShape = Handle(AIS_Shape)::DownCast(m_aisObject);
    if (!trajectoryShape.IsNull()) {
        trajectoryShape->SetShape(m_compound);

        if (!trajectoryShape->GetContext().IsNull()) {
            trajectoryShape->GetContext()->Redisplay(trajectoryShape, true);
        }
    }
}

void Trajectory::setReferenceFrame(SceneObject* owner)
{
    m_referenceFrameOwner = owner;
    // 当参考系改变时，可能需要对现有轨迹点进行坐标变换，
    // 但为了简化，我们假设在改变参考系之前，轨迹通常是空的。
    // 如果需要支持带点切换参考系，这里的逻辑会更复杂。

    // 立即调用一次 update 来更新位置
    update();
}

void Trajectory::setTrajectoryColor(const Quantity_Color& color)
{
    // 我们直接操作基类的 m_aisObject，因为它持有视觉表现
    if (!m_aisObject.IsNull()) {
        m_aisObject->SetColor(color);

        // 刷新显示以立即看到颜色变化
        if (!m_aisObject->GetContext().IsNull()) {
            m_aisObject->GetContext()->Redisplay(m_aisObject, true);
        }
    }
}

QStringList Trajectory::exportPoints() const
{
    QStringList ret;
    Quantity_Color color;
    if (!m_aisObject.IsNull()) {
        m_aisObject->Color(color);
    }

    for (const auto& pnt : m_localPoints) {
        // 遵循原有的格式，并附加上颜色信息
        ret.append(QString("[%1,%2,%3,%4,%5,%6]")
                       .arg(pnt.X())
                       .arg(pnt.Y())
                       .arg(pnt.Z())
                       .arg(color.Red())
                       .arg(color.Green())
                       .arg(color.Blue()));
    }
    return ret;
}

void Trajectory::importPoints(const QStringList &points)
{
    clear(); // 导入前先清空

    for (const QString& data : points) {
        if (data.trimmed().isEmpty()) continue; // 跳过空行
        // (这部分解析逻辑从 OccView::importRecordTrajectory 完整迁移过来)
        qint32 begin = data.indexOf("[");
        qint32 end = data.indexOf("]");
        if (begin == -1 || end == -1) continue;

        QString tmp = data.mid(begin + 1, end - begin - 1);
        QStringList pos = tmp.split(",");
        if (pos.length() < 3) continue;

        bool ok_x, ok_y, ok_z;
        double x = pos.at(0).toDouble(&ok_x);
        double y = pos.at(1).toDouble(&ok_y);
        double z = pos.at(2).toDouble(&ok_z);

        if (!(ok_x && ok_y && ok_z)) continue;

        addPoint(gp_Pnt(x, y, z));

        if (pos.length() > 5) {
            // (可选) 如果文件中包含颜色信息，可以解析并设置
            // 为了简单，我们暂时让导入的轨迹统一使用默认颜色
        }
    }
}
