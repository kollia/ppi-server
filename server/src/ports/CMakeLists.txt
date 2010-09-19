

set(portsSrcs 	counter.cpp
        		measuredness.cpp
        		measureThread.cpp
        		OwfsPort.cpp
        		portbaseclass.cpp
        		resistancemeasure.cpp
        		SaveSubValue.cpp
        		shell.cpp
        		switch.cpp
        		tempmeasure.cpp
        		timemeasure.cpp
        		timer.cpp
        		valueholder.cpp
        		Set.cpp					)
        
add_library(ppiports ${libraries} ${portsSrcs})
target_link_libraries(ppiports ppiutil ppithreadutil ppiserverlibclient ppidb ppiow ppilogger)
set_target_properties(ppiports PROPERTIES VERSION ${PPIPORTS_LIB_VERSION} SOVERSION ${PPIPORTS_LIB_SOVERSION})
install(TARGETS ppiports DESTINATION lib)