      		
include_directories(${CMAKE_SOURCE_DIR}/lib)

set(databaseSrcs    DatabaseThread.cpp 
		            DefaultChipConfigReader.cpp
		            ServerDbTransaction.cpp
		            databasemain.cpp        	)
        		
add_executable(ppi-db-server ${databaseSrcs})
target_link_libraries(ppi-db-server 	ppiutil ppistreamutil ppipropertyutil ppithreadutil ppiprocessutil 
										ppiserverlibclient ppiserverlibserver)
install(TARGETS ppi-db-server DESTINATION bin)