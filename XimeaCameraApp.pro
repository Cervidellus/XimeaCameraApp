QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix|win32: LIBS += -L$$PWD/../../../../../XIMEA/API/xiAPI/ -lxiapi64

INCLUDEPATH += $$PWD/../../../../../XIMEA/API/xiAPI
DEPENDPATH += $$PWD/../../../../../XIMEA/API/xiAPI

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../../../XIMEA/API/xiAPI/xiapi64.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../../../../XIMEA/API/xiAPI/libxiapi64.a

#  LIBS += -LC:\opencv_build\install\x86\vqtc15\lib -lopencv_core410d -lopencv_imgproc410d -lopencv_highgui410d -lopencv_imgcodecs410d -lopencv_videoio410d -lopencv_video410d -lopencv_calib3d410d -lopencv_photo410d -lopencv_features2d410d

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../vcpkg/installed/x64-windows/lib/ -lopencv_videoio4 -lopencv_core4 -lopencv_imgproc4
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../vcpkg/installed/x64-windows/debug/lib/ -lopencv_videoio4d -lopencv_core4d -lopencv_imgproc4d
else:unix: LIBS += -L$$PWD/../../../../../vcpkg/installed/x64-windows/lib/ -lopencv_videoio4 -lopencv_core4 -lopencv_imgproc4

INCLUDEPATH += $$PWD/../../../../../vcpkg/installed/x64-windows/include
DEPENDPATH += $$PWD/../../../../../vcpkg/installed/x64-windows/include


win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../vcpkg/installed/x64-windows/lib/libopencv_videoio4.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../vcpkg/installed/x64-windows/debug/lib/libopencv_videoio4d.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../vcpkg/installed/x64-windows/lib/opencv_videoio4.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../vcpkg/installed/x64-windows/debug/lib/opencv_videoio4d.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../../../../vcpkg/installed/x64-windows/lib/libopencv_videoio4.a
