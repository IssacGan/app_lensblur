
  find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
  add_executable(superpixels  
	superpixel.cpp
       	superpixels.cpp  
        labelSet.cpp    
        main_superpixels.cpp
	../libs/imageHDR.cpp
	../libs/lib_slic/SLIC.cpp)

target_link_libraries(superpixels ${OpenCV_LIBS} ${Boost_LIBRARIES})

