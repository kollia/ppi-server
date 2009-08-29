

set(serverSrcs	ClientTransaction.cpp
        		Communication.cpp
        		communicationthreadstarter.cpp
        		FileDescriptor.cpp
        		hearingthread.cpp
        		OutsideClientTransaction.cpp
        		ServerMethodTransaction.cpp
        		ServerProcess.cpp
        		ServerThread.cpp
        		ServerTransaction.cpp
        		SocketClientConnection.cpp
        		SocketServerConnection.cpp
        		SSLConnect.cpp			        )
        		
add_library(ppiserver SHARED ${serverSrcs})

install(TARGETS ppiserver DESTINATION lib)