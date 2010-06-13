

set(portsSrcs 	counter.cpp
        		measuredness.cpp
        		measureThread.cpp
        		OwfsPort.cpp
        		portbaseclass.cpp
        		resistancemeasure.cpp
        		SaveSubValue.cpp
        		shell.cpp
        		#contactpin.cpp
        		#switchcontact.cpp
        		switch.cpp
        		tempmeasure.cpp
        		timemeasure.cpp
        		timer.cpp
        		valueholder.cpp			)
        		
add_library(ppiports SHARED ${portsSrcs})
set_target_properties(ppiports PROPERTIES VERSION ${PPIPORTS_LIB_VERSION} SOVERSION ${PPIPORTS_LIB_SOVERSION})
install(TARGETS ppiports DESTINATION lib)