

cmake_minimum_required (VERSION 2.6)
project (ppi-server-project)

set(USE_SHAREDLIBS ON)
if(USE_SHAREDLIBS)
	set(libraries SHARED)
else(USE_SHAREDLIBS)
	set(libraries STATIC)
endif(USE_SHAREDLIBS)

#SET(CMAKE_VERBOSE_MAKEFILE ON)

set(RELEASE_FLAG_NR 03 CACHE STRING "release flag number 02 or 03 (only for creating an release)")
if(	RELEASE_FLAG_NR STREQUAL "02" OR
	RELEASE_FLAG_NR STREQUAL "03"	)
	
	set(RELEASE_FLAG_NR ${RELEASE_FLAG_NR})
	
else(	RELEASE_FLAG_NR STREQUAL "02" OR
		RELEASE_FLAG_NR STREQUAL "03"	)
		
	set(RELEASE_FLAG_NR 02)
		
endif(	RELEASE_FLAG_NR STREQUAL "02" OR
		RELEASE_FLAG_NR STREQUAL "03"	)
		
set(CMAKE_CXX_FLAGS_RELEASE "-${RELEASE_FLAG_NR} -DNDEBUG -mtune=native")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3") 

if(	CMAKE_BUILD_TYPE STREQUAL "Debug"	OR
	CMAKE_BUILD_TYPE STREQUAL "debug"	OR
	CMAKE_ECLIPSE_EXECUTABLE			OR
	CMAKE_KDEVELOP3_EXECUTABLE				)
		
        add_definitions("-Wall -Wclobbered -Wempty-body -Wignored-qualifiers 
                         -Wmissing-field-initializers -Wsign-compare -Wtype-limits 
                         -Winit-self -Wnon-virtual-dtor -Wsign-promo -Wno-long-long -fPIC ")
                      # debug options make memory checks
                      # make binary slower, do not implement all the time
                       #  -fsanitize=address
                       #  -fsanitize=thread 
                       #  -fsanitize=leak
                       #  -fsanitize=undefined  ")

	# for warning optins see http://gcc.gnu.org/onlinedocs/gcc-4.5.0/gcc/Warning-Options.html#Warning-Options
	#add_definitions("-pedantic-errors -Wall -Wextra -ansi -Winit-self -Woverloaded-virtual -Wnon-virtual-dtor -Wsign-promo -Wno-long-long")
	message(STATUS "DEBUG flags   '${CMAKE_CXX_FLAGS_DEBUG}'")
	message(STATUS "set build type with all warnings")

else(	CMAKE_BUILD_TYPE STREQUAL "Debug"	OR
		CMAKE_BUILD_TYPE STREQUAL "debug"	OR
		CMAKE_ECLIPSE_EXECUTABLE				OR
		CMAKE_KDEVELOP3_EXECUTABLE					)
	
	add_definitions("-fPIC")
	message(STATUS "RELEASE flags '${CMAKE_CXX_FLAGS_RELEASE}'")	
	message(STATUS "set build type with no warnings")
		
endif(	CMAKE_BUILD_TYPE STREQUAL "Debug"	OR
		CMAKE_BUILD_TYPE STREQUAL "debug"	OR
		CMAKE_ECLIPSE_EXECUTABLE				OR
		CMAKE_KDEVELOP3_EXECUTABLE					)

#if(	${CMAKE_BUILD_TYPE} STREQUAL "Debug"	OR
#	${CMAKE_BUILD_TYPE} STREQUAL "debug"	OR
#	CMAKE_ECLIPSE_EXECUTABLE				OR
#	CMAKE_KDEVELOP3_EXECUTABLE					)
	
	
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
message(STATUS "set EXECUTABLE path '${CMAKE_SOURCE_DIR}/bin'")
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
message(STATUS "set LIBRARY path '${CMAKE_SOURCE_DIR}/lib'")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)



set(PPIPATTERN_LIB_VERSION "1.0")
set(PPIPATTERN_LIB_SOVERSION "1")

set(PPIDB_LIB_VERSION "1.0")
set(PPIDB_LIB_SOVERSION "1")
set(PPIDBINTERFACE_LIB_VERSION "1.0")
set(PPIDBINTERFACE_LIB_SOVERSION "1")

set(PPISUBROUTINES_LIB_VERSION "1.0")
set(PPISUBROUTINES_LIB_SOVERSION "1")
set(PPIPORTS_LIB_VERSION "1.0")
set(PPIPORTS_LIB_SOVERSION "1")

set(PPISERVERCLIENT_LIB_VERSION "1.0")
set(PPISERVERCLIENT_LIB_SOVERSION "1")

set(PPISERVERSERVER_LIB_VERSION "1.0")
set(PPISERVERSERVER_LIB_SOVERSION "1")

set(PPISERVER_LIB_VERSION "1.0")
set(PPISERVER_LIB_SOVERSION "1")

set(PPIOW_LIB_VERSION "1.0")
set(PPIOW_LIB_SOVERSION "1")

set(PPILOGGER_LIB_VERSION "1.0")
set(PPILOGGER_LIB_SOVERSION "1")

set(PPIUTIL_LIB_VERSION "1.0")
set(PPIUTIL_LIB_SOVERSION "1")
set(PPISTREAMUTIL_LIB_VERSION "1.0")
set(PPISTREAMUTIL_LIB_SOVERSION "1")
set(PPIPROPERTYUTIL_LIB_VERSION "1.0")
set(PPIPROPERTYUTIL_LIB_SOVERSION "1")
set(PPITHREADUTIL_LIB_VERSION "1.0")
set(PPITHREADUTIL_LIB_SOVERSION "1")
set(PPIPROCESSUTIL_LIB_VERSION "1.0")
set(PPIPROCESSUTIL_LIB_SOVERSION "1")

set(PPICPPCLIENT_LIB_VERSION "0.1")
set(PPICPPCLIENT_LIB_SOVERSION "0")

set(PPI_LIBPORTSACCESS_VERSION "1.0")
set(PPI_LIBPORTSACCESS_SOVERSION "1")
set(PPI_LIBOWFSACCESS_VERSION "1.0")
set(PPI_LIBOWFSACCESS_SOVERSION "1")
set(PPI_LIBLIRCACCESS_VERSION "1.0")
set(PPI_LIBLIRCACCESS_SOVERSION "1")
set(PPI_LIBVK8055ACCESS_VERSION "1.0")
set(PPI_LIBVK8055ACCESS_SOVERSION "1")



set(INSTALL_CLIENT_PATH /var/local CACHE PATH CLIENT_PATH)
set(INSTALL_CONFIG_PATH /usr/local/etc CACHE PATH CONFIG_PATH)
set(INSTALL_DATABASE_PATH /var/local CACHE PATH DATABASE_PATH)
set(INSTALL_LOGGING_PATH /var/log CACHE PATH LOGGING_PATH)
set(INSTALL_DEFAULTUSER nobody CACHE STRING DEFAULTUSER)

if(DISTRIBUTION_RELEASE)
	add_definitions(-DDISTRIBUTION_RELEASE="${DISTRIBUTION_RELEASE}")
endif(DISTRIBUTION_RELEASE)


if(INSTALL_CLIENT_PATH)
  string(REGEX MATCH "^." reg ${INSTALL_CLIENT_PATH})
  if(reg STREQUAL "/")
    set(FULL_CLIENT_PATH ${INSTALL_CLIENT_PATH}/ppi-server/client)
  #else(reg STREQUAL "/")
  #  set(FULL_CLIENT_PATH ${CMAKE_SOURCE_DIR}/client)
  endif(reg STREQUAL "/")
endif(INSTALL_CLIENT_PATH)

if(INSTALL_CONFIG_PATH)
  string(REGEX MATCH "^." reg ${INSTALL_CONFIG_PATH})
  if(reg STREQUAL "/")
    set(FULL_CONFIG_PATH ${INSTALL_CONFIG_PATH}/ppi-config)
  #else(reg STREQUAL "/")
  #  set(FULL_CLIENT_PATH ${CMAKE_SOURCE_DIR}/conf)
  endif(reg STREQUAL "/")
endif(INSTALL_CONFIG_PATH)

if(INSTALL_DATABASE_PATH)
  string(REGEX MATCH "^." reg ${INSTALL_DATABASE_PATH})
  if(reg STREQUAL "/")
    set(FULL_DATABASE_PATH ${INSTALL_DATABASE_PATH}/ppi-server/database)
  #else(reg STREQUAL "/")
  #  set(FULL_CLIENT_PATH ${CMAKE_SOURCE_DIR}/data)
  endif(reg STREQUAL "/")
endif(INSTALL_DATABASE_PATH)

if(INSTALL_LOGGING_PATH)
  string(REGEX MATCH "^." reg ${INSTALL_LOGGING_PATH})
  if(reg STREQUAL "/")
    set(FULL_LOGGING_PATH ${INSTALL_LOGGING_PATH}/ppi-server)
  #else(reg STREQUAL "/")
  #  set(FULL_CLIENT_PATH ${CMAKE_SOURCE_DIR}/log)
  endif(reg STREQUAL "/")
endif(INSTALL_LOGGING_PATH)

if(INSTALL_CONFIG_PATH)
  add_definitions(-DPPICLIENTPATH="${FULL_CLIENT_PATH}")
  add_definitions(-DPPICONFIGPATH="${FULL_CONFIG_PATH}")
  add_definitions(-DPPIDATABASEPATH="${FULL_DATABASE_PATH}")
  add_definitions(-DPPILOGPATH="${FULL_LOGGING_PATH}")
endif(INSTALL_CONFIG_PATH)

FIND_LIBRARY(RT_LIBRARIES NAMES rt)
MARK_AS_ADVANCED(RT_LIBRARIES)

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD OFF)
FIND_PACKAGE(Boost REQUIRED COMPONENTS regex)

option(USE_LircClient "use lirc_client" ON)
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

# libraries
add_subdirectory( src/pattern/util ) # ppipattern need: ---

add_subdirectory( src/ports/subroutines ) # ppiportssubroutine need: ---
add_subdirectory( src/util/stream ) # ppistreamutil need: ---
add_subdirectory( src/util/properties ) # ppipropertyutil need: ppistreamutil ppipattern
add_subdirectory( src/util ) # ppiutil need: ppipropertyutil ppistreamutil ppipattern
add_subdirectory( src/util/thread ) # ppithreadutil need: rt ppiutil ppipattern
add_subdirectory( src/portserver/libs/chipaccess ) # need: ppiutil

add_subdirectory( src/server/libs ) # ppiserverlib need: ppithreadutil
add_subdirectory( src/server/libs/client ) # ppiserverlibclient need: ppiserverlib
add_subdirectory( src/util/process ) # ppiprocessutil need: ppistreamutil ppithreadutil ppiserverlibclient
add_subdirectory( src/database/logger/lib ) # ppilogger need: ppithreadutil ppistreamutil ppiserverlibclient
add_subdirectory( src/server/libs/server ) # ppiserverlibserver need: ppiprocessutil ppiserverlib ppiserverlibclient ppipattern
add_subdirectory( src/database/lib ) # ppidb need: ppithreadutil ppistreamutil ppiserverlibclient ppipattern
add_subdirectory( src/portserver/libs/interface ) # ppiow need: ppistreamutil ppiserverlibclient ppipattern
#add_subdirectory( src/ports ) # ppiports need: ppiutil ppithreadutil ppiserverlibclient ppidb ppiow ppipattern
add_subdirectory( src/client/lib ) # ppicppclient need: ppiutil ppistreamutil ppithreadutil ppiserverlibclient


# binaries
add_subdirectory( src/config )
add_subdirectory( src/server )
add_subdirectory( src/database )
add_subdirectory( src/portserver )
add_subdirectory( src/client )
add_subdirectory( src )

