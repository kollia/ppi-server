# - Find lirc_client
# Find the native lirc_client includes and library
# This module defines
#  LircClient_INCLUDE_DIR, where to find lirc/lirc_client.h
#  LircClient_LIBRARIES, the libraries needed to use
#  LircClient_FOUND, If false, do not try to use lirc_client


FIND_PATH(LircClient_INCLUDE_DIR lirc/lirc_client.h)

FIND_LIBRARY(LircClient_LIBRARIES NAMES lirc_client)

IF (LircClient_LIBRARIES AND LircClient_INCLUDE_DIR)
  SET(LircClient_FOUND "YES")
ELSE (LircClient_LIBRARIES AND LircClient_INCLUDE_DIR)
  SET(LircClient_FOUND "NO")
ENDIF (LircClient_LIBRARIES AND LircClient_INCLUDE_DIR)

IF (LircClient_FOUND)
   IF (NOT LircClient_FIND_QUIETLY)
      MESSAGE(STATUS "Found LircClient: ${LircClient_LIBRARIES}")
   ENDIF (NOT LircClient_FIND_QUIETLY)
ELSE (LircClient_FOUND)
   IF (LircClient_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find lirc_client library, please configure LircClient_LIBRARIES and LircClient_INCLUDE_DIR where lirc/lirc_client.h")
   ENDIF (LircClient_FIND_REQUIRED)
ENDIF (LircClient_FOUND)

MARK_AS_ADVANCED(LircClient_LIBRARIES LircClient_INCLUDE_DIR)

