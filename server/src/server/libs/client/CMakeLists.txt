

set(serverlibclientSrcs ExternClientInputTemplate.cpp
						ProcessInterfaceTemplate.cpp
        				OutsideClientTransaction.cpp
        		        SocketClientConnection.cpp      )
    
add_library(ppiserverlibclient ${libraries} ${serverlibclientSrcs})
target_link_libraries(ppiserverlibclient ppiserverlib)
set_target_properties(ppiserverlibclient PROPERTIES VERSION ${PPISERVERCLIENT_LIB_VERSION} SOVERSION ${PPISERVERCLIENT_LIB_SOVERSION})
install(TARGETS ppiserverlibclient DESTINATION lib)