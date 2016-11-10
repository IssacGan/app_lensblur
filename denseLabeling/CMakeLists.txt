 
  find_package(Boost COMPONENTS system filesystem program_options REQUIRED)

  add_executable(denseLabeling  
	../superpixels/superpixel.cpp
       	../superpixels/superpixels.cpp  
        ../superpixels/labelSet.cpp
	../libs/least-squares-linear-system2.h   
	../libs/imageHDR.cpp
	../libs/lib_slic/SLIC.cpp
        main_denseLabeling.cpp)



target_link_libraries(denseLabeling ${OpenCV_LIBS} ${Boost_LIBRARIES})

