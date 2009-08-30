

set(serverlibSrcs	FileDescriptor.cpp    )
        		
add_library(ppiserverlib SHARED ${serverlibSrcs})

install(TARGETS ppiserverlib DESTINATION lib)