/*
*    Copyright (c) 2018 Shing Liu All Rights Reserved.
*
*           File : OccView.cpp
*         Author : Shing Liu(eryar@163.com)
*           Date : 2018-01-08 21:00
*        Version : OpenCASCADE7.2.0 & Qt5.7.1
*
*    Description : Qt widget for OpenCASCADE viewer.
*/

/*
*   在OpenCASCADE中，可以使用gp_Trsf来进行坐标系之间的转换。下面是一些常见的坐标系转换方法：
*
*  将点从全局坐标系转换到局部坐标系：
*  cpp
*      gp_Trsf globalToLocal;
*  globalToLocal.SetTransformation(globalOrigin, localOrigin, localXAxis, localYAxis);
*  gp_Pnt localPoint = globalToLocal.Transformed(globalPoint);
*  这里，globalOrigin是全局坐标系的原点，localOrigin是局部坐标系的原点，localXAxis和localYAxis是局部坐标系的X轴和Y轴方向。通过创建一个变换globalToLocal，然后使用Transformed()方法将全局坐标点globalPoint转换为局部坐标点localPoint。
*
*      将点从局部坐标系转换到全局坐标系：
*      cpp
*      gp_Trsf localToGlobal;
*  localToGlobal.SetTransformation(localOrigin, globalOrigin, globalXAxis, globalYAxis);
*  gp_Pnt globalPoint = localToGlobal.Transformed(localPoint);
*  这里，localOrigin是局部坐标系的原点，globalOrigin是全局坐标系的原点，globalXAxis和globalYAxis是全局坐标系的X轴和Y轴方向。通过创建一个变换localToGlobal，然后使用Transformed()方法将局部坐标点localPoint转换为全局坐标点globalPoint。
*
*      将方向从局部坐标系转换到全局坐标系：
*      cpp
*      gp_Trsf localToGlobal;
*  localToGlobal.SetTransformation(localOrigin, globalOrigin, globalXAxis, globalYAxis);
*  gp_Dir globalDir = localToGlobal.TransformedDirection(localDir);
*  这里，localOrigin是局部坐标系的原点，globalOrigin是全局坐标系的原点，globalXAxis和globalYAxis是全局坐标系的X轴和Y轴方向。通过创建一个变换localToGlobal，然后使用TransformedDirection()方法将局部坐标方向localDir转换为全局坐标方向globalDir。
*
*      请注意，在进行坐标系转换时，确保正确设置各个坐标系的原点和方向，并根据实际情况进行适当的调整。
*/

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

#ifdef WNT
    #include <WNT_Window.hxx>
#elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
    #include <Cocoa_Window.hxx>
#else
    #undef Bool
    #undef CursorShape
    #undef None
    #undef KeyPress
    #undef KeyRelease
    #undef FocusIn
    #undef FocusOut
    #undef FontChange
    #undef Expose
    #include <Xw_Window.hxx>
#endif


static Handle(Graphic3d_GraphicDriver)& GetGraphicDriver()
{
  static Handle(Graphic3d_GraphicDriver) aGraphicDriver;
  return aGraphicDriver;
}



OccView::OccView(QWidget* parent )
    : QWidget(parent),
    myCurrentMode(CurAction3d_DynamicRotation),
    myDegenerateModeIsOn(Standard_True),
    myRectBand(NULL),
    is_edit(false)
{
    // No Background
    setBackgroundRole( QPalette::NoRole );

    // set focus policy to threat QContextMenuEvent from keyboard  
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);

    // Enable the mouse tracking, by default the mouse tracking is disabled.
    setMouseTracking( true );

    init();
}

// 新增getView()的实现
const Handle(V3d_View)& OccView::getView() const
{
    return myView;
}

void OccView::init()
{
    // Create Aspect_DisplayConnection
    Handle(Aspect_DisplayConnection) aDisplayConnection =
            new Aspect_DisplayConnection();

    // Get graphic driver if it exists, otherwise initialise it
    if (GetGraphicDriver().IsNull())
    {
        GetGraphicDriver() = new OpenGl_GraphicDriver(aDisplayConnection);
    }

    // Get window handle. This returns something suitable for all platforms.
    WId window_handle = (WId) winId();

    // Create appropriate window for platform
    #ifdef WNT
        Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle) window_handle);
    #elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
        Handle(Cocoa_Window) wind = new Cocoa_Window((NSView *) window_handle);
    #else
        Handle(Xw_Window) wind = new Xw_Window(aDisplayConnection, (Window) window_handle);
    #endif

    // Create V3dViewer and V3d_View
    myViewer = new V3d_Viewer(GetGraphicDriver());

    myView = myViewer->CreateView();

    myView->SetWindow(wind);
    if (!wind->IsMapped()) wind->Map();

    // Create AISInteractiveContext
    myContext = new AIS_InteractiveContext(myViewer);

    // Set up lights etc
    myViewer->SetDefaultLights();
    myViewer->SetLightOn();
    //< 开启网格
    GridEnable();

    // 再创建一个模型操纵器
    aManipulator = new AIS_Manipulator();

    // 可以用 SetPart 禁用或启用某些轴的平移、旋转或缩放的可视部分
    aManipulator->SetPart(0, AIS_ManipulatorMode::AIS_MM_Scaling, Standard_False);  // 禁用了 X 轴的缩放
    aManipulator->SetPart(1, AIS_ManipulatorMode::AIS_MM_Rotation, Standard_False); // 禁用了 Y 轴的旋转
    // 将操纵器附在创建的长方体上
    //aManipulator->Attach(shape.TShape);
    // 启用指定的操纵模式
    aManipulator->EnableMode(AIS_ManipulatorMode::AIS_MM_Translation);  // 启用移动
    aManipulator->EnableMode(AIS_ManipulatorMode::AIS_MM_Rotation);     // 启用旋转
    //aManipulator->EnableMode(AIS_ManipulatorMode::AIS_MM_Scaling);      // 启用缩放
    // 激活操纵器
    aManipulator->SetModeActivationOnDetection(Standard_True);
    const Handle(AIS_InteractiveContext) myContext1 = myContext;

    myView->SetBackgroundColor(Quantity_NOC_BLACK);
    myView->MustBeResized();
    myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_ZBUFFER);

    myContext->SetDisplayMode(AIS_Shaded, Standard_True);

    //显示坐标
    myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.1, V3d_ZBUFFER);

    //通过提供颜色和填充方法(默认为水平)来定义视图的渐变背景色
//    myView->SetBgGradientColors(Quantity_NOC_GRAY, Quantity_NOC_WHITE, Aspect_GFM_VER);
    this->setGrayBg();

    //视图导航器
    Handle(AIS_ViewCube) aViewCube = new AIS_ViewCube();
    aViewCube->SetTransformPersistence(
        new Graphic3d_TransformPers(
            Graphic3d_TMF_TriedronPers,
            Aspect_TOTP_RIGHT_UPPER,
            Graphic3d_Vec2i(100, 100)));
    //显示
    myContext->Display(aViewCube, Standard_True);

//    point_color = Quantity_Color(Quantity_NOC_YELLOW);

}

const Handle(AIS_InteractiveContext)& OccView::getContext() const
{
    return myContext;
}

/*!
Get paint engine for the OpenGL viewer. [ virtual public ]
*/
QPaintEngine* OccView::paintEngine() const
{
    return 0;
}

void OccView::paintEvent( QPaintEvent* /*theEvent*/ )
{
    myView->Redraw();
}

void OccView::resizeEvent( QResizeEvent* /*theEvent*/ )
{
    if( !myView.IsNull() )
    {
        myView->MustBeResized();
    }
}

void OccView::fitAll( void )
{
    myView->FitAll();
    myView->ZFitAll();
    myView->Redraw();
    emit viewScaleChange(myView->Scale());
}

void OccView::reset( void )
{
    myView->Reset();
    emit viewScaleChange(myView->Scale());

}

///原文链接：https://blog.csdn.net/qq_41810721/article/details/136767541

void OccView::onButtonFrontview()
{
    //前视图
    myView->SetProj(V3d_Yneg);
    myView->FitAll();
    emit viewScaleChange(myView->Scale());
}


void OccView::onButtonRearview()
{
    //后视图
    myView->SetProj(V3d_Ypos);
    myView->FitAll();
    emit viewScaleChange(myView->Scale());
}


void OccView::onMenuLeftview()
{
    //左视图
    myView->SetProj(V3d_Xneg);
    myView->FitAll();
    emit viewScaleChange(myView->Scale());

}


void OccView::onButtonRightview()
{
    //右视图
    myView->SetProj(V3d_Xpos);
    myView->FitAll();
    emit viewScaleChange(myView->Scale());

}


void OccView::onMenuTopview()
{
    //俯视图
    myView->SetProj(V3d_Zpos);
    myView->FitAll();
    emit viewScaleChange(myView->Scale());

}


void OccView::onButtonUpview()
{
    //仰视图
    myView->SetProj(V3d_Zneg);
    myView->FitAll();
    emit viewScaleChange(myView->Scale());

}


void OccView::onMenuAxo()
{
    //轴视图
    myView->SetProj(V3d_XposYnegZpos);
    myView->FitAll();
    emit viewScaleChange(myView->Scale());

}

void OccView::GridEnable()
{
    //< 使用网格
    myViewer->ActivateGrid (Aspect_GT_Rectangular, Aspect_GDM_Lines);
    is_update_view = true;
}

void OccView::GridDisable()
{
    myViewer->DeactivateGrid();
    is_update_view = true;
}

///
/// \brief OccView::setBlackBg  设置黑色背景
///
void OccView::setBlackBg()
{
    myView->SetBgGradientColors(Quantity_NOC_BLACK, Quantity_NOC_BLACK, Aspect_GFM_VER);
    is_update_view = true;
}

///
/// \brief OccView::setGrayBg   设置灰色渐变背景
///
void OccView::setGrayBg()
{
    //通过提供颜色和填充方法(默认为水平)来定义视图的渐变背景色
    myView->SetBgGradientColors(Quantity_NOC_GRAY, Quantity_NOC_WHITE, Aspect_GFM_VER);
    is_update_view = true;
}


void OccView::onButtonWireframemode()
{
    //关闭边界框
    const Handle(Prs3d_Drawer)& aDrawer = getContext()->DefaultDrawer();
    aDrawer->SetFaceBoundaryDraw(false);
    //线框
    getContext()->SetDisplayMode(AIS_WireFrame, false);
    getContext()->Redisplay(AIS_KOI_Shape, -1, false);
    is_update_view = true;
}

//实体
void OccView::onButtonEntitemode()
{
    //关闭边界框
    const Handle(Prs3d_Drawer)& aDrawer = getContext()->DefaultDrawer();
    aDrawer->SetFaceBoundaryDraw(false);
    //实体
    getContext()->SetDisplayMode(AIS_Shaded, false);
    getContext()->Redisplay(AIS_KOI_Shape, -1, false);
    is_update_view = true;
}


void OccView::pan( void )
{
    myCurrentMode = CurAction3d_DynamicPanning;
}

void OccView::zoom( void )
{
    myCurrentMode = CurAction3d_DynamicZooming;
}

void OccView::rotate( void )
{
    myCurrentMode = CurAction3d_DynamicRotation;
}

void OccView::setEditMode(bool mode)
{
    is_edit = mode;
}


// 将弧度转换为度
double radToDeg(double radians) {
    return radians * 180.0 / M_PI;
}

// 计算绕 Y 轴旋转的 gp_Trsf
gp_Trsf createRotationY(double angle) {
    gp_Ax1 rotationAxis(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0)); // 绕 Y 轴旋转
    gp_Trsf rotationTransform;
    rotationTransform.SetRotation(rotationAxis, angle);
    return rotationTransform;
}

// 计算从 gp_Trsf 中提取旋转角度
void OccView::getRotationAngles(const gp_Trsf& transform, double& angleX, double& angleY, double& angleZ) {
    // 获取旋转矩阵
    gp_Mat rotationMatrix = transform.VectorialPart();

    // 提取旋转角度
    angleZ = atan2(rotationMatrix.Value(2, 1), rotationMatrix.Value(1, 1)); // Z 轴旋转
    double tempY = -rotationMatrix.Value(3, 1);
    double tempSqrt = std::sqrt(rotationMatrix.Value(1, 1) * rotationMatrix.Value(1, 1) +
                                 rotationMatrix.Value(2, 1) * rotationMatrix.Value(2, 1));

    if (tempSqrt > std::numeric_limits<double>::epsilon()) {
        angleY = atan2(tempY, tempSqrt); // Y 轴旋转
    } else {
        angleY = (tempY > 0) ? M_PI_2 : -M_PI_2; // ±90 度
    }

    angleX = atan2(rotationMatrix.Value(3, 2), rotationMatrix.Value(3, 3)); // X 轴旋转

    // 将角度从弧度转换为度
    angleX = radToDeg(angleX);
    angleY = radToDeg(angleY);
    angleZ = radToDeg(angleZ);
}

void OccView::makeBox()
{
    TopoDS_Shape aTopoBox = BRepPrimAPI_MakeBox(gp_Pnt(400,0,200),100.0, 200, 300.0).Shape();
    Handle(AIS_Shape) anAisBox = new AIS_Shape(aTopoBox);

    anAisBox->SetColor(Quantity_NOC_BISQUE);

    getContext()->Display(anAisBox, Standard_True);
}

void OccView::makeCone()
{
    gp_Ax2 anAxis;
    anAxis.SetLocation(gp_Pnt(400,0,200));

    TopoDS_Shape aTopoReducer = BRepPrimAPI_MakeCone(anAxis, 100, 0, 300.0).Shape();
    Handle(AIS_Shape) anAisReducer = new AIS_Shape(aTopoReducer);

    anAisReducer->SetColor(Quantity_NOC_BISQUE);


    getContext()->Display(anAisReducer, Standard_True);
}

void OccView::makeSphere()
{
    gp_Ax2 anAxis;
    anAxis.SetLocation(gp_Pnt(400,0,200));

    TopoDS_Shape aTopoSphere = BRepPrimAPI_MakeSphere(anAxis, 3.0).Shape();
    Handle(AIS_Shape) anAisSphere = new AIS_Shape(aTopoSphere);

    anAisSphere->SetColor(Quantity_NOC_BLUE1);

    getContext()->Display(anAisSphere, Standard_True);
}

void OccView::makeCylinder()
{
#if 0
    //< 四轴水平多关节机型
    this->makeCylinder({0,0,300},dir2GpDir(0),dir2GpDir(2),245);
    this->makeCylinder({245,0,300},dir2GpDir(0),dir2GpDir(2),355);
    this->makeCylinder({600,0,300},dir2GpDir(2,-1),dir2GpDir(2),100);
    this->makeCylinder({600,0,200},dir2GpDir(5),dir2GpDir(2),0);

#endif

#if 1
    typedef struct DH_ {
        double alpha;
        double a;
        double d;
        double theta;
    } DH;
    DH SDH[6];
    SDH[0].theta = 0;       SDH[0].d = 400;     SDH[0].a = 70;       SDH[0].alpha =  0.5*PI;
    SDH[1].theta = 0.5*PI;  SDH[1].d = 0;       SDH[1].a = 390;      SDH[1].alpha =  0;
    SDH[2].theta = 0;       SDH[2].d = 0;       SDH[2].a = 117.5;    SDH[2].alpha =  0.5*PI;
    SDH[3].theta = 0;       SDH[3].d = 394;     SDH[3].a = 0;        SDH[3].alpha = -0.5*PI;
    SDH[4].theta = 0;       SDH[4].d = 0;       SDH[4].a = 0;        SDH[4].alpha =  0.5*PI;
    SDH[5].theta = 0;       SDH[5].d = 119;     SDH[5].a = 0;        SDH[5].alpha =  0;


    //< 六轴关节机型
    this->makeCylinder({                             0,  0,  SDH[0].d                      },dir2GpDir(0),dir2GpDir(5),SDH[0].a );
    this->makeCylinder({                      SDH[0].a,  0,  SDH[0].d                      },dir2GpDir(2),dir2GpDir(4),SDH[1].a );
    this->makeCylinder({                      SDH[0].a,  0,  SDH[0].d + SDH[1].a           },dir2GpDir(2),dir2GpDir(4),SDH[2].a);
    this->makeCylinder({                      SDH[0].a,  0,  SDH[0].d + SDH[1].a + SDH[2].a},dir2GpDir(0),dir2GpDir(3),SDH[3].d);
    this->makeCylinder({           SDH[0].a + SDH[3].d,  0,  SDH[0].d + SDH[1].a + SDH[2].a},dir2GpDir(0),dir2GpDir(4),SDH[5].d);
    this->makeCylinder({SDH[0].a + SDH[3].d + SDH[5].d,  0,  SDH[0].d + SDH[1].a + SDH[2].a},dir2GpDir(0),dir2GpDir(3),0);

#endif
    is_update_view = true;
}

gp_Dir OccView::dir2GpDir(int dir, int dir2)
{
    dir = dir%3;
    switch (dir) {
    case 0:
        return gp_Dir(1*dir2,0,0);
    case 1:
        return gp_Dir(0,1*dir2,0);
    case 2:
        return gp_Dir(0,0,1*dir2);
    }
    return gp_Dir();
}

QVector<Handle(AIS_Shape)> OccView::makeCylinder(const gp_Pnt& pnt,const gp_Dir& dir,const gp_Dir& r_dir,double length ,double radius)
{
    QVector<Handle(AIS_Shape)> ret;
    gp_Ax2 anAxis(pnt,dir);

    //< 主体
    TopoDS_Shape aTopoCylinder = BRepPrimAPI_MakeCylinder(anAxis, radius, length).Shape();
    Handle(AIS_Shape) anAisCylinder = new AIS_Shape(aTopoCylinder);

    anAisCylinder->SetColor(Quantity_NOC_BLUE1);
    getContext()->Display(anAisCylinder, Standard_True);

    //< 接头处
    gp_Pnt pnt1 = pnt;
//    if(dir.X() != r_dir.X() || dir.Y() != r_dir.Y()|| dir.Z() != r_dir.Z())
    {
        //< 方向不一致时，需要调整接头位置，保证接头在主体中心
        if(r_dir.Y()){
            pnt1 = gp_Pnt(pnt.X(),pnt.Y()-15,pnt.Z());
        }
        if(r_dir.X()){
            pnt1 = gp_Pnt(pnt.X()-15,pnt.Y(),pnt.Z());
        }
        if(r_dir.Z()){
            pnt1 = gp_Pnt(pnt.X(),pnt.Y(),pnt.Z()-15);
        }
    }

    gp_Ax2 anAxis1(pnt1,r_dir);
    TopoDS_Shape aTopoCylinder1 = BRepPrimAPI_MakeCylinder(anAxis1, radius+20, 30).Shape();
    Handle(AIS_Shape) anAisCylinder1 = new AIS_Shape(aTopoCylinder1);

    anAisCylinder->SetColor(Quantity_NOC_RED1);
    getContext()->Display(anAisCylinder1, Standard_True);

    ret.append(anAisCylinder);
    ret.append(anAisCylinder1);

    return ret;
}

void OccView::makeTorus()
{
    gp_Ax2 anAxis;
    anAxis.SetLocation(gp_Pnt(0.0, 40.0, 0.0));

    TopoDS_Shape aTopoTorus = BRepPrimAPI_MakeTorus(anAxis, 3.0, 1.0).Shape();
    Handle(AIS_Shape) anAisTorus = new AIS_Shape(aTopoTorus);

    anAisTorus->SetColor(Quantity_NOC_YELLOW);

    anAxis.SetLocation(gp_Pnt(8.0, 40.0, 0.0));
    TopoDS_Shape aTopoElbow = BRepPrimAPI_MakeTorus(anAxis, 3.0, 1.0, M_PI_2).Shape();
    Handle(AIS_Shape) anAisElbow = new AIS_Shape(aTopoElbow);

    anAisElbow->SetColor(Quantity_NOC_THISTLE);

    getContext()->Display(anAisTorus, Standard_True);
    getContext()->Display(anAisElbow, Standard_True);
}

void OccView::makeFillet()
{
    gp_Ax2 anAxis;
    anAxis.SetLocation(gp_Pnt(0.0, 50.0, 0.0));

    TopoDS_Shape aTopoBox = BRepPrimAPI_MakeBox(anAxis, 3.0, 4.0, 5.0).Shape();
    BRepFilletAPI_MakeFillet MF(aTopoBox);

    // Add all the edges to fillet.
    for (TopExp_Explorer ex(aTopoBox, TopAbs_EDGE); ex.More(); ex.Next())
    {
        MF.Add(1.0, TopoDS::Edge(ex.Current()));
    }

    Handle(AIS_Shape) anAisShape = new AIS_Shape(MF.Shape());
    anAisShape->SetColor(Quantity_NOC_VIOLET);

    getContext()->Display(anAisShape, Standard_True);
}

void OccView::makeChamfer()
{
    gp_Ax2 anAxis;
    anAxis.SetLocation(gp_Pnt(8.0, 50.0, 0.0));

    TopoDS_Shape aTopoBox = BRepPrimAPI_MakeBox(anAxis, 3.0, 4.0, 5.0).Shape();
    BRepFilletAPI_MakeChamfer MC(aTopoBox);
    TopTools_IndexedDataMapOfShapeListOfShape aEdgeFaceMap;

    TopExp::MapShapesAndAncestors(aTopoBox, TopAbs_EDGE, TopAbs_FACE, aEdgeFaceMap);

    for (Standard_Integer i = 1; i <= aEdgeFaceMap.Extent(); ++i)
    {
        TopoDS_Edge anEdge = TopoDS::Edge(aEdgeFaceMap.FindKey(i));
        TopoDS_Face aFace = TopoDS::Face(aEdgeFaceMap.FindFromIndex(i).First());

        MC.Add(0.6, 0.6, anEdge, aFace);
    }

    Handle(AIS_Shape) anAisShape = new AIS_Shape(MC.Shape());
    anAisShape->SetColor(Quantity_NOC_TOMATO);

    getContext()->Display(anAisShape, Standard_True);
}

void OccView::makeExtrude()
{
    // prism a vertex result is an edge.
    TopoDS_Vertex aVertex = BRepBuilderAPI_MakeVertex(gp_Pnt(0.0, 60.0, 0.0));
    TopoDS_Shape aPrismVertex = BRepPrimAPI_MakePrism(aVertex, gp_Vec(0.0, 0.0, 5.0));
    Handle(AIS_Shape) anAisPrismVertex = new AIS_Shape(aPrismVertex);

    // prism an edge result is a face.
    TopoDS_Edge anEdge = BRepBuilderAPI_MakeEdge(gp_Pnt(5.0, 60.0, 0.0), gp_Pnt(10.0, 60.0, 0.0));
    TopoDS_Shape aPrismEdge = BRepPrimAPI_MakePrism(anEdge, gp_Vec(0.0, 0.0, 5.0));
    Handle(AIS_Shape) anAisPrismEdge = new AIS_Shape(aPrismEdge);

    // prism a wire result is a shell.
    gp_Ax2 anAxis;
    anAxis.SetLocation(gp_Pnt(16.0, 60.0, 0.0));

    TopoDS_Edge aCircleEdge = BRepBuilderAPI_MakeEdge(gp_Circ(anAxis, 3.0));
    TopoDS_Wire aCircleWire = BRepBuilderAPI_MakeWire(aCircleEdge);
    TopoDS_Shape aPrismCircle = BRepPrimAPI_MakePrism(aCircleWire, gp_Vec(0.0, 0.0, 5.0));
    Handle(AIS_Shape) anAisPrismCircle = new AIS_Shape(aPrismCircle);

    // prism a face or a shell result is a solid.
    anAxis.SetLocation(gp_Pnt(24.0, 60.0, 0.0));
    TopoDS_Edge aEllipseEdge = BRepBuilderAPI_MakeEdge(gp_Elips(anAxis, 3.0, 2.0));
    TopoDS_Wire aEllipseWire = BRepBuilderAPI_MakeWire(aEllipseEdge);
    TopoDS_Face aEllipseFace = BRepBuilderAPI_MakeFace(gp_Pln(gp::XOY()), aEllipseWire);
    TopoDS_Shape aPrismEllipse = BRepPrimAPI_MakePrism(aEllipseFace, gp_Vec(0.0, 0.0, 5.0));
    Handle(AIS_Shape) anAisPrismEllipse = new AIS_Shape(aPrismEllipse);

    anAisPrismVertex->SetColor(Quantity_NOC_PAPAYAWHIP);
    anAisPrismEdge->SetColor(Quantity_NOC_PEACHPUFF);
    anAisPrismCircle->SetColor(Quantity_NOC_PERU);
    anAisPrismEllipse->SetColor(Quantity_NOC_PINK);

    getContext()->Display(anAisPrismVertex, Standard_True);
    getContext()->Display(anAisPrismEdge, Standard_True);
    getContext()->Display(anAisPrismCircle, Standard_True);
    getContext()->Display(anAisPrismEllipse, Standard_True);
}

void OccView::makeRevol()
{
    gp_Ax1 anAxis;

    // revol a vertex result is an edge.
    anAxis.SetLocation(gp_Pnt(0.0, 70.0, 0.0));
    TopoDS_Vertex aVertex = BRepBuilderAPI_MakeVertex(gp_Pnt(2.0, 70.0, 0.0));
    TopoDS_Shape aRevolVertex = BRepPrimAPI_MakeRevol(aVertex, anAxis);
    Handle(AIS_Shape) anAisRevolVertex = new AIS_Shape(aRevolVertex);

    // revol an edge result is a face.
    anAxis.SetLocation(gp_Pnt(8.0, 70.0, 0.0));
    TopoDS_Edge anEdge = BRepBuilderAPI_MakeEdge(gp_Pnt(6.0, 70.0, 0.0), gp_Pnt(6.0, 70.0, 5.0));
    TopoDS_Shape aRevolEdge = BRepPrimAPI_MakeRevol(anEdge, anAxis);
    Handle(AIS_Shape) anAisRevolEdge = new AIS_Shape(aRevolEdge);

    // revol a wire result is a shell.
    anAxis.SetLocation(gp_Pnt(20.0, 70.0, 0.0));
    anAxis.SetDirection(gp::DY());

    TopoDS_Edge aCircleEdge = BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(gp_Pnt(15.0, 70.0, 0.0), gp::DZ()), 1.5));
    TopoDS_Wire aCircleWire = BRepBuilderAPI_MakeWire(aCircleEdge);
    TopoDS_Shape aRevolCircle = BRepPrimAPI_MakeRevol(aCircleWire, anAxis, M_PI_2);
    Handle(AIS_Shape) anAisRevolCircle = new AIS_Shape(aRevolCircle);

    // revol a face result is a solid.
    anAxis.SetLocation(gp_Pnt(30.0, 70.0, 0.0));
    anAxis.SetDirection(gp::DY());

    TopoDS_Edge aEllipseEdge = BRepBuilderAPI_MakeEdge(gp_Elips(gp_Ax2(gp_Pnt(25.0, 70.0, 0.0), gp::DZ()), 3.0, 2.0));
    TopoDS_Wire aEllipseWire = BRepBuilderAPI_MakeWire(aEllipseEdge);
    TopoDS_Face aEllipseFace = BRepBuilderAPI_MakeFace(gp_Pln(gp::XOY()), aEllipseWire);
    TopoDS_Shape aRevolEllipse = BRepPrimAPI_MakeRevol(aEllipseFace, anAxis, M_PI_4);
    Handle(AIS_Shape) anAisRevolEllipse = new AIS_Shape(aRevolEllipse);

    anAisRevolVertex->SetColor(Quantity_NOC_LIMEGREEN);
    anAisRevolEdge->SetColor(Quantity_NOC_LINEN);
    anAisRevolCircle->SetColor(Quantity_NOC_MAGENTA1);
    anAisRevolEllipse->SetColor(Quantity_NOC_MAROON);

    getContext()->Display(anAisRevolVertex, Standard_True);
    getContext()->Display(anAisRevolEdge, Standard_True);
    getContext()->Display(anAisRevolCircle, Standard_True);
    getContext()->Display(anAisRevolEllipse, Standard_True);
}

void OccView::makeLoft()
{
    // bottom wire.
    TopoDS_Edge aCircleEdge = BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(gp_Pnt(0.0, 80.0, 0.0), gp::DZ()), 1.5));
    TopoDS_Wire aCircleWire = BRepBuilderAPI_MakeWire(aCircleEdge);

    // top wire.
    BRepBuilderAPI_MakePolygon aPolygon;
    aPolygon.Add(gp_Pnt(-3.0, 77.0, 6.0));
    aPolygon.Add(gp_Pnt(3.0, 77.0, 6.0));
    aPolygon.Add(gp_Pnt(3.0, 83.0, 6.0));
    aPolygon.Add(gp_Pnt(-3.0, 83.0, 6.0));
    aPolygon.Close();

    BRepOffsetAPI_ThruSections aShellGenerator;
    BRepOffsetAPI_ThruSections aSolidGenerator(true);

    aShellGenerator.AddWire(aCircleWire);
    aShellGenerator.AddWire(aPolygon.Wire());

    aSolidGenerator.AddWire(aCircleWire);
    aSolidGenerator.AddWire(aPolygon.Wire());

    // translate the solid.
    gp_Trsf aTrsf;
    aTrsf.SetTranslation(gp_Vec(18.0, 0.0, 0.0));
    BRepBuilderAPI_Transform aTransform(aSolidGenerator.Shape(), aTrsf);

    Handle(AIS_Shape) anAisShell = new AIS_Shape(aShellGenerator.Shape());
    Handle(AIS_Shape) anAisSolid = new AIS_Shape(aTransform.Shape());

    anAisShell->SetColor(Quantity_NOC_OLIVEDRAB);
    anAisSolid->SetColor(Quantity_NOC_PEACHPUFF);

    getContext()->Display(anAisShell, Standard_True);
    getContext()->Display(anAisSolid, Standard_True);
}

void OccView::mousePressEvent( QMouseEvent* theEvent )
{
    if (theEvent->button() == Qt::LeftButton)
    {
        onLButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    }
    else if (theEvent->button() == Qt::MidButton)
    {
        onMButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    }
    else if (theEvent->button() == Qt::RightButton)
    {
        onRButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    }
}

void OccView::mouseReleaseEvent( QMouseEvent* theEvent )
{
    if (theEvent->button() == Qt::LeftButton)
    {
        onLButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
    else if (theEvent->button() == Qt::MidButton)
    {
        onMButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
    else if (theEvent->button() == Qt::RightButton)
    {
        onRButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
}

void OccView::mouseMoveEvent( QMouseEvent * theEvent )
{
    onMouseMove(theEvent->buttons(), theEvent->pos());
}

void OccView::wheelEvent( QWheelEvent * theEvent )
{
    onMouseWheel(theEvent->buttons(), theEvent->delta(), theEvent->pos());
}

void OccView::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    switch (key) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        {
            //< 删除选中
            if(!is_edit)
                return;
            //< 选中的对象
            Handle(AIS_InteractiveObject) selected = myContext->FirstSelectedObject();
            if (selected.IsNull()){
                return;
            }
            getContext()->Remove(selected,true);
            myView->Redraw();
        }
    }

}

void OccView::onLButtonDown( const int /*theFlags*/, const QPoint thePoint )
{

    // Save the current mouse coordinate in min.
    myXmin = thePoint.x();
    myYmin = thePoint.y();
    myXmax = thePoint.x();
    myYmax = thePoint.y();

    //< 操作器
    Handle(AIS_InteractiveObject) selected = myContext->FirstSelectedObject();
    if (!selected.IsNull()) {
        if(!is_edit)
            return;
        if(selected == aManipulator){
            aManipulator->StartTransform(myXmin, myYmin, myView); // 初始化转换，记录起始位置
        }
        else{
            AIS_Manipulator::OptionsForAttach anOptions;
            anOptions.SetAdjustPosition(true);
            anOptions.SetAdjustSize(true);
            anOptions.SetEnableModes(true);
            //m_Manipulator->SetPart (0, AIS_MM_Rotation, true);
            //m_Manipulator->SetPart (1, AIS_Manipulator::Rotation, Standard_False);
            if (aManipulator->IsAttached())
                aManipulator->Detach();
            aManipulator->Attach(selected, anOptions);

            aManipulator->SetModeActivationOnDetection(true);
            if (myContext->IsDisplayed(aManipulator))
                myContext->Remove(aManipulator, false);
            myContext->Display(aManipulator, true);
            aManipulator->EnableMode(AIS_MM_Translation);
            aManipulator->EnableMode(AIS_MM_Rotation);
            //        aManipulator->EnableMode(AIS_MM_Scaling);
            aManipulator->EnableMode(AIS_MM_TranslationPlane);
            myView->Redraw();
            aManipulator_selected = selected;
        }
    }
    else {
        if (myContext->IsDisplayed(aManipulator)) {
            aManipulator->Detach();
            myContext->Remove(aManipulator, false);
            myView->Redraw();
            aManipulator_selected = nullptr;
        }
    }

}

void OccView::onMButtonDown( const int /*theFlags*/, const QPoint thePoint )
{
    // Save the current mouse coordinate in min.
    myXmin = thePoint.x();
    myYmin = thePoint.y();
    myXmax = thePoint.x();
    myYmax = thePoint.y();

    if (myCurrentMode == CurAction3d_DynamicRotation)
    {
        myView->StartRotation(thePoint.x(), thePoint.y());
    }
}

void OccView::onRButtonDown( const int theFlags, const QPoint thePoint )
{
    // Save the current mouse coordinate in min.
    myXmin = thePoint.x();
    myYmin = thePoint.y();
    myXmax = thePoint.x();
    myYmax = thePoint.y();
}

void OccView::onMouseWheel( const int /*theFlags*/, const int theDelta, const QPoint thePoint )
{
    Standard_Integer aFactor = 16;

    Standard_Integer aX = thePoint.x();
    Standard_Integer aY = thePoint.y();

    if (theDelta > 0)
    {
        aX += aFactor;
        aY += aFactor;
    }
    else
    {
        aX -= aFactor;
        aY -= aFactor;
    }

    myView->Zoom(thePoint.x(), thePoint.y(), aX, aY);
    emit viewScaleChange(myView->Scale());

}

void OccView::addItemInPopup( QMenu* /*theMenu*/ )
{
}

void OccView::popup( const int /*x*/, const int /*y*/ )
{
}

void OccView::onLButtonUp( const int theFlags, const QPoint thePoint )
{
    // Hide the QRubberBand
    if (myRectBand)
    {
        myRectBand->hide();
    }

    // Ctrl for multi selection.
    if (thePoint.x() == myXmin && thePoint.y() == myYmin)
    {
        if (theFlags & Qt::ControlModifier)
        {
            multiInputEvent(thePoint.x(), thePoint.y());
        }
        else
        {
            inputEvent(thePoint.x(), thePoint.y());
        }
    }
    if(aManipulator_selected){
//        aManipulator_selected->SetLocalTransformation(aManipulator->StartTransformation());
        aManipulator_selected = nullptr;
    }

}

void OccView::onMButtonUp( const int /*theFlags*/, const QPoint thePoint )
{
    if (thePoint.x() == myXmin && thePoint.y() == myYmin)
    {
        panByMiddleButton(thePoint);
    }
}

void OccView::onRButtonUp( const int /*theFlags*/, const QPoint thePoint )
{
    popup(thePoint.x(), thePoint.y());
}

void OccView::onMouseMove( const int theFlags, const QPoint thePoint )
{
    // Draw the rubber band.
    if (theFlags & Qt::LeftButton)
    {
//        drawRubberBand(myXmin, myYmin, thePoint.x(), thePoint.y());

//        dragEvent(thePoint.x(), thePoint.y());
        if (aManipulator->HasActiveMode())
        {
            aManipulator->Transform(thePoint.x(), thePoint.y(), myView);

            myView->Redraw();
        }
    }

    if (theFlags & Qt::RightButton)
    {
        myView->Pan(thePoint.x() - myXmin, myYmin - thePoint.y());
        //记录当前点
        myXmin = thePoint.x();
        myYmin = thePoint.y();
    }

    // Ctrl for multi selection.
    if (theFlags & Qt::ControlModifier)
    {
        multiMoveEvent(thePoint.x(), thePoint.y());
    }
    else
    {
        moveEvent(thePoint.x(), thePoint.y());
    }

    // Middle button.
    if (theFlags & Qt::MidButton)
    {
        switch (myCurrentMode)
        {
        case CurAction3d_DynamicRotation:
            myView->Rotation(thePoint.x(), thePoint.y());
            break;

        case CurAction3d_DynamicZooming:
            myView->Zoom(myXmin, myYmin, thePoint.x(), thePoint.y());
            break;

        case CurAction3d_DynamicPanning:
            myView->Pan(thePoint.x() - myXmax, myYmax - thePoint.y());
            myXmax = thePoint.x();
            myYmax = thePoint.y();
            break;

         default:
            break;
        }
    }

}

void OccView::dragEvent( const int x, const int y )
{
    myContext->Select(myXmin, myYmin, x, y, myView, Standard_True);

    emit selectionChanged();
}

void OccView::multiDragEvent( const int x, const int y )
{
    myContext->ShiftSelect(myXmin, myYmin, x, y, myView, Standard_True);

    emit selectionChanged();

}

void OccView::inputEvent( const int x, const int y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    myContext->Select(Standard_True);

    emit selectionChanged();
}

void OccView::multiInputEvent( const int x, const int y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    myContext->ShiftSelect(Standard_True);

    emit selectionChanged();
}

void OccView::moveEvent( const int x, const int y )
{
    myContext->MoveTo(x, y, myView, Standard_True);
}

void OccView::multiMoveEvent( const int x, const int y )
{
    myContext->MoveTo(x, y, myView, Standard_True);
}

void OccView::drawRubberBand( const int minX, const int minY, const int maxX, const int maxY )
{
    QRect aRect;

    // Set the rectangle correctly.
    (minX < maxX) ? (aRect.setX(minX)) : (aRect.setX(maxX));
    (minY < maxY) ? (aRect.setY(minY)) : (aRect.setY(maxY));

    aRect.setWidth(abs(maxX - minX));
    aRect.setHeight(abs(maxY - minY));

    if (!myRectBand)
    {
        myRectBand = new QRubberBand(QRubberBand::Rectangle, this);

        // setStyle is important, set to windows style will just draw
        // rectangle frame, otherwise will draw a solid rectangle.
        myRectBand->setStyle(QStyleFactory::create("windows"));
    }

    myRectBand->setGeometry(aRect);
    myRectBand->show();
}

void OccView::panByMiddleButton( const QPoint& thePoint )
{
    Standard_Integer aCenterX = 0;
    Standard_Integer aCenterY = 0;

    QSize aSize = size();

    aCenterX = aSize.width() / 2;
    aCenterY = aSize.height() / 2;

    myView->Pan(aCenterX - thePoint.x(), thePoint.y() - aCenterY);
}

bool OccView::IsCommon(TopoDS_Shape shapeA, TopoDS_Shape shapeB)
{
    if (shapeA.IsNull() || shapeB.IsNull())
    {
        qDebug() << "no model imported";
        return false;
    }

    //没有实时性要求，采用bool求交的方法
    Standard_Boolean bRunParallel;
    Standard_Real aFuzzyValue;
    BRepAlgoAPI_Common aBuilder;

    // perpare the arguments
    TopTools_ListOfShape aLS;
    TopTools_ListOfShape aLT;

    aLS.Append(shapeA);  //xx   m_TppoTargetBody
    aLT.Append(shapeB);  //yy   m_TopoMoveBody

    bRunParallel = Standard_True;
    aFuzzyValue = 2.1e-5;

    // set the arguments
    aBuilder.SetArguments(aLS);
    aBuilder.SetTools(aLT);

    aBuilder.SetRunParallel(bRunParallel);
    aBuilder.SetFuzzyValue(aFuzzyValue);

    // run the algorithm
    aBuilder.Build();
    if (aBuilder.HasErrors())
    {
        std::ofstream fout;
        fout.open("errormessage.txt", std::ios_base::app);
        aBuilder.DumpErrors(fout);
        fout.flush();
        fout.close();
        qDebug() << "error";
        return false;
    }

    // result of the operation aR
    const TopoDS_Shape& aR = aBuilder.Shape();

    Bnd_Box B;
    BRepBndLib::Add(aR, B);

    double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
    if (!B.IsVoid())
    {
        B.Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
    }
    int nb = aR.NbChildren();

    if (!B.IsVoid())   //交集部分的最小包围box存在，则说明碰撞
    {
        std::cout << "common" << std::endl;
        return true;
    }
    else
    {
        std::cout << "no common" << std::endl;
        return false;
    }
}

