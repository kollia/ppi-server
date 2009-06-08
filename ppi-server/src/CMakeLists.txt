
set(mainSrcs 	main.cpp 
		starter.cpp				)
set(databaseSrcs database/Database.cpp 
		database/DefaultChipConfigReader.cpp	)
set(loggerSrcs	logger/LogThread.cpp			)
set(portsSrcs 	ports/contactpin.cpp
		ports/counter.cpp
		ports/measuredness.cpp
		ports/measureThread.cpp
		ports/OwfsPort.cpp
		ports/portbaseclass.cpp
		ports/resistancemeasure.cpp
		ports/SaveSubValue.cpp
		ports/shell.cpp
		ports/switchcontact.cpp
		ports/switch.cpp
		ports/tempmeasure.cpp
		ports/timemeasure.cpp
		ports/timer.cpp
		ports/valueholder.cpp			)
set(portserverSrcs portserver/LircClient.cpp
		portserver/maximchipaccess.cpp
		portserver/owserver.cpp
		portserver/VellemannK8055.cpp		)
set(serverSrcs	server/ClientTransaction.cpp
		server/Communication.cpp
		server/communicationthreadstarter.cpp
		server/FileDescriptor.cpp
		server/hearingthread.cpp
		server/ServerThread.cpp
		server/ServerTransaction.cpp
		server/SocketClientConnection.cpp
		server/SocketServerConnection.cpp
		server/SSLConnect.cpp			)
set(utilSrcs	util/actionproperties.cpp
		util/CalculatorContainer.cpp
		util/configpropertycasher.cpp
		util/exception.cpp
		util/interlacedactionproperties.cpp
		util/interlacedproperties.cpp
		util/process.cpp
		util/properties.cpp
		util/Terminal.cpp
		util/Thread.cpp
		util/URL.cpp
		util/usermanagement.cpp
		util/XMLStartEndTagReader.cpp		)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

 
set(INSTALL_CLIENT_PATH /var/local CACHE PATH CLIENT_PATH)
set(INSTALL_CONFIG_PATH /usr/local/etc CACHE PATH CONFIG_PATH)
set(INSTALL_DATABASE_PATH /var/local CACHE PATH DATABASE_PATH)
set(INSTALL_LOGGING_PATH /var/log CACHE PATH LOGGING_PATH)
set(INSTALL_DEFAULTUSER nobody CACHE STRING DEFAULTUSER)

string(REGEX MATCH "^." reg ${INSTALL_CLIENT_PATH})
if(reg STREQUAL "/")
  set(FULL_CLIENT_PATH ${INSTALL_CLIENT_PATH}/ppi-server/client)
else(reg STREQUAL "/")
  set(FULL_CLIENT_PATH ${CMAKE_SOURCE_DIR}/client)
endif(reg STREQUAL "/")

string(REGEX MATCH "^." reg ${INSTALL_CONFIG_PATH})
if(reg STREQUAL "/")
  set(FULL_CONFIG_PATH ${INSTALL_CONFIG_PATH}/ppi-config)
else(reg STREQUAL "/")
  set(FULL_CLIENT_PATH ${CMAKE_SOURCE_DIR}/conf)
endif(reg STREQUAL "/")

string(REGEX MATCH "^." reg ${INSTALL_DATABASE_PATH})
if(reg STREQUAL "/")
  set(FULL_DATABASE_PATH ${INSTALL_DATABASE_PATH}/ppi-server/database)
else(reg STREQUAL "/")
  set(FULL_CLIENT_PATH ${CMAKE_SOURCE_DIR}/data)
endif(reg STREQUAL "/")

string(REGEX MATCH "^." reg ${INSTALL_LOGGING_PATH})
if(reg STREQUAL "/")
  set(FULL_LOGGING_PATH ${INSTALL_LOGGING_PATH}/ppi-server)
else(reg STREQUAL "/")
  set(FULL_CLIENT_PATH ${CMAKE_SOURCE_DIR}/log)
endif(reg STREQUAL "/")

add_definitions(-DPPICLIENTPATH="${FULL_CLIENT_PATH}")
add_definitions(-DPPICONFIGPATH="${FULL_CONFIG_PATH}")
add_definitions(-DPPIDATABASEPATH="${FULL_DATABASE_PATH}")
add_definitions(-DPPILOGPATH="${FULL_LOGGING_PATH}")

FIND_LIBRARY(RT_LIBRARIES NAMES rt)
MARK_AS_ADVANCED(RT_LIBRARIES)

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD OFF)
FIND_PACKAGE(Boost REQUIRED)

set(CMAKE_MODULE_PATH ../cmake)

#option(USE_LircClient "use lirc_client" ON)
if(USE_LircClient)
  find_package(LircClient REQUIRED)
  include_directories(${LircClient_INCLUDE_DIR})
  add_definitions(-D_LIRCCLIENTLIBRARY)
endif(USE_LircClient)

#option(USE_OPENSSL "use openSSL" ON)
if(USE_OPENSSL)
  find_package(OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIR})
  add_definitions(-D_OPENSSLLIBRARY)
endif(USE_OPENSSL)

option(USE_OWFS "use dallas/maxim semiconductors" OFF)
if(USE_OWFS)
  find_package(OWFS REQUIRED)
  include_directories(${OWFS_INCLUDE_DIR})
  add_definitions(-D_OWFSLIBRARY)
endif(USE_OWFS)

option(USE_K8055 "use Vellemann K8055 ports" OFF)
if(USE_K8055)
  find_package(USB REQUIRED)
  include_directories(${USB_INCLUDE_DIR})
  find_package(K8055 REQUIRED)
  include_directories(${K8055_INCLUDE_DIR})
  add_definitions(-D_K8055LIBRARY)
endif(USE_K8055)


add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/install.sh
		COMMAND ${CMAKE_SOURCE_DIR}/cmake/install.sh ${CMAKE_SOURCE_DIR} ${INSTALL_DEFAULTUSER} ${FULL_CLIENT_PATH} ${FULL_CONFIG_PATH} ${FULL_DATABASE_PATH} ${FULL_LOGGING_PATH})
add_executable(ppi-server ${mainSrcs} ${databaseSrcs} ${loggerSrcs} ${portsSrcs} ${portserverSrcs} ${serverSrcs} ${utilSrcs} ${CMAKE_CURRENT_BINARY_DIR}/install.sh)

target_link_libraries(ppi-server ${RT_LIBRARIES} ${OWFS_LIBRARIES} ${USB_LIBRARIES} ${K8055_LIBRARIES} ${OPENSSL_LIBRARIES} ${LircClient_LIBRARIES})

install(TARGETS ppi-server DESTINATION bin)
#option(FIRST_INSTALL "only istall directorys by frist installation" ON)
#if(FIRST_INSTALL)
#  install(DIRECTORY ${CMAKE_SOURCE_DIR}/client/ DESTINATION ${FULL_CLIENT_PATH})
#  install(DIRECTORY ${CMAKE_SOURCE_DIR}/conf/ DESTINATION ${FULL_CONFIG_PATH})
#  install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${FULL_DATABASE_PATH})
#  install(DIRECTORY ${CMAKE_SOURCE_DIR}/log/ DESTINATION ${FULL_LOGGING_PATH})
#endif(FIRST_INSTALL)

#install(DIRECTORY ${CMAKE_SOURCE_DIR}/log/ DESTINATION ${FULL_CLIENT_PATH})
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/log/ DESTINATION ${FULL_CONFIG_PATH})
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/log/ DESTINATION ${FULL_DATABASE_PATH})
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/log/ DESTINATION ${FULL_LOGGING_PATH})
