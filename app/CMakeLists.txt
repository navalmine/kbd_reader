set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
  set(QT_PACKAGE Qt6)
else()
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
  set(QT_PACKAGE Qt5)
endif()

add_executable(kbd_ui
  main.cpp
  mainwindow.cpp
)

target_include_directories(kbd_ui PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(kbd_ui PRIVATE kbdcore ${QT_PACKAGE}::Widgets)
