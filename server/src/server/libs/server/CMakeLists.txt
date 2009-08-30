

set(serverlibserverSrcs    Communication.cpp
        		           communicationthreadstarter.cpp
                		   ServerProcess.cpp
                		   ServerThread.cpp
                		   SocketServerConnection.cpp
                		   SSLConnect.cpp			        )
        		
add_library(ppiserverlibserver SHARED ${serverlibserverSrcs})

install(TARGETS ppiserverlibserver DESTINATION lib)