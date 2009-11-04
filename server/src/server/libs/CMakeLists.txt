

set(serverlibSrcs	FileDescriptor.cpp    )
        		
add_library(ppiserverlib SHARED ${serverlibSrcs})
set_target_properties(ppiserverlib PROPERTIES VERSION ${PPISERVER_LIB_VERSION} SOVERSION ${PPISERVER_LIB_SOVERSION})
install(TARGETS ppiserverlib DESTINATION lib)