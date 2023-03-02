QT += core svg xml gui widgets

CONFIG += c++1z

SOURCES += \
    src/cleanupdialog.cpp \
    src/components/errorlog/errorlog.cpp \
    src/components/logview/graphdelegate.cpp \
    src/resetdialog.cpp \
    src/core.cpp \
    src/initialwindowconfiguration.cpp \
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
    src/project.cpp \
    src/projectsettingsdialog.cpp \
    src/qtreewidgetutils.cpp \
    src/toolbareditor.cpp \
    src/toolbaractions.cpp \
    src/treewidgetitem.cpp \
    src/gittree.cpp

HEADERS += \
    src/cleanupdialog.hpp \
    src/components/errorlog/errorlog.hpp \
    src/components/logview/graphdelegate.h \
    src/resetdialog.hpp \
    src/core.hpp \
    src/gitinterface.hpp \
    src/gitcommit.hpp \
    src/gitfile.hpp \
    src/initialwindowconfiguration.hpp \
    src/mainwindow.hpp \
    src/components/dockwidget.hpp \
    src/components/repositoryfiles/repositoryfiles.hpp \
    src/components/commit/commit.hpp \
    src/components/logview/logview.hpp \
    src/components/branchlist/branchlist.hpp \
    src/gitbranch.hpp \
    src/project.hpp \
    src/projectsettingsdialog.hpp \
    src/qtreewidgetutils.hpp \
    src/components/repositorylist/repositorylist.hpp \
    src/components/diffview/diffview.hpp \
    src/gitdiffline.hpp \
    src/toolbareditor.hpp \
    src/toolbaractions.hpp \
    src/qobjecthelpers.hpp \
    src/treewidgetitem.hpp \
    src/gittree.hpp \
    src/gitref.hpp

OTHER_FILES += \
    .gitignore \
    qpm.json \
    README.md \
    LICENSE \
    de.feelx88.git-qui.svg \
    de.feelx88.git-qui.desktop \
    deploy/installer/config-mac.xml \
    deploy/installer/packages/git-qui/meta/LICENSE \
    deploy/installer/packages/git-qui/meta/package.xml \
    de.feelx88.git-qui.yml \
    .circleci/config.yml

FORMS += \
    src/cleanupdialog.ui \
    src/components/errorlog/errorlog.ui \
    src/resetdialog.ui \
    src/mainwindow.ui \
    src/components/repositoryfiles/repositoryfiles.ui \
    src/components/commit/commit.ui \
    src/components/diffview/diffview.ui \
    src/components/repositorylist/repositorylist.ui \
    src/components/logview/logview.ui \
    src/components/branchlist/branchlist.ui \
    src/projectsettingsdialog.ui \
    src/toolbareditor.ui

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
else: unix:contains(DEFINES, FLATPAK_BUILD): target.path = /app
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:contains(DEFINES, FLATPAK_BUILD) {
  desktopfile.path = /app/share/icons/hicolor/128x128/apps
  desktopfile.files = de.feelx88.git-qui.svg
  icon.path = /app/share/applications
  icon.files = de.feelx88.git-qui.desktop
  metainfo.path = /app/share/metainfo
  matainfo.files = de.feelx88.git-qui.metainfo.xml
  INSTALLS += desktopfile icon metainfo
}

DEFINES += GIT_VERSION=\\\"$$system(git describe --always --abbrev=0 --tags --exact-match 2> /dev/null || git describe --always --abbrev=0)\\\"

RESOURCES += \
    resources.qrc

unix:contains(DEFINES, FLATPAK_BUILD) {
  LIBS += -L/app/lib
} else {
  LIBS += -L$$PWD/Qt-Advanced-Docking-System/lib
}
include(Qt-Advanced-Docking-System/ads.pri)
INCLUDEPATH += Qt-Advanced-Docking-System/src
DEPENDPATH += Qt-Advanced-Docking-System/src

DISTFILES += \
  de.feelx88.git-qui.metainfo.xml
