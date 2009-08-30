

set(serverSrcs	ClientTransaction.cpp
        		hearingthread.cpp
        		ServerMethodTransaction.cpp
        		ServerTransaction.cpp	        )
        		
add_library(ppiserver SHARED ${serverSrcs})

install(TARGETS ppiserver DESTINATION lib)