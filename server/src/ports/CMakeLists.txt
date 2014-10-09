

set(portsSrcs 	ListCalculator.cpp
				counter.cpp
				MeasureInformerCache.cpp
				DbFiller.cpp
				DbFillerFactory.cpp
				DbFillerCache.cpp
        		measuredness.cpp
        		measureThread.cpp
        		ExternPort.cpp
        		LircPort.cpp
        		portbaseclass.cpp
        		output.cpp
        		resistancemeasure.cpp
        		SaveSubValue.cpp
        		shell.cpp
        		switch.cpp
        		tempmeasure.cpp
        		timemeasure.cpp
        		timer.cpp
        		valueholder.cpp
        		Set.cpp
        		SubroutineSubVarHolder.cpp
        		read.cpp
        		ReadWorkder.cpp	)
        
add_library(ppiports ${libraries} ${portsSrcs})
target_link_libraries(ppiports ppiutil ppithreadutil ppiserverlibclient ppidb ppiow ppipattern)
set_target_properties(ppiports PROPERTIES VERSION ${PPIPORTS_LIB_VERSION} SOVERSION ${PPIPORTS_LIB_SOVERSION})
install(TARGETS ppiports DESTINATION lib)