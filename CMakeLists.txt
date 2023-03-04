cmake_minimum_required(VERSION 3.20)
project(git-qui LANGUAGES CXX)

set(SOURCE_FILES
  src/cleanupdialog.cpp
  src/components/errorlog/errorlog.cpp
  src/components/logview/graphdelegate.cpp
  src/resetdialog.cpp
  src/core.cpp
  src/initialwindowconfiguration.cpp
  src/main.cpp
  src/gitinterface.cpp
  src/mainwindow.cpp
  src/components/dockwidget.cpp
  src/components/repositoryfiles/repositoryfiles.cpp
  src/components/commit/commit.cpp
  src/components/diffview/diffview.cpp
  src/components/repositorylist/repositorylist.cpp
  src/gitdiffline.cpp
  src/components/logview/logview.cpp
  src/components/branchlist/branchlist.cpp
  src/project.cpp
  src/projectsettingsdialog.cpp
  src/qtreewidgetutils.cpp
  src/toolbareditor.cpp
  src/toolbaractions.cpp
  src/treewidgetitem.cpp
  src/gittree.cpp
)

set(HEADER_FILES
  src/cleanupdialog.hpp
  src/components/errorlog/errorlog.hpp
  src/components/logview/graphdelegate.h
  src/resetdialog.hpp
  src/core.hpp
  src/gitinterface.hpp
  src/gitcommit.hpp
  src/gitfile.hpp
  src/initialwindowconfiguration.hpp
  src/mainwindow.hpp
  src/components/dockwidget.hpp
  src/components/repositoryfiles/repositoryfiles.hpp
  src/components/commit/commit.hpp
  src/components/logview/logview.hpp
  src/components/branchlist/branchlist.hpp
  src/gitbranch.hpp
  src/project.hpp
  src/projectsettingsdialog.hpp
  src/qtreewidgetutils.hpp
  src/components/repositorylist/repositorylist.hpp
  src/components/diffview/diffview.hpp
  src/gitdiffline.hpp
  src/toolbareditor.hpp
  src/toolbaractions.hpp
  src/qobjecthelpers.hpp
  src/treewidgetitem.hpp
  src/gittree.hpp
  src/gitref.hpp
)

set(OTHER_FILES
  .gitignore
  README.md
  LICENSE
  de.feelx88.git-qui.svg
  de.feelx88.git-qui.desktop
  de.feelx88.git-qui.metainfo.xml.in
  de.feelx88.git-qui.yml
  .circleci/config.yml
  resources.qrc
)

set(FORMS
  src/cleanupdialog.ui
  src/components/errorlog/errorlog.ui
  src/resetdialog.ui
  src/mainwindow.ui
  src/components/repositoryfiles/repositoryfiles.ui
  src/components/commit/commit.ui
  src/components/diffview/diffview.ui
  src/components/repositorylist/repositorylist.ui
  src/components/logview/logview.ui
  src/components/branchlist/branchlist.ui
  src/projectsettingsdialog.ui
  src/toolbareditor.ui
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_DEPRECATED_WARNINGS ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS
  Core
  Svg
  Xml
  Gui
  Widgets
)

include_directories(src)

execute_process(
  COMMAND sh -c "git describe --always --abbrev=0 --tags --exact-match 2> /dev/null || git describe --always --abbrev=0"
  OUTPUT_VARIABLE GIT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

option(BUILD_EXAMPLES OFF)
add_subdirectory(Qt-Advanced-Docking-System/)
include_directories(Qt-Advanced-Docking-System/src)

add_executable(git-qui ${SOURCE_FILES} ${HEADER_FILES})
add_custom_target(forms SOURCES ${FORMS})
add_custom_target(others SOURCES ${OTHER_FILES})

target_compile_definitions(git-qui
  PUBLIC GIT_VERSION=\"${GIT_VERSION}\"
)

target_link_libraries(git-qui
  Qt::Core
  Qt::Svg
  Qt::Xml
  Qt::Gui
  Qt::Widgets
  ads::qtadvanceddocking
)

if(FLATPAK_BUILD)
  execute_process(
    COMMAND sh -c "date --iso-8601=seconds"
    OUTPUT_VARIABLE DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  configure_file(
    de.feelx88.git-qui.metainfo.xml.in
    de.feelx88.git-qui.metainfo.xml
  )

  install(TARGETS git-qui DESTINATION /app)
  install(FILES de.feelx88.git-qui.svg
    DESTINATION /app/share/icons/hicolor/128x128/apps
  )
  install(FILES de.feelx88.git-qui.desktop
    DESTINATION /app/share/applications
  )
  install(FILES de.feelx88.git-qui.metainfo.xml
    DESTINATION /app/share/metainfo
  )
endif()
