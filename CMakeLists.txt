#--------------------------------------------------------------------------
# SPACEPARK 
#--------------------------------------------------------------------------
PROJECT(spacepark)
CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(ENV_ROOT ${CMAKE_CURRENT_DIR})
SET(CMAKE_CXX_STANDARD 17)

IF (MSVC)
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/bin>)
ELSE()
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
ENDIF()

#SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ...)


ADD_SUBDIRECTORY(exts)

SET(files main.cc)

SOURCE_GROUP("spacepark" FILES ${files})

ADD_EXECUTABLE(spacepark ${files})

TARGET_LINK_LIBRARIES(spacepark PUBLIC exts)

#FIND_PACKAGE(libconfig++ REQUIRED)
#TARGET_LINK_LIBRARIES(spacepark config++)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_CXX_FLAGS "-Wall -Wextra")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")

