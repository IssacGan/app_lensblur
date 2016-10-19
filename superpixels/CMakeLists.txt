cmake_minimum_required(VERSION 2.8.9)
  project(main_superpixels)

  # Tell CMake to run moc when necessary:
  set(CMAKE_AUTOMOC ON)
  # As moc files are generated in the binary dir, tell CMake
  # to always look for includes there:
  set(CMAKE_INCLUDE_CURRENT_DIR ON)


  ##################################################################################
  #   PATH  OpenCV
  ##################################################################################
  find_package(OpenCV REQUIRED)
  include_directories(${OpenCV_INCLUDES})

  
  ##################################################################################
  #     paths: include and lib user
  ##################################################################################

  include_directories(../libs)
  include_directories(../libs/lib_slic)
 
  find_package(Boost COMPONENTS system filesystem program_options REQUIRED)

  ##################################################################################
  #     Compiler FLAGS
  ##################################################################################
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++11 -stdlib=libc++ ")


  ##################################################################################
  #     src
  ##################################################################################
  add_library(slic ../libs/lib_slic/SLIC.cpp)

  add_executable(${PROJECT_NAME}  
	superpixel.cpp
       	superpixels.cpp  
        labelSet.cpp    
        main_superpixels.cpp)



target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} slic ${Boost_LIBRARIES})

