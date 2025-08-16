QT       += core gui
 QT += opengl network concurrent sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

VERSION = 1.2.9     # 版本号修改这里

RC_ICONS ="Resources/logo.ico"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

suffix = $${VERSION}
CONFIG(debug, debug|release) {
suffix = debug
}
else{
suffix = release
}
DESTDIR = output/bin_$${suffix}
OBJECTS_DIR = output/temp/temp_$${suffix}
UI_DIR = output/temp/temp_$${suffix}
MOC_DIR = output/temp/temp_$${suffix}
RCC_DIR = output/temp/temp_$${suffix}

#DEFINES += SW_VER=\"$${VERSION}\"
VERSTR = '\\"$${VERSION}\\"'
DEFINES += SW_VER=\"$${VERSTR}\"

DEFINES +=  WNT

#DEFINES += DYNC_LIB

QMAKE_CXXFLAGS += /source-charset:utf-8 /execution-charset:utf-8

include(network/network.pri)
include(uicontrols/uicontrols.pri)
include(control/control.pri)
include(vendor/lua-5.3.5/lua.pri)
include(vendor/qcodeEditor/qcodeEditor.pri)
contains(DEFINES,DYNC_LIB){
    include(vendor/dync/dync.pri)
}
include(vendor/mqtt/qmqtt.pri)

SOURCES += \
#    AIS_ViewCube.cxx \
    object/dynamicobject.cpp \
    object/sceneobject.cpp \
    object/staticobject.cpp \
    object/tool.cpp \
    object/trajectory.cpp \
    uipage/attachmentdialog.cpp \
    uipage/createrobotconfigdialog.cpp \
    uipage/globalconfigdialog.cpp \
    uipage/historyversionlistdialog.cpp \
    uipage/jointconfigitem.cpp \
    uipage/jointcontrol.cpp \
    uipage/jointsetting.cpp \
    main.cpp \
    uipage/moldingtreeview.cpp \
    occQt.cpp \
    occView.cpp \
    uipage/multirecorddialog.cpp \
    uipage/oscilloscopewindow.cpp \
    uipage/robotbase.cpp \
    uipage/robotconfig.cpp \
    uipage/robotinstance.cpp \
    uipage/robotjoints.cpp \
    utils/autoupdate.cpp \
    utils/icutils.cpp \
    utils/mqttmanage.cpp \
    utils/systemsetting.cpp

HEADERS += \
#    AIS_ViewCube.hxx \
    object/dynamicobject.h \
    object/sceneobject.h \
    object/staticobject.h \
    object/tool.h \
    object/trajectory.h \
    uipage/attachmentdialog.h \
    uipage/createrobotconfigdialog.h \
    uipage/globalconfigdialog.h \
    uipage/historyversionlistdialog.h \
    uipage/jointconfigitem.h \
    uipage/jointcontrol.h \
    uipage/jointsetting.h \
    uipage/moldingtreeview.h \
    occQt.h \
    occView.h \
    uipage/multirecorddialog.h \
    uipage/oscilloscopewindow.h \
    uipage/robotbase.h \
    uipage/robotconfig.h \
    uipage/robotinstance.h \
    uipage/robotjoints.h \
    utils/autoupdate.h \
    utils/icutils.h \
    utils/mqttmanage.h \
    utils/systemsetting.h

INCLUDEPATH += $${PWD}/utils $${PWD}/uipage $${PWD}/object

FORMS += \
    uipage/attachmentdialog.ui \
    uipage/createrobotconfigdialog.ui \
    uipage/globalconfigdialog.ui \
    uipage/historyversionlistdialog.ui \
    uipage/incentivetrajectorydialog.ui \
    uipage/incentivetrajectoryrunstate.ui \
    uipage/jointconfigitem.ui \
    uipage/jointcontrol.ui \
    uipage/jointsetting.ui \
    uipage/moldingtreeview.ui \
    occQt.ui \
    uipage/multirecorddialog.ui \
    uipage/oscilloscopewindow.ui \
    uipage/robotbase.ui \
    uipage/robotjoints.ui



contains(DEFINES,DYNC_LIB){
SOURCES += \
    uipage/incentivetrajectorydialog.cpp \
    uipage/incentivetrajectoryrunstate.cpp

HEADERS += \
    uipage/incentivetrajectorydialog.h \
    uipage/incentivetrajectoryrunstate.h
}




#win32: LIBS += -LC:/OpenCASCADE-7.3.0-vc14-64/opencascade-7.3.0/win64/vc14/lib/ \
#-lTKBin \
#-lTKG3d \
#-lTKShHealing \
#-lTKVCAF \
#-lTKBinL \
#-lTKGeomAlgo \
#-lTKStd \
#-lTKView \
#-lTKBinTObj \
#-lTKGeomBase \
#-lTKStdL \
#-lTKViewerTest \
#-lTKBinXCAF \
#-lTKHLR \
#-lTKSTEP \
#-lTKVInspector \
#-lTKBO \
#-lTKIGES \
#-lTKSTEP209 \
#-lTKVRML \
#-lTKBool \
#-lTKIVtk \
#-lTKSTEPAttr \
#-lTKXCAF \
#-lTKBRep \
#-lTKIVtkDraw \
#-lTKSTEPBase \
#-lTKXDEDRAW \
#-lTKCAF \
#-lTKLCAF \
#-lTKSTL \
#-lTKXDEIGES \
#-lTKCDF \
#-lTKMath \
#-lTKTInspector \
#-lTKXDESTEP \
#-lTKD3DHost \
#-lTKMesh \
#-lTKTInspectorAPI \
#-lTKXMesh \
#-lTKDCAF \
#-lTKMeshVS \
#-lTKTObj \
#-lTKXml \
#-lTKDFBrowser \
#-lTKOffset \
#-lTKTObjDRAW \
#-lTKXmlL \
#-lTKDraw \
#-lTKOpenGl \
#-lTKToolsDraw \
#-lTKXmlTObj \
#-lTKernel \
#-lTKPrim \
#-lTKTopAlgo \
#-lTKXmlXCAF \
#-lTKFeat \
#-lTKQADraw \
#-lTKTopTest \
#-lTKXSBase \
#-lTKFillet \
#-lTKService \
#-lTKTreeModel \
#-lTKXSDRAW \
#-lTKG2d \
#-lTKShapeView \
#-lTKV3d


#INCLUDEPATH += C:/OpenCASCADE-7.3.0-vc14-64/opencascade-7.3.0/inc
#DEPENDPATH += C:/OpenCASCADE-7.3.0-vc14-64/opencascade-7.3.0/inc

win32: LIBS += -L$${PWD}/occt-vc143-64/win64/vc14/lib \
-lTKBin \
-lTKDFBrowser \
-lTKOpenGlesTest \
-lTKViewerTest \
-lTKBinL \
-lTKDraw \
-lTKOpenGlTest \
-lTKVInspector \
-lTKBinTObj \
-lTKernel \
-lTKPrim \
-lTKXCAF \
-lTKBinXCAF \
-lTKFeat \
-lTKQADraw \
-lTKXDEDRAW \
-lTKBO \
-lTKFillet \
-lTKRWMesh \
-lTKXMesh \
-lTKBool \
-lTKG2d \
-lTKService \
-lTKXml \
-lTKBRep \
-lTKG3d \
-lTKShapeView \
-lTKXmlL \
-lTKCAF \
-lTKGeomAlgo \
-lTKShHealing \
-lTKXmlTObj \
-lTKCDF \
-lTKGeomBase \
-lTKStd \
-lTKXmlXCAF \
-lTKD3DHost \
-lTKHLR \
-lTKStdL \
-lTKXSBase \
-lTKD3DHostTest \
-lTKIVtk \
-lTKTInspector \
-lTKXSDRAW \
-lTKDCAF \
-lTKIVtkDraw \
-lTKTInspectorAPI \
-lTKXSDRAWDE \
-lTKDE \
-lTKLCAF \
-lTKTObj \
-lTKXSDRAWGLTF \
-lTKDECascade \
-lTKMath \
-lTKTObjDRAW \
-lTKXSDRAWIGES \
-lTKDEGLTF \
-lTKMesh \
-lTKToolsDraw \
-lTKXSDRAWOBJ \
-lTKDEIGES \
-lTKMeshVS \
-lTKTopAlgo \
-lTKXSDRAWPLY \
-lTKDEOBJ \
-lTKMessageModel \
-lTKTopTest \
-lTKXSDRAWSTEP \
-lTKDEPLY \
-lTKMessageView \
-lTKTreeModel \
-lTKXSDRAWSTL \
-lTKDESTEP \
-lTKOffset \
-lTKV3d \
-lTKXSDRAWVRML \
-lTKDESTL \
-lTKOpenGl \
-lTKVCAF \
-lTKDEVRML \
-lTKOpenGles \
-lTKView

INCLUDEPATH += $${PWD}/occt-vc143-64/inc
DEPENDPATH += $${PWD}/occt-vc143-64/inc



RESOURCES += \
    occqt.qrc \
    qss/qss.qrc

win32:QMAKE_POST_LINK += "$${PWD}/copy.bat"

DISTFILES += \
    CHANGELOG
