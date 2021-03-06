#--------------------------------------------------------------------------
# SPACEPARK 
#--------------------------------------------------------------------------
PROJECT(spacepark)
CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(ENV_ROOT ${CMAKE_CURRENT_DIR})
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_CXX_FLAGS "-Wall -Wextra")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")

SET(server_files 
	server.cc 
	parksrv.h 
	parksrv.cc 
	db.h 
	db.cc 
	protocol.h)

SET(config_files 
	config.cc 
	db.h 
	db.cc)

ADD_SUBDIRECTORY(exts)

SOURCE_GROUP("spacepark_server" FILES ${server_files})
SOURCE_GROUP("spacepark_config" FILES ${config_files})

ADD_EXECUTABLE(spacepark-server ${server_files})
ADD_EXECUTABLE(spacepark-config ${config_files})

ADD_DEPENDENCIES(spacepark-server exts)
ADD_DEPENDENCIES(spacepark-config exts)

TARGET_LINK_LIBRARIES(spacepark-server PUBLIC exts)
TARGET_LINK_LIBRARIES(spacepark-config PUBLIC exts)
