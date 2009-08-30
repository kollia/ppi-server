

set(serverlibclientSrcs	OutsideClientTransaction.cpp
        		        SocketClientConnection.cpp      )
        		
add_library(ppiserverlibclient SHARED ${serverlibclientSrcs})

install(TARGETS ppiserverlibclient DESTINATION lib)