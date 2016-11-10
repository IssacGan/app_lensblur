add_executable(dof 
		labelimage.h 
		widgetFocus.h
		gridButtons.h 
		imageviewer2.h 
		imageviewer2.cc
		mythread.cpp 
		mythread.h
		main.cc
		../../superpixels/superpixel.cpp
		../../superpixels/superpixels.cpp
		../../superpixels/labelSet.cpp
		../../denseLabeling/denseLabeling.cpp
		../../libs/least-squares-linear-system2.h
        	../../libs/depthoffielddefocus.cc
        	../../libs/imageHDR.cpp
		../../libs/lib_slic/SLIC.cpp)

target_link_libraries(dof ${OpenCV_LIBS} Qt5::Gui Qt5::Core Qt5::Widgets)

