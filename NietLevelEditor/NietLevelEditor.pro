QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#CONFIG += c++20
CONFIG += console c++17
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Wpedantic -Og

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BackgroundForm.cpp \
    CheckpointForm.cpp \
    ConfNewLevelForm.cpp \
    EventFilter.cpp \
    GridEditor.cpp \
    LevelDataManager.cpp \
    LineWallMove.cpp \
    LogForm.cpp \
    MainWindow.cpp \
    MoveableWallForm.cpp \
    SelectableLineLayout.cpp \
    TableModel.cpp \
    main.cpp

HEADERS += \
    BackgroundForm.hpp \
    CheckpointForm.hpp \
    ConfNewLevelForm.hpp \
    EventFilter.hpp \
    GridEditor.hpp \
    LevelDataManager.hpp \
    LineWallMove.hpp \
    LogForm.hpp \
    MainWindow.hpp \
    MoveableWallForm.hpp \
    SelectableLineLayout.hpp \
    TableModel.hpp

FORMS += \
    BackgroundForm.ui \
    CheckpointForm.ui \
    ConfNewLevelForm.ui \
    GridEditor.ui \
    LogForm.ui \
    MainWindow.ui \
    MoveableWallForm.ui

CONFIG += ldebug
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
