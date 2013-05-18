      		
include_directories(${CMAKE_SOURCE_DIR}/lib)

set(databaseSrcs    logger/LogThread.cpp
					DatabaseThread.cpp 
		            DefaultChipConfigReader.cpp
		            ServerDbTransaction.cpp
		            databasemain.cpp        	)
        		
add_executable(ppi-db-server ${databaseSrcs})
target_link_libraries(ppi-db-server 	ppiutil ppithreadutil ppistreamutil ppipropertyutil ppithreadutil ppiprocessutil 
										ppidb ppiserverlibclient ppiserverlibserver boost_regex)
install(TARGETS ppi-db-server DESTINATION bin)