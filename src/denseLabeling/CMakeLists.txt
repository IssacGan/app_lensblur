 
if(${Boost_FOUND} AND ${OpenCV_FOUND})
  add_executable(denseLabeling
	  ${SUPERPIXELS_SRCS}  
	../../libs/imageHDR.cpp
        main_denseLabeling.cpp)

  target_link_libraries(denseLabeling ${OpenCV_LIBS} ${Boost_LIBRARIES})
endif()

