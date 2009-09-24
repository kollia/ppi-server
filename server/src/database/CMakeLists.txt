      		
include_directories(${CMAKE_SOURCE_DIR}/lib)

set(databaseSrcs    DatabaseThread.cpp 
		            DefaultChipConfigReader.cpp
		            ServerDbTransaction.cpp
		            databasemain.cpp        	)
        		
add_executable(ppi-db-server ${databaseSrcs})
target_link_libraries(ppi-db-server ppiutil ppilogger ppidb ppiserverlib ppiserverlibserver ppiserverlibclient ${RT_LIBRARIES} )
#                                ppiportserver ${OWFS_LIBRARIES} ${USB_LIBRARIES} ${K8055_LIBRARIES} ) # for debug-info from owserver
install(TARGETS ppi-db-server DESTINATION bin)