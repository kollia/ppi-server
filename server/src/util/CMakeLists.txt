
set(utilSrcs	actionproperties.cpp
		        CalculatorContainer.cpp
		        Calendar.cpp
        		configpropertycasher.cpp
        		exception.cpp
        		ExternClientInputTemplate.cpp
        		IMethodStringStream.cpp
        		interlacedactionproperties.cpp
        		interlacedproperties.cpp
        		IParameterStringStream.cpp
        		OMethodStringStream.cpp
        		OParameterStringStream.cpp
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
set_target_properties(ppiutil PROPERTIES VERSION ${PPIUTIL_LIB_VERSION} SOVERSION ${PPIUTIL_LIB_SOVERSION})
install(TARGETS ppiutil DESTINATION lib)
