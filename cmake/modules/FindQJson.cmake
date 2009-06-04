# Copyright (c) 2009 Ingmar Vanhassel <ingmar@exherbo.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.

find_path(QJSON_INCLUDE_DIR json_driver.hh
  PATH_SUFFIXES qjson
)

find_library(QJSON_LIBRARIES NAMES qjson)

if (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)
  set(QJSON_FOUND TRUE)
endif (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)

if (QJSON_FOUND)
  if (NOT QJson_FIND_QUIETLY)
    message(STATUS "Found QJson: libs - ${QJSON_LIBRARIES}; includes - ${QJSON_INCLUDE_DIR}")
  endif (NOT QJson_FIND_QUIETLY)
else (QJSON_FOUND)
  if (QJson_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find QJson")
  endif (QJson_FIND_REQUIRED)
endif (QJSON_FOUND)

mark_as_advanced(
  QJSON_INCLUDE_DIR
  QJSON_LIBRARIES
)

