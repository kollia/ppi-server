
set(utilSrcs	GlobalStaticMethods.cpp
                actionproperties.cpp
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
        		ProcessInterfaceTemplate.cpp
        		properties.cpp
        		Terminal.cpp
        		URL.cpp
        		usermanagement.cpp
        		XMLStartEndTagReader.cpp
        		thread/Thread.cpp
        		thread/process.cpp
        		thread/ProcessStarter.cpp
        		thread/StatusLogRoutine.cpp		)

add_library(ppiutil SHARED ${utilSrcs})
#add_library(ppiutil STATIC ${utilSrcs})
#target_link_libraries(ppiutil ${RT_LIBRARIES} ppiserverlibclient ppilogger)
set_target_properties(ppiutil PROPERTIES VERSION ${PPIUTIL_LIB_VERSION} SOVERSION ${PPIUTIL_LIB_SOVERSION})
install(TARGETS ppiutil DESTINATION lib)
