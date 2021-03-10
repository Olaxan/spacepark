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

SET(server_files 
	server.cc 
	parksrv.h 
	parksrv.cc 
	db.h 
	db.cc 
	session.h 
	session.cc
	protocol.h)

SET(config_files 
	config.cc 
	db.h 
	db.cc)

SOURCE_GROUP("spacepark_server" FILES ${server_files})
SOURCE_GROUP("spacepark_config" FILES ${config_files})

ADD_EXECUTABLE(spacepark-server ${server_files})
ADD_EXECUTABLE(spacepark-config ${config_files})

TARGET_LINK_LIBRARIES(spacepark-server PUBLIC exts)
TARGET_LINK_LIBRARIES(spacepark-config PUBLIC exts)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_CXX_FLAGS "-Wall -Wextra")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")

