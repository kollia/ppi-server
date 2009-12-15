

set(portserverSrcs  owserver.cpp
                    OwServerQuestions.cpp
            	    maximchipaccess.cpp
            		VellemannK8055.cpp
            		ExternPorts.cpp    )
#            		LircClient.cpp		)
        		
add_library(ppiportserver SHARED ${portserverSrcs})
set_target_properties(ppiportserver PROPERTIES VERSION ${PPIPORTSERVER_LIB_VERSION} SOVERSION ${PPIPORTSERVER_LIB_SOVERSION})
install(TARGETS ppiportserver DESTINATION lib)