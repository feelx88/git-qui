QT += core gui widgets

CONFIG += c++1z

SOURCES += \
    src/main.cpp \
    src/gitinterface.cpp \
    src/mainwindow.cpp \
    src/components/dockwidget.cpp \
    src/components/repositoryfiles/repositoryfiles.cpp \
    src/components/commit/commit.cpp \
    src/components/diffview/diffview.cpp \
    src/components/repositorylist/repositorylist.cpp \
    src/gitdiffline.cpp \
    src/components/logview/logview.cpp \
    src/components/branchlist/branchlist.cpp \
    src/qtreewidgetutils.cpp

HEADERS += \
    src/gitinterface.hpp \
    src/gitcommit.hpp \
    src/gitfile.hpp \
    src/mainwindow.hpp \
    src/components/dockwidget.hpp \
    src/components/repositoryfiles/repositoryfiles.hpp \
    src/components/commit/commit.hpp \
    src/components/logview/logview.hpp \
    src/components/branchlist/branchlist.hpp \
    src/gitbranch.hpp \
    src/qtreewidgetutils.hpp \
    src/components/repositorylist/repositorylist.hpp \
    src/components/diffview/diffview.hpp \
    src/gitdiffline.hpp

OTHER_FILES += \
    .gitignore \
    qpm.json \
    README.md \
    LICENSE \
    deploy/git-qui.svg \
    deploy/git-qui.desktop

FORMS += \
    src/mainwindow.ui \
    src/components/repositoryfiles/repositoryfiles.ui \
    src/components/commit/commit.ui \
    src/components/diffview/diffview.ui \
    src/components/repositorylist/repositorylist.ui \
    src/components/logview/logview.ui \
    src/components/branchlist/branchlist.ui

INCLUDEPATH += src/

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

RESOURCES += \
    resources.qrc

# qpm dependencies
# include(vendor/vendor.pri)
