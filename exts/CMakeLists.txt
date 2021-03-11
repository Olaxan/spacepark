#--------------------------------------------------------------------------
# exts
#--------------------------------------------------------------------------

ADD_LIBRARY(exts INTERFACE)

TARGET_INCLUDE_DIRECTORIES(exts INTERFACE sqlite)
TARGET_LINK_LIBRARIES(exts INTERFACE sqlite3)

IF (CMAKE_HOST_WIN32)
	SET(libname "libconfig")
ELSE()
	SET(libname "config")
ENDIF()

TARGET_INCLUDE_DIRECTORIES(exts INTERFACE "libconfig/lib")
TARGET_LINK_LIBRARIES(exts INTERFACE ${libname}++)
