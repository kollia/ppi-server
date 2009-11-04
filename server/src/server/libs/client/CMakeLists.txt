

set(serverlibclientSrcs	OutsideClientTransaction.cpp
        		        SocketClientConnection.cpp      )
        		
add_library(ppiserverlibclient SHARED ${serverlibclientSrcs})
set_target_properties(ppiserverlibclient PROPERTIES VERSION ${PPISERVERCLIENT_LIB_VERSION} SOVERSION ${PPISERVERCLIENT_LIB_SOVERSION})
install(TARGETS ppiserverlibclient DESTINATION lib)