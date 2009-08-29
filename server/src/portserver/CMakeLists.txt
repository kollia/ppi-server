

set(portserverSrcs  owserver.cpp
            	    maximchipaccess.cpp
            		VellemannK8055.cpp    )
#            		LircClient.cpp		)
        		
add_library(ppiportserver SHARED ${portserverSrcs})

install(TARGETS ppiportserver DESTINATION lib)