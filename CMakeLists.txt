cmake_minimum_required(VERSION 2.8.9)
  project(app_image_editing)

  # Tell CMake to run moc when necessary:
  set(CMAKE_AUTOMOC ON)
  # As moc files are generated in the binary dir, tell CMake
  # to always look for includes there:
  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  ##################################################################################
  #     PATHS
  ##################################################################################

  # Path for output executable files
  IF(NOT (IS_DIRECTORY "${app_image_editing_SOURCE_DIR}/bin"))
     file(MAKE_DIRECTORY "${app_image_editing_SOURCE_DIR}/bin")
  endif()
  set(EXECUTABLE_OUTPUT_PATH "${app_image_editing_SOURCE_DIR}/bin")
  
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
  # find_package(Qt5)
  find_package(Qt5Core)
  find_package(Qt5Widgets)
  find_package(Qt5Gui)

  if(${Qt5Core_FOUND})
     # The Qt5Widgets_INCLUDES also includes the include directories for
     # dependencies QtCore and QtGui
     include_directories(${Qt5Core_INCLUDES})
     include_directories(${Qt5Widgets_INCLUDES})

     # We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
     add_definitions(${Qt5Core_DEFINITIONS})
     add_definitions(${Qt5Widgets_DEFINITIONS})

     # Executables fail to build with Qt 5 in the default configuration
     # without -fPIE. We add that here.
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
  endif()

  ##################################################################################
  #     OpenCV
  ##################################################################################
  
  find_package(OpenCV)
  if(${OpenCV_FOUND})
     include_directories(${OpenCV_INCLUDES})
     link_directories(${OpenCV})
  endif()


  ##################################################################################
  #     Boost
  ##################################################################################
 
  find_package(Boost COMPONENTS system filesystem program_options)
	  
  ##################################################################################
  #     Compiler FLAGS
  ##################################################################################

 if ( ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel") )
    set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtune=native")
    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
       set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Dsrandom=srand -Drandom=rand -D_USE_MATH_DEFINES -D_WIN")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message("Using Visual Studio, are you sure?")
endif()
  
message(STATUS "Compiler  = ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "System    = ${CMAKE_SYSTEM_NAME}")
message(STATUS "Prefix    = ${CMAKE_PREFIX_PATH}")
message(STATUS "Flags     = ${CMAKE_CXX_FLAGS}")
message(STATUS "Build     = ${CMAKE_BUILD_TYPE}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
   message(STATUS "R.Flags   = ${CMAKE_CXX_FLAGS_RELEASE}")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
   message(STATUS "D.Flags   = ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

  ##################################################################################
  #     paths: include and lib
  ##################################################################################

  include_directories(${app_image_editing_SOURCE_DIR})
  include_directories(libs)
  include_directories(libs/lib_slic)
  include_directories(superpixels)  
  include_directories(denseLabeling)  

  ##################################################################################
  #     src
  ##################################################################################
  add_subdirectory(src/dof)
  add_subdirectory(src/tonemapping)
  add_subdirectory(src/dehaz)
  add_subdirectory(superpixels)
  add_subdirectory(denseLabeling)

