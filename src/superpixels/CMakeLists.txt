if(${Boost_FOUND} AND ${OpenCV_FOUND})
  find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
  add_executable(superpixels
	  ${SUPERPIXELS_SRCS}  
        main_superpixels.cpp
	../../libs/imageHDR.cpp
	)

  target_link_libraries(superpixels ${OpenCV_LIBS} ${Boost_LIBRARIES})
endif()
