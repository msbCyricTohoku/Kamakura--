QT       += core gui \
        printsupport \
        xml \
        network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Ensure generated build files include headers in the src directory
INCLUDEPATH += src

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/airequester.cpp \
    src/codeeditor.cpp \
    src/finddialog.cpp \
    src/highlighter.cpp \
    src/main.cpp \
    src/kamakura.cpp \
    src/metrics.cpp
    src/airequester.cpp

HEADERS += \
    src/airequester.h \
    src/codeeditor.h \
    src/documentmetrics.h \
    src/finddialog.h \
    src/highlighter.h \
    src/kamakura.h \
    src/linenumberarea.h \
    src/metrics.h
    src/airequester.h

FORMS += \
    src/kamakura.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src/resources.qrc
