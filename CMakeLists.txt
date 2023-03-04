cmake_minimum_required(VERSION 3.20)
project(git-qui LANGUAGES CXX)

file(GLOB_RECURSE
  SOURCE_FILES
  LIST_DIRECTORIES false
  src/*.cpp
)

file(GLOB_RECURSE
  HEADER_FILES
  LIST_DIRECTORIES false
  src/*.hpp
)

file(GLOB_RECURSE
  FORMS
  LIST_DIRECTORIES false
  src/*.ui
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
  .clang-format
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

add_executable(git-qui MACOSX_BUNDLE ${SOURCE_FILES} ${HEADER_FILES} ${FORMS})
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
  target_compile_definitions(git-qui
    PUBLIC FLATPAK_BUILD
  )

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
