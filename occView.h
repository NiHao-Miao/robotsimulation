/*
*    Copyright (c) 2018 Shing Liu All Rights Reserved.
*
*           File : OccView.h
*         Author : Shing Liu(eryar@163.com)
*           Date : 2018-01-09 21:00
*        Version : OpenCASCADE7.2.0 & Qt5.7.1
*
*    Description : Adapte OpenCASCADE view for Qt.
*/

#ifndef _OCCVIEW_H_
#define _OCCVIEW_H_

#include <QGLWidget>
#include <QThread>
#include <QTimer>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <XCAFPrs_AISObject.hxx>
#include <AIS_Manipulator.hxx>
#include <V3d_View.hxx>

#include "robotconfig.h"
#include "luathread.h"
#include "robotinstance.h"
class QMenu;
class QRubberBand;


//! Adapted a QWidget for OpenCASCADE viewer.
class OccView : public QWidget
{
    Q_OBJECT

public:
    //! mouse actions.
    enum CurrentAction3d
    {
        CurAction3d_Nothing,
        CurAction3d_DynamicZooming,
        CurAction3d_WindowZooming,
        CurAction3d_DynamicPanning,
        CurAction3d_GlobalPanning,
        CurAction3d_DynamicRotation
    };

public:
    //! constructor.
    OccView(QWidget* parent);


    const Handle(AIS_InteractiveContext)& getContext() const;
    const Handle(V3d_View)& getView() const;


signals:
    void selectionChanged(void);

    ///
    /// \brief addPoint 添加个点
    /// \param dis
    ///
    void addPoint(double dis);

    ///
    /// \brief robotPosUpdate  机器人世界坐标更新
    ///
    void robotPosUpdate(const ROBOT_POS&);

    ///
    /// \brief viewScaleChange   视图缩放大小改变
    ///
    void viewScaleChange(double);

public slots:
    //! operations for the view.

    void pan(void);
    void fitAll(void);
    void reset(void);
    void zoom(void);
    void rotate(void);

    void setEditMode(bool mode);

    // 视图切换
    void onButtonFrontview();
    void onButtonRearview();
    void onMenuLeftview();
    void onButtonRightview();
    void onMenuTopview();
    void onButtonUpview();
    void onMenuAxo();

    //< 背景网格
    void GridEnable();
    void GridDisable();

    //< 设置背景
    void setBlackBg();
    void setGrayBg();

    //< 线框模式
    void onButtonWireframemode();
    //< 实体模式
    void onButtonEntitemode();

    //! make box test.
    void makeBox(void);

    //! make cone test.
    void makeCone(void);

    //! make sphere test.
    void makeSphere(void);

    //! make cylinder test.
    //! 创建圆柱体
    void makeCylinder(void);

    ///
    /// \brief dir2GpDir
    /// \param dir      轴方向
    /// \param dir2     正负方向
    /// \return
    ///
    gp_Dir dir2GpDir(int dir,int dir2 = 1);
    ///
    /// \brief makeCylinder  创建圆柱体
    /// \param pnt          旋转中心
    /// \param dir          手臂方向
    /// \param r_dir        旋转方向
    /// \param length       手臂长
    /// \param radius       手臂半径
    ///
    QVector<Handle(AIS_Shape)> makeCylinder(const gp_Pnt& pnt,const gp_Dir& dir,const gp_Dir& r_dir,double length = 500,double radius = 10);

    //! make torus test.
    void makeTorus(void);

    //! fillet test.
    void makeFillet(void);

    //! chamfer test.
    void makeChamfer(void);

    //! test extrude algorithm.
    void makeExtrude(void);

    //! test revol algorithm.
    void makeRevol(void);

    //! test loft algorithm.
    void makeLoft(void);


    void getRotationAngles(const gp_Trsf &transform, double &angleX, double &angleY, double &angleZ);

protected:
    virtual QPaintEngine* paintEngine() const;

    // Paint events.
    virtual void paintEvent(QPaintEvent* theEvent);
    virtual void resizeEvent(QResizeEvent* theEvent);

    // Mouse events.
    virtual void mousePressEvent(QMouseEvent* theEvent);
    virtual void mouseReleaseEvent(QMouseEvent* theEvent);
    virtual void mouseMoveEvent(QMouseEvent * theEvent);
    virtual void wheelEvent(QWheelEvent * theEvent);

    // Key events
    virtual void keyPressEvent(QKeyEvent *event);

    // Button events.
    virtual void onLButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onMButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onRButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onMouseWheel(const int theFlags, const int theDelta, const QPoint thePoint);
    virtual void onLButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onMButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onRButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onMouseMove(const int theFlags, const QPoint thePoint);

    // Popup menu.
    virtual void addItemInPopup(QMenu* theMenu);

protected:
    void init(void);
    void popup(const int x, const int y);
    void dragEvent(const int x, const int y);
    void inputEvent(const int x, const int y);
    void moveEvent(const int x, const int y);
    void multiMoveEvent(const int x, const int y);
    void multiDragEvent(const int x, const int y);
    void multiInputEvent(const int x, const int y);
    void drawRubberBand(const int minX, const int minY, const int maxX, const int maxY);
    void panByMiddleButton(const QPoint& thePoint);

private:

    bool IsCommon(TopoDS_Shape shapeA, TopoDS_Shape shapeB);

    //! the occ viewer.
    Handle(V3d_Viewer) myViewer;

    //! the occ view.
    Handle(V3d_View) myView;

    //! the occ context.
    Handle(AIS_InteractiveContext) myContext;

    //! save the mouse position.
    Standard_Integer myXmin;
    Standard_Integer myYmin;
    Standard_Integer myXmax;
    Standard_Integer myYmax;

    //! the mouse current mode.
    CurrentAction3d myCurrentMode;

    //! save the degenerate mode state.
    Standard_Boolean myDegenerateModeIsOn;

    //! rubber rectangle for the mouse selection.
    QRubberBand* myRectBand;


private:
    // 再创建一个模型操纵器
    Handle(AIS_Manipulator) aManipulator;
    Handle(AIS_InteractiveObject) aManipulator_selected;//< 模型操作器对应的对象；
    QVector<QVector<Handle(AIS_Shape)>> robot_shape; //< 保存机器人模型所有的对象
    QVector<Handle(AIS_Shape)> tcp_xyz_shape;        //< 绘制末端坐标系
    QTimer update_timer;
    bool is_update_view;
    bool is_edit;
};

#endif // _OCCVIEW_H_
