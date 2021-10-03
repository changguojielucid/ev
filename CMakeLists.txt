# Copyright (c) Elucid Bioimaging

cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(EVServer)

find_package(ITK 4.12.2 EXACT REQUIRED)
INCLUDE(${ITK_USE_FILE})

find_package(VTK 8.2.0 EXACT REQUIRED)
INCLUDE(${VTK_USE_FILE})

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)



include_directories(caplib)
include_directories(dicomlib)



add_subdirectory(caplib caplib)
add_subdirectory(dicomlib dicomlib)


set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(CMAKE_AUTOMOC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CPP EVLinkedViewers.cpp EVTargetDefine.cpp EVWorkItem.cpp EVVesselPipeline.cpp  EVWorkItemManager.cpp EVTargetDef.cpp EVWorkClient.cpp EVWorkItemThread.cpp)
add_executable(EVServer  main.cpp ${CPP} )
set(MOC)
qt5_wrap_cpp(MOC_GENERATED ${MOC})

source_group("Generated Files" FILES ${MOC_GENERATED})


include_directories(${Qt5Widgets_INCLUDES})
# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})


target_link_libraries(EVServer caplib dicomlib ${Qt5Widgets_LIBRARIES} ${Qt5WebKitWidgets_LIBRARIES} ${ITK_LIBRARIES} ${VTK_LIBRARIES} )


