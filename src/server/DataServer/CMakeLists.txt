mu_add_server(DataServer
  AllowableIpList.cpp
  BadSyntax.cpp
  CharacterManager.cpp
  CommandManager.cpp
  Console.cpp
  CriticalSection.cpp
  DataServerLinux.cpp
  DataServerProtocol.cpp
  GoldenArcher.cpp
  Guild.cpp
  GuildManager.cpp
  Log.cpp
  MiniDump.cpp
  QueryManager.cpp
  Queue.cpp
  ReadScript.cpp
  ServerDisplayer.cpp
  ServerManager.cpp
  SocketManagerLinux.cpp
  stdafx.cpp
  Util.cpp
  Warehouse.cpp)

target_compile_definitions(DataServer PRIVATE MYSQL)
target_link_libraries(DataServer PRIVATE MySQL::ConnectorCpp)
