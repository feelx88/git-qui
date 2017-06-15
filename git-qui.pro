QT += qml quick quickcontrols2

CONFIG += c++11

SOURCES += src/main.cpp \
    src/git/gitfile.cpp \
    src/git/gitdiffline.cpp \
    src/git/agitmanager.cpp \
    src/git/git-bin/gitbingitmanager.cpp \
    src/git/gitcommit.cpp

HEADERS += \
    include/git/gitfile.h \
    include/git/gitdiffline.h \
    include/git/agitmanager.h \
    include/git/git-bin/gitbingitmanager.h \
    include/git/gitcommit.h

OTHER_FILES += .gitignore \
    qtquickcontrols2.conf \
    ui/default/main.qml \
    ui/default/CommitPage/CommitPage.qml \
    ui/default/CommitPage/CommitForm.ui.qml \
    ui/default/HistoryPage/HistoryPage.qml \
    ui/default/CommitPage/FileRow.qml \
    ui/default/CommitPage/DiffRow.qml

libgit2 {
  LIBS += -lgit2
  DEFINES += USE_LIBIGT2

  SOURCES += src/git/libgit2/libgit2gitmanager.cpp
  HEADERS += include/git/libgit2/libgit2gitmanager.h
}

INCLUDEPATH += include/

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ui/default/TreePage/HistoryLine.qml

RESOURCES += \
    res.qrc

# qpm dependencies
include(vendor/vendor.pri)
