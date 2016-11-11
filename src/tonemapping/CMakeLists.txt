if (${OpenCV_FOUND} AND ${Qt5Core_FOUND})
  add_executable(tonemapping 
		labelimage.h 
		gridButtons.h 
		imageviewer2.h 
		imageviewer2.cc
		mythread.cpp 
		mythread.h
		main.cc
		widgetImage.h
		../../superpixels/superpixel.cpp
		../../superpixels/superpixels.cpp
		../../superpixels/labelSet.cpp
		../../denseLabeling/denseLabeling.cpp
		../../libs/least-squares-linear-system2.h
        	../../libs/tonemapping/tonemapping.cc
        	../../libs/imageHDR.cpp
		../../libs/lib_slic/SLIC.cpp)

  target_link_libraries(tonemapping ${OpenCV_LIBS} Qt5::Gui Qt5::Core Qt5::Widgets)
endif()

