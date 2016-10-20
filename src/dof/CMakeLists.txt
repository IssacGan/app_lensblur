cmake_minimum_required(VERSION 2.8.9)
  project(app_dof)

  # Tell CMake to run moc when necessary:
  set(CMAKE_AUTOMOC ON)
  # As moc files are generated in the binary dir, tell CMake
  # to always look for includes there:
  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  ##################################################################################
  #     PATHS
  ##################################################################################
  ################################
  # PATH Qt: http://www.qt.io/download/
  ################################

  set(Qt5_DIR /Users/acambra/Qt5.1.1/5.1.1/clang_64/lib/cmake/Qt5) #CUSTOMIZE: installation Qt path


  ################################
  # PATH OpenCV
  ################################
  set(OpenCV_DIR /usr/local/share/OpenCV) #CUSTOMIZE: installation OpenCV path

 
  ##################################################################################
  #     Qt
  ##################################################################################

  # Widgets finds its own dependencies (QtGui and QtCore).

  set( Qt5Core_DIR ${Qt5_DIR}/../Qt5Core)
  set( Qt5Widgets_DIR  ${Qt5_DIR}/../Qt5Widgets)
  set( Qt5Gui_DIR  ${Qt5_DIR}/../Qt5Gui)
	
  # Find packages Qt
  find_package(Qt5)
  find_package(Qt5Core )
  find_package(Qt5Widgets)
  find_package(Qt5Gui)

  # The Qt5Widgets_INCLUDES also includes the include directories for
  # dependencies QtCore and QtGui
  include_directories(${Qt5Core_INCLUDES})
  include_directories(${Qt5Widgets_INCLUDES})

  # We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
  add_definitions(${Qt5Core_DEFINITIONS})
  add_definitions(${Qt5Widgets_DEFINITIONS})

  # Executables fail to build with Qt 5 in the default configuration
  # without -fPIE. We add that here.
  set(CMAKE_CXX_FLAGS "${Qt5Core_EXECUTABLE_COMPILE_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

  ##################################################################################
  #     OpenCV
  ##################################################################################
  
  find_package(OpenCV REQUIRED)
  include_directories(${OpenCV_INCLUDES})
  
  
  ##################################################################################
  #     Compiler FLAGS
  ##################################################################################
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++11 -stdlib=libc++")

 
  ##################################################################################
  #     paths: include and lib
  ##################################################################################

  include_directories(libs)
  include_directories(libs/lib_slic)
  include_directories(superpixels)  

  ##################################################################################
  #     src
  ##################################################################################
  add_library(slic libs/lib_slic/SLIC.cpp)
  add_library(blur libs/depthoffielddefocus.cc)

  add_executable(${PROJECT_NAME} 
		src/dof/labelimage.h src/dof/widgetFocus.h src/dof/gridButtons.h src/dof/imageviewer2.h src/dof/imageviewer2.cc
		src/dof/mythread.cpp src/dof/mythread.h src/dof/main.cc
		superpixels/superpixel.cpp
		superpixels/superpixels.cpp
		superpixels/labelSet.cpp
		denseLabeling/denseLabeling.cpp
		libs/least-squares-linear-system2.h)


  target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} slic blur)

  # The Qt5Widgets_LIBRARIES variable also includes QtGui and QtCore
  qt5_use_modules(${PROJECT_NAME} Core Widgets)

