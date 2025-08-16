// RobotInstance.cpp

#include "robotinstance.h"
#include "luathread.h" //
#include <OpenGl_GraphicDriver.hxx>

#include "occView.h"
#include "occQt.h"
#include "icutils.h"
#include <QMenu>
#include <QMouseEvent>
#include <QRubberBand>
#include <QStyleFactory>

#include <V3d_View.hxx>

#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include "BRepBuilderAPI_Transform.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include <gp_Circ.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <Geom_Circle.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include "AIS_PointCloud.hxx"
#include "gp_Trsf.hxx"
#include "gp_Quaternion.hxx"
#include "BRep_Tool.hxx"
#include "TDataStd_Name.hxx"
#include "TDocStd_Document.hxx"
#include "TDF_ChildIterator.hxx"
#include "TopoDS.hxx"
#include "TopoDS_Builder.hxx"
#include "TopoDS_Solid.hxx"
#include "TopExp.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include "TopExp_Explorer.hxx"
#include "TopoDS_Compound.hxx"
#include "STEPControl_Reader.hxx"
#include "STEPCAFControl_Reader.hxx"
#include "XCAFApp_Application.hxx"
#include "XCAFPrs_AISObject.hxx"
#include "XCAFDoc_DocumentTool.hxx"
#include "XCAFDoc_ShapeTool.hxx"
#include "XCAFDoc_ColorTool.hxx"
#include "Standard_Real.hxx"
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS.hxx>
#include <AIS_Manipulator.hxx>
#include "AIS_ViewCube.hxx"
#include "BRepAlgoAPI_Common.hxx"
#include "BRepBndLib.hxx"
#include "Interface_Static.hxx"
#include "TDataXtd_Position.hxx"
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>
#include <cmath>
#include "systemsetting.h"
#include <BRepLib.hxx>
#include <QMessageBox>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>

#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>

#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>

#include <robotconfig.h>

#define PI (3.1415926)
//计算一个局部变换。这个 gp_Trsf 描述的是“从当前连杆坐标系的原点，移动到下一个连杆坐标系原点”所需要进行的平移和旋转。它是一个相对变换。
gp_Trsf DH::toTrsf(double theta1) {
    //将角度转为弧度计算cos和sin值
    double f = this->alpha * PI / 180;
    double a = this->a;
    double d = this->d;
    double t = (this->theta + theta1) * PI/180;
    double ca = cos(f);
    double sa = sin(f);
    double ckz = cos(t);
    double skz = sin(t);

    //根据DH参数公式填充矩阵的12个值
    double a11 = ckz;
    double a12 = -ca * skz;
    double a13 = sa * skz;
    double a14 = a * ckz;

    double a21 = skz;
    double a22 = ca * ckz;
    double a23 = -sa * ckz;
    double a24 = a * skz;

    double a31 = 0;
    double a32 = sa;
    double a33 = ca;
    double a34 = d;

    gp_Trsf trsf;
    trsf.SetValues(a11, a12, a13, a14, a21, a22, a23, a24, a31, a32, a33, a34);
    return trsf;
}

// --- 构造函数与析构函数 ---
RobotInstance::RobotInstance(QObject *parent)
//    : QObject(parent),
    : SceneObject(parent),
      m_config(nullptr),
      m_luaThread(nullptr)

{
    m_linkWorldTransforms.clear(); // 确保初始状态是空的
    m_recordingTasks.clear(); // 确保任务列表初始为空


}

RobotInstance::~RobotInstance()
{
    // 释放所有动态分配的内存
    delete m_config;
    delete m_luaThread;
    // Handle (智能指针) 会自动管理内存，无需手动delete
}

// --- 核心功能实现 ---

bool RobotInstance::load(const Handle(AIS_InteractiveContext)& context, const QVariant& data)
{
    // 从 QVariant 中提取文件路径
    if (data.type() != QVariant::String) {
        qWarning() << "RobotInstance::load expects a QString file path in QVariant data.";
        return false;
    }
    QString configPath = data.toString();

    // 1. 加载配置文件
    m_config = new RobotConfig(this);
    if (!m_config->loadRobotConfig(configPath))
    {
        QMessageBox::warning(nullptr, "模型加载失败", "配置文件打开失败,请重新启动");
        delete m_config;
        m_config = nullptr;
        return false;
    }

    if (!m_config->getDhEnable() && m_config->absStepFiles().size() == 0) {
        QMessageBox::warning(nullptr, "模型加载失败", "未找到可用的模型文件");
        delete m_config;
        m_config = nullptr;
        return false;
    }
    m_id = m_config->name();
    // 2. 加载关节信息
    m_jointsInfo.clear();
    for (int i = 0; i < m_config->jointNumber(); ++i) {
        m_jointsInfo.append(m_config->getJointInfo(i));
    }
    m_currentJointValues.resize(m_config->jointNumber());
    m_currentJointValues.fill(0.0);

    if (m_luaThread) {
        delete m_luaThread;
    }
    m_luaThread = new LuaThread(this);
    QString lua = m_config->getLuaScript();
    if (lua.size()) {
        m_luaThread->startLua(lua);
    }

    // 3. 加载几何模型 (此部分逻辑完整迁移自 OccView::loadRobot)
    m_jointShapeMap.clear();
    m_robotShapeParts.clear();
    m_robotBase = JointParam(); // 确保基座和工具被重置
    m_robotTool = JointParam();

    // 本次只实现STEP文件加载部分 (即 !getDhEnable() 的情况)
    if (!m_config->getDhEnable()) {
        QStringList filePaths = m_config->absStepFiles();
        for (const QString& filePath : filePaths) {

            // --- Start: STEP文件读取的完整逻辑，逐字迁移自 OccView::loadRobot ---
            QVector<Handle(AIS_Shape)> joint_shapes;
            JointParam param;

            Handle(XCAFApp_Application) anApp = XCAFApp_Application::GetApplication();
            Handle(TDocStd_Document) doc;
            anApp->NewDocument("MDTV-XCAF", doc);

            TCollection_AsciiString aFileName(filePath.toStdString().data());
            STEPCAFControl_Reader aStepReader;
            Interface_Static::SetCVal("read.step.codepage", "GB");
            aStepReader.SetColorMode(true);
            aStepReader.SetNameMode(true);
            aStepReader.SetLayerMode(true);

            IFSelect_ReturnStatus ret = aStepReader.ReadFile(aFileName.ToCString());
            if (ret != IFSelect_RetDone) {
                qDebug() << "open " << aFileName.ToCString() << "failed!error:" << ret;
                continue; // 与原逻辑一致，跳过错误文件
            }
            if (aStepReader.Transfer(doc) == FALSE) {
                continue; // 与原逻辑一致
            }

            Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
            TDF_LabelSequence theShapeLSecond;
            aShapeTool->GetFreeShapes(theShapeLSecond);

            for (const TDF_Label& aSecondLabel : theShapeLSecond) {
                Handle(XCAFPrs_AISObject) aPrs = new XCAFPrs_AISObject(aSecondLabel);
                joint_shapes.append(aPrs);
            }

            param.shape = joint_shapes;
            param.topods_shape = aShapeTool->GetOneShape();
            param.parent_shape = aShapeTool;
            // --- End: STEP文件读取的完整逻辑 ---

            // --- Start: 关联模型到关节/基座/工具的逻辑，逐字迁移自 OccView::loadRobot ---
            QFileInfo fileInfo(filePath);
            QString baseName = fileInfo.baseName();
            quint32 jointNumber = -1; // 保持原有逻辑

            QRegularExpression regex("\\d+$");
            QRegularExpressionMatch match = regex.match(baseName);
            if (match.hasMatch()) {
                jointNumber = match.captured().toUInt();
            }

            if (jointNumber != -1) {
                for (int i = 0; i < 6; i++) {
                    param.joint[i] = 0;
                }
                m_jointShapeMap.insert(jointNumber - 1, param);
            } else if (filePath.indexOf("base") != -1) {
                m_robotBase = param;
            } else if (filePath.indexOf("tool") != -1) {
                m_robotTool = param;
            }
            // --- End: 关联逻辑 ---

            m_robotShapeParts.append(joint_shapes);
        }
    } else {
        // 第3.3步将在这里实现基于DH参数生成模型的逻辑
        gp_Trsf robot_trsf;
        robot_trsf.SetValues(1, 0, 0, 0,
                             0, 1, 0, 0,
                             0, 0, 1, 0);
        gp_Vec3f old_point[6]; // 原文为 gp_Vec3f，与原版保持一致

        int size = m_jointsInfo.size();
        for (int file_id = 0; file_id < size; file_id++) {
            quint32 jointNumber = file_id + 1;
            QVector<Handle(AIS_Shape)> joint_shapes;
            JointParam param;

            if (m_config->getDhParamEnable()) {
                // 根据DH参数计算
                DH SDH;
                double point[6] = {0};
                gp_Dir joint_dir;
                gp_Vec3f joint_vec, next_point[6];

                if (jointNumber == 1) {
                    old_point[jointNumber - 1] = gp_Vec3f(m_jointsInfo.at(jointNumber - 1).x, m_jointsInfo.at(jointNumber - 1).y, m_jointsInfo.at(jointNumber - 1).z);
                }
                SDH.theta = m_jointsInfo.at(jointNumber - 1).dh.theta;
                SDH.d = m_jointsInfo.at(jointNumber - 1).dh.d;
                SDH.a = m_jointsInfo.at(jointNumber - 1).dh.a;
                SDH.alpha = m_jointsInfo.at(jointNumber - 1).dh.alpha;

                gp_Trsf curr_joint_transform = SDH.toTrsf(0);
                robot_trsf = robot_trsf * curr_joint_transform;
                tr2pt(point, robot_trsf);
                next_point[jointNumber - 1] = gp_Vec3f(point[0], point[1], point[2]);
                joint_vec = next_point[jointNumber - 1] - old_point[jointNumber - 1];

                if (m_jointsInfo.at(jointNumber - 1).type < 3) {
                    joint_dir = dir2GpDir(m_jointsInfo.at(jointNumber - 1).type);
                } else {
                    if (jointNumber != m_jointsInfo.size()) {
                        if (joint_vec.Modulus() != 0) {
                            joint_dir = gp_Dir(joint_vec.x(), joint_vec.y(), joint_vec.z());
                        }
                    }
                }

                m_jointsInfo[jointNumber - 1].x = old_point[jointNumber - 1].x();
                m_jointsInfo[jointNumber - 1].y = old_point[jointNumber - 1].y();
                m_jointsInfo[jointNumber - 1].z = old_point[jointNumber - 1].z();

                joint_shapes.append(this->makeCylinder({old_point[jointNumber - 1].x(), old_point[jointNumber - 1].y(), old_point[jointNumber - 1].z()},
                                                       joint_dir, dir2GpDir(m_jointsInfo.at(jointNumber - 1).type), joint_vec.Modulus()));

                if (jointNumber == m_jointsInfo.size()) {
                    joint_shapes.append(this->makeCylinder({old_point[jointNumber - 1].x(), old_point[jointNumber - 1].y(), old_point[jointNumber - 1].z()},
                                                           dir2GpDir(m_jointsInfo.at(jointNumber - 1).type), dir2GpDir(m_jointsInfo.at(jointNumber - 1).type), 0));
                } else {
                    old_point[jointNumber] = next_point[jointNumber - 1];
                }
            } else {
                // 根据坐标点计算
                if (jointNumber < m_jointsInfo.size()) {
                    gp_Vec3f next_joint = gp_Vec3f(m_jointsInfo.at(jointNumber).x, m_jointsInfo.at(jointNumber).y, m_jointsInfo.at(jointNumber).z);
                    gp_Vec3f curr_joint = gp_Vec3f(m_jointsInfo.at(jointNumber - 1).x, m_jointsInfo.at(jointNumber - 1).y, m_jointsInfo.at(jointNumber - 1).z);
                    gp_Vec3f joint_vec = next_joint - curr_joint;

                    double x = m_jointsInfo.at(jointNumber).x - m_jointsInfo.at(jointNumber-1).x;
                    double y = m_jointsInfo.at(jointNumber).y - m_jointsInfo.at(jointNumber-1).y;
                    double z = m_jointsInfo.at(jointNumber).z - m_jointsInfo.at(jointNumber-1).z;
                    gp_Dir joint_dir;

                    if (m_jointsInfo.at(jointNumber - 1).type < 3) {
                        joint_dir = dir2GpDir(m_jointsInfo.at(jointNumber-1).type);
                    } else {
                        if(abs(x) < 0.001 && abs(y) < 0.001 && abs(z) < 0.001 ){
                            joint_dir = gp_Dir(0,0,0.1);
                        } else {
                            joint_dir = gp_Dir(x,y,z);
                        }
                    }
                    joint_shapes.append(this->makeCylinder({m_jointsInfo.at(jointNumber-1).x, m_jointsInfo.at(jointNumber-1).y, m_jointsInfo.at(jointNumber-1).z},
                                                           joint_dir, dir2GpDir(m_jointsInfo.at(jointNumber-1).type), joint_vec.Modulus()));
                }
                if (jointNumber == m_jointsInfo.size()) {
                    joint_shapes.append(this->makeCylinder({m_jointsInfo.at(jointNumber-1).x,m_jointsInfo.at(jointNumber-1).y,m_jointsInfo.at(jointNumber-1).z},
                                                           dir2GpDir(m_jointsInfo.at(jointNumber-1).type),dir2GpDir(m_jointsInfo.at(jointNumber-1).type),0));
                }
            }

            param.shape = joint_shapes;
            for(int i = 0; i < sizeof(param.joint) / sizeof(param.joint[0]); i++){
                param.joint[i] = 0;
            }
            m_jointShapeMap.insert(jointNumber - 1, param);
            m_robotShapeParts.append(joint_shapes);
        }
    }

    // --- Start: 创建TCP坐标系的逻辑，逐字迁移自 OccView::loadRobot ---
    if (!m_jointsInfo.isEmpty()) {
        auto joint = m_jointsInfo.last();
        double x = joint.x;
        double y = joint.y;
        double z = joint.z;
        double length = 200; // 与原设置保持一致

        m_tcpXYZShape.clear();

        TopoDS_Edge edgeX = BRepBuilderAPI_MakeEdge({x, y, z}, {length + x, y, z});
        Handle(AIS_Shape) aisBody_tcp_xaxis = new AIS_Shape(edgeX);
        aisBody_tcp_xaxis->SetColor(Quantity_NOC_RED);
        context->Display(aisBody_tcp_xaxis, Standard_False);
        m_tcpXYZShape.append(aisBody_tcp_xaxis);

        TopoDS_Edge edgeY = BRepBuilderAPI_MakeEdge({x, y, z}, {x, length + y, z});
        Handle(AIS_Shape) aisBody_tcp_yaxis = new AIS_Shape(edgeY);
        aisBody_tcp_yaxis->SetColor(Quantity_NOC_GREEN);
        context->Display(aisBody_tcp_yaxis, Standard_False);
        m_tcpXYZShape.append(aisBody_tcp_yaxis);

        TopoDS_Edge edgeZ = BRepBuilderAPI_MakeEdge({x, y, z}, {x, y, length + z});
        Handle(AIS_Shape) aisBody_tcp_zaxis = new AIS_Shape(edgeZ);
        aisBody_tcp_zaxis->SetColor(Quantity_NOC_BLUE);
        context->Display(aisBody_tcp_zaxis, Standard_False);
        m_tcpXYZShape.append(aisBody_tcp_zaxis);
    }
    // --- End: TCP坐标系创建逻辑 ---

    // ----- 结束：逻辑严格迁移 -----

    // 将从配置文件中读取的名字，设置为基类的ID
    m_id = m_config->name();
    // (在 occQt::loadRobot 中有重命名逻辑，最终ID将在那里被设定)

    // 最后，将所有加载的几何体显示出来
    this->setVisible(true, context);
    return true;
}

/**
 * @brief 更新关节角度值
 * @param joints 关节角度数组
 * 逻辑严格迁移自 OccView::updateJoints
 */
void RobotInstance::updateJoints(const QVector<double>& joints)
{
    // ----- 开始：逻辑严格迁移自 OccView::updateJoints -----
    QVector<double> real_joints = joints;
    // 首先通过Lua脚本处理关节值
    if (m_luaThread && m_luaThread->isRunning()) {
        QVector<double> joints_out;
        if (m_luaThread->updateJoints(joints, &joints_out)) {
            real_joints = joints_out;
        }
    }

    m_currentJointValues = real_joints; // 保存当前关节值

    for (int i = 0; i < real_joints.size(); i++) {
        if (i >= this->m_jointsInfo.size()) {
            break;
        }

        const auto& info = m_jointsInfo.at(i);
        if (!m_jointShapeMap.contains(info.axis)) {
            continue;
        }

        auto& joint = m_jointShapeMap[info.axis];

        //1. 更新关节变量 (平移或旋转值)值
        joint.joint[info.type] = real_joints[i] * info.dir + info.default_origin;

        // 2. 创建这个关节的局部运动变换矩阵
        gp_Trsf joint_local_tr;
        // 2.1 应用平移
        joint_local_tr.SetTranslationPart(gp_Vec(joint.joint[0], joint.joint[1], joint.joint[2]));

        // 2.2 应用旋转 (注意：这里是绕着一个偏移后的点旋转)
        gp_Trsf tr_u, tr_v, tr_w;
        gp_Pnt rot_center(joint.joint[0] + info.x, joint.joint[1] + info.y, joint.joint[2] + info.z);

        tr_u.SetRotation(gp_Ax1(rot_center, gp_Dir(1, 0, 0)), joint.joint[3] / 180 * PI);
        joint_local_tr.Multiply(tr_u);

        tr_v.SetRotation(gp_Ax1(rot_center, gp_Dir(0, 1, 0)), joint.joint[4] / 180 * PI);
        joint_local_tr.Multiply(tr_v);

        tr_w.SetRotation(gp_Ax1(rot_center, gp_Dir(0, 0, 1)), joint.joint[5] / 180 * PI);
        joint_local_tr.Multiply(tr_w);

        // 3. 将计算出的局部运动矩阵存储起来
        joint.tr = joint_local_tr;
    }
    // ----- 结束：逻辑严格迁移 -----
}


void RobotInstance::update()
{
    // ----- 开始：逻辑严格迁移自 OccView::onViewChanged -----
    if (m_jointsInfo.size() == 0) {
        return;
    }

    // --- 新增：在每次更新开始时，清空旧的位姿缓存 ---
    m_linkWorldTransforms.clear();
    m_geometricKeyPoints_World.clear();

    // --- 【核心修改】获取机器人这个SceneObject自身的世界变换 ---
    // 这个变换可能是单位矩阵（如果它在场景根节点下），
    // 也可能是它父节点（如移动平台）的世界变换乘以它自己的局部变换。
    // 这使得机器人可以被“装载”到其他移动物体上。
    gp_Trsf robot_base_world_transform = this->getTransform();
    // 正向运动学计算
    // FK的累积变换矩阵，其起点就是机器人基座的实时世界位姿。
    gp_Trsf cumulative_transform = robot_base_world_transform;
    gp_Trsf robot_tr;
    // a. 应用基座的固定偏移和内部动态变换（由关节控制面板的“基座”部分控制）
    cumulative_transform.Multiply(m_robotBase.base_tr);// 固定偏移
    cumulative_transform.Multiply(m_robotBase.tr); // 用户移动

    // b. 缓存“基座”连杆的世界位姿和其几何关键点
    m_linkWorldTransforms.insert("Base", cumulative_transform);
    // 基座的几何点就是其原点
    m_geometricKeyPoints_World.insert("Base", cumulative_transform.TranslationPart());

    // c. 将计算出的最终世界位姿，应用到基座的视觉模型上
    for (int i = 0; i < m_robotBase.shape.size(); i++) {
        m_robotBase.shape[i]->SetLocalTransformation(cumulative_transform);
    }

    // d. 遍历关节链 关节链变换
    for (int i = 0; i < m_jointShapeMap.size(); i++) {
        // i. 获取变换前的“父”连杆的世界变换，用于计算几何关键点
        gp_Trsf parent_transform = cumulative_transform;
        // ii. 正常进行累积变换，得到当前连杆坐标系的世界变换
        auto& joint = m_jointShapeMap[i]; // 使用引用以确保能修改
        // 关键：在上一步结果的基础上，继续累乘
        cumulative_transform.Multiply(joint.base_tr);// 连杆间的固定偏移
        cumulative_transform.Multiply(joint.tr);// 关节电机运动
        // iii. 缓存当前连杆坐标系的世界位姿
        // 我们用 "Link" + 关节编号 (从1开始) 作为ID
        QString linkId = QString("Link %1").arg(i + 1);
        m_linkWorldTransforms.insert(linkId, cumulative_transform);

        // v. 将最终的世界位姿，应用到当前连杆的视觉模型上
        for (int j = 0; j < joint.shape.size(); j++) {
            joint.shape[j]->SetLocalTransformation(cumulative_transform);
        }
        // 4. 计算并缓存“几何关键点”的世界坐标
        const auto& jointInfo = m_jointsInfo.at(i);
        // a. 获取在配置文件中定义的、相对于“父”坐标系的局部几何点
        gp_Pnt keyPoint_local(jointInfo.x, jointInfo.y, jointInfo.z);
        // b. 用“父”坐标系的变换，将其转换到世界坐标系
        gp_Pnt keyPoint_world = keyPoint_local.Transformed(parent_transform);
        // c. 以一个清晰的ID存入缓存
        QString keyPointId = QString("Joint %1").arg(i + 1); // 我们称之为 "Joint X"
        m_geometricKeyPoints_World.insert(keyPointId, keyPoint_world);
    }

    // 对工具和TCP坐标系做同样的操作
    // 工具变换
    cumulative_transform.Multiply(m_robotTool.base_tr);
    cumulative_transform.Multiply(m_robotTool.tr);
    // --- 记录“工具”的位姿 ---
    m_linkWorldTransforms.insert("Tool", cumulative_transform);
    for (int i = 0; i < m_robotTool.shape.size(); i++) {
        m_robotTool.shape[i]->SetLocalTransformation(cumulative_transform);
    }

    // TCP坐标系变换
    for (int i = 0; i < m_tcpXYZShape.size(); i++) {
        m_tcpXYZShape[i]->SetLocalTransformation(cumulative_transform);
    }

    // 更新并存储当前TCP的世界坐标和姿态
    gp_Pnt tcp_pnt = gp_Pnt(m_jointsInfo.last().x, m_jointsInfo.last().y, m_jointsInfo.last().z);
    gp_Pnt tcp_world = tcp_pnt.Transformed(cumulative_transform);//使用最终的累积变换，核心
    m_geometricKeyPoints_World.insert("Tool TCP", tcp_world);
    m_currentTcpPos.x = tcp_world.X();
    m_currentTcpPos.y = tcp_world.Y();
    m_currentTcpPos.z = tcp_world.Z();

    gp_Trsf final_trsf_for_angle = cumulative_transform;
    if (m_jointShapeMap.size() >= 6) {
        gp_Trsf rotationTransform;
        rotationTransform.SetRotation(gp_Ax1(gp::Origin(), gp::DY()), M_PI / 2.0);
        final_trsf_for_angle = rotationTransform * robot_tr;
    }
    getRotationAngles(final_trsf_for_angle, m_currentTcpPos.rx, m_currentTcpPos.ry, m_currentTcpPos.rz);
    // 检查当前是否有任何记录任务在进行
    if (!m_recordingTasks.isEmpty()) {
        // 遍历每一个独立的记录任务
        for (const auto& task : m_recordingTasks) {

            // 安全检查：确保任务所需的数据都有效
            if (task.targetTrajectory == nullptr || !m_geometricKeyPoints_World.contains(task.trackedLinkId)) {
                continue; // 如果数据无效，跳过这个任务
            }

            // 1. 从缓存中获取被追踪点的【世界坐标】
            gp_Pnt tracked_point_world = m_geometricKeyPoints_World.value(task.trackedLinkId);

            // 2. 获取该任务的目标轨迹对象，以及它的显示参考系
            Trajectory* targetTraj = task.targetTrajectory;
            SceneObject* frameOwner = targetTraj->getReferenceFrameOwner();

            // 3. 准备坐标变换矩阵
            gp_Trsf frame_transform; // 默认是单位矩阵 (世界坐标系)
            if (frameOwner) {
                frame_transform = frameOwner->getTransform();
            }

            // 4. 计算出相对于参考系的【局部坐标】
            gp_Pnt local_point = tracked_point_world.Transformed(frame_transform.Inverted());

            // 5. 将这个局部坐标点，添加到它对应的轨迹对象中
            targetTraj->addPoint(local_point);
        }
    }

    /*  `***数学原理**：这个过程等价于矩阵链式乘法：
        `T_final_J3 = T_base * T_J1_offset * T_J1_move * T_J2_offset * T_J2_move * T_J3_offset * T_J3_move`
        `robot_tr` 这个变量就像一个累加器，它一步步地从基座开始，把每个连杆的变换乘上去，最终得到当前连杆相对于世界坐标系的总变换。
    */
}

QList<TrackableLink> RobotInstance::getTrackableLinks() const
{
    QList<TrackableLink> links;

    // 遍历所有已计算的几何关键点
    for (auto it = m_geometricKeyPoints_World.constBegin(); it != m_geometricKeyPoints_World.constEnd(); ++it) {
        // 我们需要为每个点创建一个临时的变换矩阵
        gp_Trsf point_transform;
        point_transform.SetTranslation(it.value().XYZ());
        links.append({it.key(), point_transform});
    }

    return links;
}

/**
 * @brief 更新基座位姿
 * @param joints 6D位姿 (x, y, z, rx, ry, rz)
 * 逻辑严格迁移自 OccView::updateBase
 */
void RobotInstance::updateBase(const QVector<double>& joints)
{
    // ----- 开始：逻辑严格迁移自 OccView::updateBase -----
    if (joints.size() != 6) return;
    gp_Trsf robot_tr;
    robot_tr.SetTranslationPart(gp_Vec(joints[0], joints[1], joints[2]));
    gp_Trsf tr_u, tr_v, tr_w;
    gp_Pnt center(joints[0], joints[1], joints[2]);
    tr_u.SetRotation(gp_Ax1(center, gp_Dir(1, 0, 0)), joints[3] / 180 * PI);
    robot_tr.Multiply(tr_u);
    tr_v.SetRotation(gp_Ax1(center, gp_Dir(0, 1, 0)), joints[4] / 180 * PI);
    robot_tr.Multiply(tr_v);
    tr_w.SetRotation(gp_Ax1(center, gp_Dir(0, 0, 1)), joints[5] / 180 * PI);
    robot_tr.Multiply(tr_w);
    m_robotBase.tr = robot_tr;
    // ----- 结束：逻辑严格迁移 -----
}

/**
 * @brief 更新工具位姿
 * @param joints 6D位姿 (x, y, z, rx, ry, rz)
 * 逻辑严格迁移自 OccView::updateTool
 */
void RobotInstance::updateTool(const QVector<double>& joints)
{
    // ----- 开始：逻辑严格迁移自 OccView::updateTool -----
    if (joints.size() != 6) return;
    gp_Trsf robot_tr;
    robot_tr.SetTranslationPart(gp_Vec(joints[0], joints[1], joints[2]));
    gp_Trsf tr_u, tr_v, tr_w;
    gp_Pnt center(joints[0], joints[1], joints[2]);
    tr_u.SetRotation(gp_Ax1(center, gp_Dir(1, 0, 0)), joints[3] / 180 * PI);
    robot_tr.Multiply(tr_u);
    tr_v.SetRotation(gp_Ax1(center, gp_Dir(0, 1, 0)), joints[4] / 180 * PI);
    robot_tr.Multiply(tr_v);
    tr_w.SetRotation(gp_Ax1(center, gp_Dir(0, 0, 1)), joints[5] / 180 * PI);
    robot_tr.Multiply(tr_w);
    m_robotTool.tr = robot_tr;
    // ----- 结束：逻辑严格迁移 -----
}

// updateJointBase 的完整实现 ---
void RobotInstance::updateJointBase(int id, const QVector<double>& pose)
{
    // ----- 开始：逻辑严格迁移自原 OccView::updateJointBase -----
    if (pose.size() != 6) {
        return;
    }

    if (!m_jointShapeMap.contains(id)) {
        return;
    }

    gp_Trsf joint_base_tr;
    joint_base_tr.SetTranslationPart(gp_Vec(pose[0], pose[1], pose[2]));

    gp_Trsf tr_u;
    tr_u.SetRotation(gp_Ax1(gp_Pnt(pose[0], pose[1], pose[2]), gp_Dir(1, 0, 0)), pose[3] / 180 * PI);
    joint_base_tr.Multiply(tr_u);

    gp_Trsf tr_v;
    tr_v.SetRotation(gp_Ax1(gp_Pnt(pose[0], pose[1], pose[2]), gp_Dir(0, 1, 0)), pose[4] / 180 * PI);
    joint_base_tr.Multiply(tr_v);

    gp_Trsf tr_w;
    tr_w.SetRotation(gp_Ax1(gp_Pnt(pose[0], pose[1], pose[2]), gp_Dir(0, 0, 1)), pose[5] / 180 * PI);
    joint_base_tr.Multiply(tr_w);

    m_jointShapeMap[id].base_tr = joint_base_tr;
    // ----- 结束：逻辑严格迁移 -----
}

bool RobotInstance::addRecordingTask(const QString& trackedLinkId, Trajectory* targetTrajectory)
{
    if (trackedLinkId.isEmpty() || targetTrajectory == nullptr) {
        qWarning() << "addRecordingTask failed: Invalid trackedLinkId or null targetTrajectory.";
        return false;
    }

    // 检查是否已经存在针对同一个 Trajectory 的任务，避免重复
    for (const auto& task : m_recordingTasks) {
        if (task.targetTrajectory == targetTrajectory) {
            // 可以选择更新任务，或者直接返回
            return true;
        }
    }

    m_recordingTasks.append({trackedLinkId, targetTrajectory});
    return true;
}

void RobotInstance::removeRecordingTask(Trajectory* targetTrajectory)
{
    if (targetTrajectory == nullptr) return;

    for (int i = 0; i < m_recordingTasks.size(); ++i) {
        if (m_recordingTasks[i].targetTrajectory == targetTrajectory) {
            m_recordingTasks.removeAt(i);
            // 如果可能有多条任务指向同一个轨迹（逻辑上不应该），则不return继续查找
            // 但按设计一个轨迹只被一个任务写入，所以可以直接返回
            return;
        }
    }
}

void RobotInstance::clearAllRecordingTasks()
{
    m_recordingTasks.clear();
}

bool RobotInstance::isRecording() const
{
    return !m_recordingTasks.isEmpty();
}

void RobotInstance::addWorldPointToAllTasks(const gp_Pnt& world_point)
{
    // 这个函数的逻辑与 update() 中的记录逻辑几乎完全一样
    if (isRecording()) {
        for (const auto& task : m_recordingTasks) {
            if (task.targetTrajectory != nullptr) {
                SceneObject* frameOwner = task.targetTrajectory->getReferenceFrameOwner();
                gp_Trsf frame_transform;
                if (frameOwner) {
                    frame_transform = frameOwner->getTransform();
                }
                gp_Pnt local_point = world_point.Transformed(frame_transform.Inverted());
                task.targetTrajectory->addPoint(local_point);
            }
        }
    }
}

QString RobotInstance::getBaseName() const
{
    if (m_config && m_config->isLoad()) {
        return m_config->name();
    }
    return "Robot"; // 如果配置加载失败，返回一个默认名
}

void RobotInstance::setTransform(const gp_Trsf& transform)
{
    // 将新的变换直接应用到机器人基座的“动态”变换部分
    m_robotBase.tr = transform;

}

void RobotInstance::removeFromScene(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull()) {
        return;
    }

    // --- 核心修正：严格遵循原有逻辑，使用 getAllShapes() ---

        // 1. 获取该机器人实例所有的视觉部件
        //    这个函数封装了获取所有部件的逻辑，确保不会有遗漏。
        QVector<Handle(AIS_Shape)> allShapes = this->getAllShapes();

        // 2. 遍历并从场景中移除它们
        for(const auto& shape : allShapes) {
            if (!shape.IsNull() && context->IsDisplayed(shape)) {
                context->Remove(shape, false); // 使用 false 避免每次都重绘
            }
        }

        // 4. 所有对象都标记移除后，统一刷新一次视图
        context->UpdateCurrentViewer();
}


void RobotInstance::setId(const QString &newId)
{
    this->m_id = newId;
}

// --- 数据访问实现 ---


QVector<Handle(AIS_Shape)> RobotInstance::getAllShapes() const {
    QVector<Handle(AIS_Shape)> all_shapes;
    for (const auto& part : m_robotShapeParts) {
        all_shapes += part;
    }
    all_shapes += m_tcpXYZShape;
    return all_shapes;
}

void RobotInstance::setVisible(bool visible, const Handle(AIS_InteractiveContext)& context) {
    if (context.IsNull()) return;
    QVector<Handle(AIS_Shape)> shapes = this->getAllShapes();
    for (const auto& shape : shapes) {
        if (visible) {
            if (!context->IsDisplayed(shape)) context->Display(shape, Standard_False);
        } else {
            if (context->IsDisplayed(shape)) context->Remove(shape, Standard_False);
        }
    }
}

// --- 私有辅助函数 (全部迁移自 OccView.cpp) ---
QVector<double> RobotInstance::getCurrentJointValues() const
{
    return m_currentJointValues;
}

const QList<RecordingTask> &RobotInstance::getRecordingTasks() const { return m_recordingTasks; }
QString RobotInstance::getId() const { return m_id; }
RobotConfig* RobotInstance::getConfig() const { return m_config; }

BOOL RobotInstance::tr2pt(double* pt, const gp_Trsf& tr)
{
    // ----- 开始：逻辑严格迁移自 OccView::tr2pt -----
    double Fmin = 1e-5;
    double r11, r12, r21, r22, r31, r32, r33;
    double az, ay, ax;

    r11 = tr.HVectorialPart().Column(0).X();
    r21 = tr.HVectorialPart().Column(0).Y();
    r31 = tr.HVectorialPart().Column(0).Z();
    r12 = tr.HVectorialPart().Column(1).X();
    r22 = tr.HVectorialPart().Column(1).Y();
    r32 = tr.HVectorialPart().Column(1).Z();
    r33 = tr.HVectorialPart().Column(2).Z();

    pt[0] = tr.TranslationPart().X();
    pt[1] = tr.TranslationPart().Y();
    pt[2] = tr.TranslationPart().Z();

    if (r31 > (1 - Fmin)) {
        ax = -atan2(r12, r22) * (180. / PI);
        ay = -90.;
        az = 0;
    } else if (r31 < (Fmin - 1)) {
        ax = atan2(r12, r22) * (180. / PI);
        ay = 90.;
        az = 0;
    } else {
        ax = atan2(r32, r33) * (180. / PI);
        ay = -asin(r31) * (180. / PI);
        az = atan2(r21, r11) * (180. / PI);
    }
    pt[3] = ax;
    pt[4] = ay;
    pt[5] = az;

    return TRUE;
    // ----- 结束：逻辑严格迁移 -----
}

void RobotInstance::getRotationAngles(const gp_Trsf& transform, double& angleX, double& angleY, double& angleZ)
{
    gp_Mat rotationMatrix = transform.VectorialPart();
    angleZ = atan2(rotationMatrix.Value(2, 1), rotationMatrix.Value(1, 1));
    double tempY = -rotationMatrix.Value(3, 1);
    double tempSqrt = std::sqrt(rotationMatrix.Value(1, 1) * rotationMatrix.Value(1, 1) + rotationMatrix.Value(2, 1) * rotationMatrix.Value(2, 1));
    if (tempSqrt > std::numeric_limits<double>::epsilon()) {
        angleY = atan2(tempY, tempSqrt);
    } else {
        angleY = (tempY > 0) ? M_PI_2 : -M_PI_2;
    }
    angleX = atan2(rotationMatrix.Value(3, 2), rotationMatrix.Value(3, 3));
    auto radToDeg = [](double rad) { return rad * 180.0 / PI; };
    angleX = radToDeg(angleX);
    angleY = radToDeg(angleY);
    angleZ = radToDeg(angleZ);
    // ----- 结束：逻辑严格迁移 -----
}

gp_Dir RobotInstance::dir2GpDir(int dir, int dir2)
{
    // ----- 开始：逻辑严格迁移自 OccView::dir2GpDir -----
    dir = dir % 3;
    switch (dir) {
    case 0:
        return gp_Dir(1 * dir2, 0, 0);
    case 1:
        return gp_Dir(0, 1 * dir2, 0);
    case 2:
        return gp_Dir(0, 0, 1 * dir2);
    }
    return gp_Dir();
    // ----- 结束：逻辑严格迁移 -----
}

QVector<Handle(AIS_Shape)> RobotInstance::makeCylinder(const gp_Pnt& pnt, const gp_Dir& dir, const gp_Dir& r_dir, double length, double radius)
{
    // ----- 开始：逻辑严格迁移自 OccView::makeCylinder -----
    QVector<Handle(AIS_Shape)> ret;
    gp_Ax2 anAxis(pnt, dir);

    // 主体
    TopoDS_Shape aTopoCylinder = BRepPrimAPI_MakeCylinder(anAxis, radius, length).Shape();
    Handle(AIS_Shape) anAisCylinder = new AIS_Shape(aTopoCylinder);
    anAisCylinder->SetColor(Quantity_NOC_BLUE1);
    ret.append(anAisCylinder);

    // 接头处
    gp_Pnt pnt1 = pnt;
    if (r_dir.Y()) {
        pnt1 = gp_Pnt(pnt.X(), pnt.Y() - 15, pnt.Z());
    }
    if (r_dir.X()) {
        pnt1 = gp_Pnt(pnt.X() - 15, pnt.Y(), pnt.Z());
    }
    if (r_dir.Z()) {
        pnt1 = gp_Pnt(pnt.X(), pnt.Y(), pnt.Z() - 15);
    }

    gp_Ax2 anAxis1(pnt1, r_dir);
    TopoDS_Shape aTopoCylinder1 = BRepPrimAPI_MakeCylinder(anAxis1, radius + 20, 30).Shape();
    Handle(AIS_Shape) anAisCylinder1 = new AIS_Shape(aTopoCylinder1);
    anAisCylinder1->SetColor(Quantity_NOC_RED1); // 原代码为anAisCylinder->SetColor，已修正为anAisCylinder1
    ret.append(anAisCylinder1);

    return ret;
    // ----- 结束：逻辑严格迁移 -----
}
ROBOT_POS RobotInstance::getCurrentTcpPos() const
{
    return m_currentTcpPos;
}

