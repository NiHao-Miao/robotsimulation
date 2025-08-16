#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <QObject>
#include <QString>
#include <AIS_InteractiveObject.hxx>
#include <gp_Trsf.hxx>


/**
@brief 场景中所有独立物体的抽象基类。
这个类定义了所有场景物体（无论是机器人、静态环境还是动态道具）
都必须具备的通用属性和行为接口。
它确保了所有物体都能被主控制器以统一的方式进行管理、变换和更新。
*/

class SceneObject :public QObject
{
    Q_OBJECT

public:
    /**
    * @brief 构造函数。
    * @param parent Qt的父对象，用于内存管理。
    */
    explicit SceneObject(QObject *parent = nullptr);
    /**
     * @brief 虚析构函数。
     * 确保通过基类指针删除派生类对象时，能够正确调用派生类的析构函数。
     */
    virtual ~SceneObject();

    // --- 核心属性访问接口 ---

    /**
     * @brief 获取物体的唯一标识ID。
     * @return 物体的ID字符串。
     */
    QString getId() const { return m_id; }

    /**
     * @brief 设置物体的唯一标识ID。
     * @param id 新的ID字符串。
     */
    void setId(const QString& id) { m_id = id; }

    /**
     * @brief 获取代表该物体的OpenCASCADE可交互对象。
     * @return AIS_InteractiveObject 的句柄（智能指针）。
     */
    Handle(AIS_InteractiveObject) getAisObject() const { return m_aisObject; }

    /**
     * @brief 获取物体的世界坐标变换矩阵。
     * @return gp_Trsf 变换矩阵。
     */
    gp_Trsf getTransform() const { return m_transform; }   //恢复

    /**
     * @brief 设置物体的世界坐标变换矩阵。
     * @param transform 新的变换矩阵。
     */
    virtual void setTransform(const gp_Trsf& transform)
    {
        m_transform = transform;
        // 确保视觉表现与内部状态同步
        if (!m_aisObject.IsNull()) {
            m_aisObject->SetLocalTransformation(m_transform);
        }
    }  //恢复


    // --- 核心行为接口 (由子类具体实现) ---

    /**
     * @brief [纯虚函数] 加载物体的数据。
     *
     * 子类必须实现此方法来定义如何创建或加载自身的几何模型。
     * @param context OpenCASCADE的显示上下文，用于显示加载后的模型。
     * @param data 一个通用的QVariant，用于传递加载所需的数据（例如，文件路径）。
     * @return 如果加载成功，返回true。
     */
    virtual bool load(const Handle(AIS_InteractiveContext)& context, const QVariant& data) = 0;

    /**
     * @brief [纯虚函数] 每帧更新物体的状态。
     *
     * 子类必须实现此方法来定义其动态行为。
     * 对于静态物体，此方法体可以为空。
     * 对于动态物体（如机器人、旋转体），此方法将包含更新其 m_transform 或内部状态的逻辑。
     */
    virtual void update() = 0;

    /**
     * @brief [纯虚函数] 获取物体的基础名称。
     *
     * 子类必须实现此方法来返回一个用于生成唯一ID的基础名字。
     * 例如，机器人返回配置文件中的名字，桌子可以返回 "Table"。
     * @return 基础名称字符串。
     */
    virtual QString getBaseName() const = 0;


    // --- 新增: “自我清理”的虚函数接口 ---
    /**
     * @brief [虚函数] 将该对象的所有视觉表现从场景中移除。
     *
     * 子类可以重写此方法以实现自定义的清理逻辑，
     * 特别是对于管理多个AIS对象的复杂物体（如机器人）。
     * @param context 需要从中移除对象的显示上下文。
     */
    virtual void removeFromScene(const Handle(AIS_InteractiveContext)& context);



protected:
    QString m_id; // 对象的唯一标识符
    Handle(AIS_InteractiveObject) m_aisObject; // 对象在OCC场景中的视觉代表
    gp_Trsf m_transform; // 对象【相对于其父对象】的【局部】变换矩阵
};


#endif // SCENEOBJECT_H
