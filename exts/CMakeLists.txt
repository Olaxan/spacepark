#--------------------------------------------------------------------------
# exts
#--------------------------------------------------------------------------

ADD_LIBRARY(exts INTERFACE)

TARGET_INCLUDE_DIRECTORIES(exts INTERFACE sqlite)
TARGET_LINK_LIBRARIES(exts INTERFACE sqlite3)

TARGET_INCLUDE_DIRECTORIES(exts INTERFACE "libconfig/lib")
TARGET_LINK_LIBRARIES(exts INTERFACE config++)
