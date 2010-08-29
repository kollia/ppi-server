

set(serverlibserverSrcs    Communication.cpp
        		           communicationthreadstarter.cpp
                		   ServerMethodTransaction.cpp
                           ServerProcess.cpp
                		   ServerThread.cpp
                		   SocketServerConnection.cpp
                		   SSLConnect.cpp			        )

add_library(ppiserverlibserver ${libraries} ${serverlibserverSrcs})
target_link_libraries(ppiserverlibserver ppiprocessutil ppiserverlib ppiserverlibclient ppilogger)
set_target_properties(ppiserverlibserver PROPERTIES VERSION ${PPISERVERSERVER_LIB_VERSION} SOVERSION ${PPISERVERSERVER_LIB_SOVERSION})
install(TARGETS ppiserverlibserver DESTINATION lib)