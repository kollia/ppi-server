
set(mainSrcs 	main.cpp
                ProcessChecker.cpp 
		        starter.cpp				
		        
		        ports/ListCalculator.cpp
        		ports/measureThread.cpp
		        ports/counter.cpp
        		ports/measuredness.cpp
        		ports/ExternPort.cpp
        		ports/portbaseclass.cpp
        		ports/output.cpp
        		ports/resistancemeasure.cpp
        		ports/SaveSubValue.cpp
        		ports/shell.cpp
        		ports/switch.cpp
        		ports/tempmeasure.cpp
        		ports/timemeasure.cpp
        		ports/timer.cpp
        		ports/valueholder.cpp
        		ports/Set.cpp				)


if(FULL_CLIENT_PATH)
  add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/install.sh
		  COMMAND ${CMAKE_SOURCE_DIR}/cmake/install.sh ${INSTALL_DEFAULTUSER} ${CMAKE_SOURCE_DIR} ${FULL_CLIENT_PATH} ${FULL_CONFIG_PATH} ${FULL_DATABASE_PATH} ${FULL_LOGGING_PATH})
  set(INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/install.sh)
endif(FULL_CLIENT_PATH)

add_executable(ppi-server ${mainSrcs} ${INSTALL_SCRIPT})

target_link_libraries(ppi-server	ppiutil ppistreamutil ppipropertyutil ppithreadutil ppiprocessutil
									ppiserverlibclient ppiserverlibserver ppilogger ppiow ppidb
									lirc_client															)

install(TARGETS ppi-server DESTINATION bin)

