QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Wpedantic -Og

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ConfNewLevelForm.cpp \
    GridEditor.cpp \
    LevelDataManager.cpp \
    MainWindow.cpp \
    SelectableLineLayout.cpp \
    TableModel.cpp \
    main.cpp

HEADERS += \
    ConfNewLevelForm.hpp \
    GridEditor.hpp \
    LevelDataManager.hpp \
    MainWindow.hpp \
    SelectableLineLayout.hpp \
    TableModel.hpp

FORMS += \
    ConfNewLevelForm.ui \
    GridEditor.ui \
    MainWindow.ui

CONFIG += ldebug
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
