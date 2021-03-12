#--------------------------------------------------------------------------
# exts
#--------------------------------------------------------------------------

ADD_LIBRARY(exts INTERFACE)

FIND_PACKAGE(SQLite3 REQUIRED)
TARGET_LINK_LIBRARIES(exts INTERFACE SQLite::SQLite3)

FIND_PACKAGE(libconfig++ REQUIRED)
TARGET_INCLUDE_DIRECTORIES(exts INTERFACE "libconfig/lib")
TARGET_LINK_LIBRARIES(exts INTERFACE config++)
