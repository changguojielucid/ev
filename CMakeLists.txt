# Copyright (c) Elucid Bioimaging

cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(ProServer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CONFIGURATION_TYPES "RelWithDebInfo" CACHE STRING "" FORCE)
set(CMAKE_AUTOMOC ON)

if(APPLE)
  add_definitions(
    -Wno-macro-redefined
    -Wno-inconsistent-missing-override
    -Wno-deprecated-declarations
    -Wno-potentially-evaluated-expression
    -Wno-constant-conversion
    )
  # suppress warning about linking in Qt
elseif(WIN32)
  cmake_policy(SET CMP0020 NEW)
  add_definitions(/EHsc)
  add_definitions(/D_USE_MATH_DEFINES)
endif(APPLE)


find_package(ITK HINTS ${ITK_DIR})
find_package(VTK HINTS ${VTK_DIR})
find_package(Qt5Widgets)
find_package(Qt5WebSockets REQUIRED)
find_package(Qt5Network REQUIRED)


# Tell CMake to run moc when necessary.
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake to always look for includes there.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(libs/caplib)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/caplib)
include_directories(libs/dicomlib)
include_directories(${Qt5Widgets_INCLUDES})
include_directories(${Qt5WebSockets_INCLUDES})
include_directories(${Qt5Network_INCLUDES})


# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

add_subdirectory(libs/dicomlib dicomlib)
add_subdirectory(libs/caplib caplib)

if (ITK_FOUND)
  include(${ITK_USE_FILE})
endif()

if( VTK_FOUND )
  if (${VTK_VERSION_MAJOR} LESS 9)
    include(${VTK_USE_FILE}) # not needed from VTK9
  endif()
endif()


set(EV_SRC
  EVLinkedViewers.cpp
  EVTargetDef.cpp
  EVTargetDefine.cpp
  EVTargetDefine_CLR.cpp
  EVTargetDefine_CRS.cpp
  EVTargetDefine_LMN.cpp
  EVTargetDefine_PDP.cpp
  EVTargetDefine_SEG.cpp
  EVTargetDefine_WAL.cpp
  EVVesselPipeline.cpp
  EVWorkClient.cpp
  EVWorkClientManager.cpp
  EVWorkItem.cpp
  EVTask.cpp
  EVProServer.cpp
  EVSocketClient.cpp
  EVSocketServer.cpp
  EVWebSocketClient.cpp
  EVWebSocketServer.cpp
  EVWorkItemManager.cpp
  EVWorkItemThread.cpp
  main.cpp
)

add_definitions(-fPIC)
add_executable(ProServer MACOSX_BUNDLE ${EV_SRC})

install(TARGETS ProServer DESTINATION ${INSTALL_DIR})
target_link_libraries(ProServer caplib dicomlib ${Qt5WebSockets_LIBRARIES}  ${Qt5Network_LIBRARIES} ${Qt5Widgets_LIBRARIES}  ${ITK_LIBRARIES} ${VTK_LIBRARIES} )

if (NOT VTK_VERSION VERSION_LESS "8.90.0")
  vtk_module_autoinit(TARGETS ProServer MODULES ${VTK_LIBRARIES} )
endif()

