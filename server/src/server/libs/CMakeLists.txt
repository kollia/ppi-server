

set(serverlibSrcs	FileDescriptor.cpp
                    SocketErrorHandling.cpp    )

add_library(ppiserverlib ${libraries} ${serverlibSrcs})
target_link_libraries(ppiserverlib ppithreadutil)
set_target_properties(ppiserverlib PROPERTIES VERSION ${PPISERVER_LIB_VERSION} SOVERSION ${PPISERVER_LIB_SOVERSION})
install(TARGETS ppiserverlib DESTINATION lib)