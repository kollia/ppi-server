
set(utilSrcs	GlobalStaticMethods.cpp
		        CalculatorContainer.cpp
		        Calendar.cpp
#        		exception.cpp
        		Terminal.cpp
        		URL.cpp
        		ParamCommand.cpp
        		MainParams.cpp
        		usermanagement.cpp
	       		XMLStartEndTagReader.cpp	)
#        		thread/StatusLogRoutine.cpp
#        		thread/Thread.cpp
#        		stream/IMethodStringStream.cpp
#        		stream/IParameterStringStream.cpp
#        		stream/OMethodStringStream.cpp
#        		stream/OParameterStringStream.cpp
#        		properties/properties.cpp
#        		properties/interlacedproperties.cpp
#                properties/actionproperties.cpp
#        		properties/interlacedactionproperties.cpp
#        		properties/configpropertycasher.cpp			)
#        		process/process.cpp
#        		process/ProcessStarter.cpp					)

add_library(ppiutil ${libraries} ${utilSrcs})
target_link_libraries(ppiutil ppipropertyutil ppistreamutil ppipattern)
set_target_properties(ppiutil PROPERTIES VERSION ${PPIUTIL_LIB_VERSION} SOVERSION ${PPIUTIL_LIB_SOVERSION})
install(TARGETS ppiutil DESTINATION lib)
