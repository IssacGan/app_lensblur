cmake_minimum_required(VERSION 2.8.9)
  project(app_defocus)

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

  set( Qt5  /Users/acambra/Qt5.1.1/5.1.1/clang_64/lib/cmake) #CUSTOMIZE: installation Qt path

  IF(NOT IS_DIRECTORY Qt5)
     set( Qt5  /Users/acambra/TESIS/code/libs/Qt/5.3/clang_64/lib/cmake)
   ENDIF(NOT IS_DIRECTORY VLFEAT_DIR)

  ################################
  # PATH OpenCV
  ################################
  set(OpenCV_DIR /usr/local/share/OpenCV) #CUSTOMIZE: installation OpenCV path

  ################################
  # PATH vlFeat: http://www.vlfeat.org/download.html
  ################################

  set( VLFEAT_DIR  /Users/acambra/TESIS/code/libs/vlfeat-0.9.20) #macbook

  IF(NOT IS_DIRECTORY VLFEAT_DIR)
     set( VLFEAT_DIR  /Users/acambra/TESIS/vlfeat-0.9.18) #iMac
  ENDIF(NOT IS_DIRECTORY VLFEAT_DIR)

  ##################################################################################
  #     Qt
  ##################################################################################

  # Widgets finds its own dependencies (QtGui and QtCore).

  set( Qt5_DIR ${Qt5}/Qt5)
  set( Qt5Core_DIR ${Qt5}/Qt5Core)
  set( Qt5Widgets_DIR ${Qt5}/Qt5Widgets)
  set( Qt5Gui_DIR ${Qt5}/Qt5Gui)
	
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

  #set(OpenCV_DIR /usr/local/share/OpenCV) #CUSTOMIZE: installation OpenCV path
  
  find_package(OpenCV REQUIRED)
  include_directories(${OpenCV_INCLUDES})
  
  ##################################################################################
  #     VlFeat: slic superpixels
  ##################################################################################


  set (VLFEAT_LIB ${VLFEAT_DIR}/bin/maci64)

  ##################################################################################
  #     Compiler FLAGS
  ##################################################################################
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++11 -stdlib=libc++")

 
  ##################################################################################
  #     paths: include and lib
  ##################################################################################

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
  include_directories(${VLFEAT_DIR})

  ##################################################################################
  #     src
  ##################################################################################

  add_executable(${PROJECT_NAME} src/labelimage.h src/widgetFocus.h src/gridButtons.h src/imageviewer2.h src/imageviewer2.cc 
			src/superpixels/superpixel.h  src/superpixels/superpixel.cpp
            src/Nsuperpixels/superpixels.h
			src/Nsuperpixels/superpixels.cpp
			include/depthoffielddefocus.h
			src/mythread.cpp src/mythread.h src/main.cc
			include/least-squares-linear-system2.h )


  target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${VLFEAT_LIB}/libvl.dylib)

  file(COPY ${VLFEAT_LIB}/libvl.dylib DESTINATION ${CMAKE_BINARY_DIR}/Debug)

  # The Qt5Widgets_LIBRARIES variable also includes QtGui and QtCore
  qt5_use_modules(${PROJECT_NAME} Core Widgets)

