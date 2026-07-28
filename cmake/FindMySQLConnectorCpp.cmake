find_path(MySQLConnectorCpp_ROOT_INCLUDE_DIR
  NAMES mysql_connection.h
  DOC "Directory containing mysql_connection.h")

find_path(MySQLConnectorCpp_CPPCONN_INCLUDE_DIR
  NAMES cppconn/driver.h
  DOC "Directory containing the cppconn headers")

find_library(MySQLConnectorCpp_LIBRARY
  NAMES mysqlcppconn
  DOC "MySQL Connector/C++ library")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MySQLConnectorCpp
  REQUIRED_VARS
    MySQLConnectorCpp_ROOT_INCLUDE_DIR
    MySQLConnectorCpp_CPPCONN_INCLUDE_DIR
    MySQLConnectorCpp_LIBRARY)

if(MySQLConnectorCpp_FOUND AND NOT TARGET MySQL::ConnectorCpp)
  add_library(MySQL::ConnectorCpp UNKNOWN IMPORTED)
  set_target_properties(MySQL::ConnectorCpp PROPERTIES
    IMPORTED_LOCATION "${MySQLConnectorCpp_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES
      "${MySQLConnectorCpp_ROOT_INCLUDE_DIR};${MySQLConnectorCpp_CPPCONN_INCLUDE_DIR}")
endif()

mark_as_advanced(
  MySQLConnectorCpp_ROOT_INCLUDE_DIR
  MySQLConnectorCpp_CPPCONN_INCLUDE_DIR
  MySQLConnectorCpp_LIBRARY)
