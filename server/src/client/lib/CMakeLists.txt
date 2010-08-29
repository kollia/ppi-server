

set(clientLibSrcs   ClientTransaction.cpp 
                    hearingthread.cpp    )
                    
add_library(ppicppclient SHARED ${clientLibSrcs})
set_target_properties(ppicppclient PROPERTIES VERSION ${PPICPPCLIENT_LIB_VERSION} SOVERSION ${PPICPPCLIENT_LIB_SOVERSION})
target_link_libraries(ppicppclient ppiutil ppistreamutil ppithreadutil ppiserverlibclient)
install(TARGETS ppicppclient DESTINATION lib)
