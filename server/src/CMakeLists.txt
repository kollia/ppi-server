
set(mainSrcs 	main.cpp
                ProcessChecker.cpp 
		        starter.cpp				)


if(FULL_CLIENT_PATH)
  add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/install.sh
		  COMMAND ${CMAKE_SOURCE_DIR}/cmake/install.sh ${INSTALL_DEFAULTUSER} ${CMAKE_SOURCE_DIR} ${FULL_CLIENT_PATH} ${FULL_CONFIG_PATH} ${FULL_DATABASE_PATH} ${FULL_LOGGING_PATH})
  set(INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/install.sh)
endif(FULL_CLIENT_PATH)

add_executable(ppi-server ${mainSrcs} ${INSTALL_SCRIPT})

target_link_libraries(ppi-server ppiutil ppilogger ppiports ppidb ppiow ppiserverlib ppiserverlibclient 
                                                ppiserverlibserver lirc_client ${RT_LIBRARIES})

install(TARGETS ppi-server DESTINATION bin)

