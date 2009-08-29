

set(portsSrcs 	contactpin.cpp
        		counter.cpp
        		measuredness.cpp
        		measureThread.cpp
        		OwfsPort.cpp
        		portbaseclass.cpp
        		resistancemeasure.cpp
        		SaveSubValue.cpp
        		shell.cpp
        		switchcontact.cpp
        		switch.cpp
        		tempmeasure.cpp
        		timemeasure.cpp
        		timer.cpp
        		valueholder.cpp			)
        		
add_library(ppiports SHARED ${portsSrcs})

install(TARGETS ppiports DESTINATION lib)