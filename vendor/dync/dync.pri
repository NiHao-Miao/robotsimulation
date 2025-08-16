INCLUDEPATH+=$$PWD

HEADERS += \
    $$PWD/incentivetrajectorythread.h \
    $$PWD/intf/dynamics/dynmodelintf.h \
    $$PWD/intf/dynamics/inccurveintf.h \
    $$PWD/intf/optimize/curveoptimizeintf.h


win32: LIBS += -L$${PWD}/lib -lhcdyn-dynamics -lhcdyn-optimize

SOURCES += \
    $$PWD/incentivetrajectorythread.cpp

