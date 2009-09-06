
set(utilSrcs	actionproperties.cpp
		        CalculatorContainer.cpp
		        Calendar.cpp
        		configpropertycasher.cpp
        		exception.cpp
        		ExternClientInputTemplate.cpp
        		interlacedactionproperties.cpp
        		interlacedproperties.cpp
        		process.cpp
        		ProcessStarter.cpp
        		ProcessInterfaceTemplate.cpp
        		properties.cpp
        		StatusLogRoutine.cpp
        		Terminal.cpp
        		Thread.cpp
        		URL.cpp
        		usermanagement.cpp
        		XMLStartEndTagReader.cpp		)

add_library(ppiutil SHARED ${utilSrcs})

install(TARGETS ppiutil DESTINATION lib)
