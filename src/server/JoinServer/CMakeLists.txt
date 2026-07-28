mu_add_server(JoinServer
  AccountManager.cpp
  AllowableIpList.cpp
  Console.cpp
  CriticalSection.cpp
  JoinServerLinux.cpp
  JoinServerProtocol.cpp
  Log.cpp
  MD5.cpp
  MiniDump.cpp
  QueryManager.cpp
  Queue.cpp
  ReadScript.cpp
  ServerDisplayer.cpp
  ServerManager.cpp
  SocketManagerLinux.cpp
  SocketManagerUdpLinux.cpp
  stdafx.cpp
  Util.cpp)

target_compile_definitions(JoinServer PRIVATE MYSQL)
target_link_libraries(JoinServer PRIVATE MySQL::ConnectorCpp)
