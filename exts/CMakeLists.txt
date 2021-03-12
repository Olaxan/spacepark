#--------------------------------------------------------------------------
# exts
#--------------------------------------------------------------------------

ADD_LIBRARY(exts INTERFACE)

#TARGET_INCLUDE_DIRECTORIES(exts INTERFACE "sqlite-amalgamation")
#TARGET_LINK_LIBRARIES(exts INTERFACE sqlite3)

FIND_PACKAGE(SQLite3 REQUIRED)
TARGET_LINK_LIBRARIES(exts INTERFACE SQLite::SQLite3)

FIND_PACKAGE(libconfig++ REQUIRED)
TARGET_LINK_LIBRARIES(exts INTERFACE config++)

#TARGET_INCLUDE_DIRECTORIES(exts INTERFACE "libconfig/lib")
