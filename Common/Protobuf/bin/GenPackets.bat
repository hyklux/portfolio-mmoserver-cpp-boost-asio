pushd %~dp0
protoc.exe -I=./ --cpp_out=./ ./Enum.proto
protoc.exe -I=./ --cpp_out=./ ./Struct.proto
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto

GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
GenPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --recv=S_ --send=C_

IF ERRORLEVEL 1 PAUSE

XCOPY /Y Enum.pb.h "../../../GameServer"
XCOPY /Y Enum.pb.cc "../../../GameServer"
XCOPY /Y Struct.pb.h "../../../GameServer"
XCOPY /Y Struct.pb.cc "../../../GameServer"
XCOPY /Y Protocol.pb.h "../../../GameServer"
XCOPY /Y Protocol.pb.cc "../../../GameServer"
XCOPY /Y ClientPacketHandler.h "../../../GameServer"

XCOPY /Y Enum.pb.h "../../../DummyClient"
XCOPY /Y Enum.pb.cc "../../../DummyClient"
XCOPY /Y Struct.pb.h "../../../DummyClient"
XCOPY /Y Struct.pb.cc "../../../DummyClient"
XCOPY /Y Protocol.pb.h "../../../DummyClient"
XCOPY /Y Protocol.pb.cc "../../../DummyClient"
XCOPY /Y ServerPacketHandler.h "../../../DummyClient"

XCOPY /Y Enum.pb.h "../../../ServerCore"
XCOPY /Y Enum.pb.cc "../../../ServerCore"
XCOPY /Y Struct.pb.h "../../../ServerCore"
XCOPY /Y Struct.pb.cc "../../../ServerCore"
XCOPY /Y Protocol.pb.h "../../../ServerCore"
XCOPY /Y Protocol.pb.cc "../../../ServerCore"
XCOPY /Y ServerPacketHandler.h "../../../ServerCore"

XCOPY /Y Enum.pb.h "../../../NetworkServer"
XCOPY /Y Enum.pb.cc "../../../NetworkServer"
XCOPY /Y Struct.pb.h "../../../NetworkServer"
XCOPY /Y Struct.pb.cc "../../../NetworkServer"
XCOPY /Y Protocol.pb.h "../../../NetworkServer"
XCOPY /Y Protocol.pb.cc "../../../NetworkServer"
XCOPY /Y ServerPacketHandler.h "../../../NetworkServer"

XCOPY /Y Enum.pb.h "../../../ConnectorServer"
XCOPY /Y Enum.pb.cc "../../../ConnectorServer"
XCOPY /Y Struct.pb.h "../../../ConnectorServer"
XCOPY /Y Struct.pb.cc "../../../ConnectorServer"
XCOPY /Y Protocol.pb.h "../../../ConnectorServer"
XCOPY /Y Protocol.pb.cc "../../../ConnectorServer"
XCOPY /Y ServerPacketHandler.h "../../../ConnectorServer"

XCOPY /Y Enum.pb.h "../../../UserServer"
XCOPY /Y Enum.pb.cc "../../../UserServer"
XCOPY /Y Struct.pb.h "../../../UserServer"
XCOPY /Y Struct.pb.cc "../../../UserServer"
XCOPY /Y Protocol.pb.h "../../../UserServer"
XCOPY /Y Protocol.pb.cc "../../../UserServer"
XCOPY /Y ServerPacketHandler.h "../../../UserServer"

XCOPY /Y Enum.pb.h "../../../ZoneServer"
XCOPY /Y Enum.pb.cc "../../../ZoneServer"
XCOPY /Y Struct.pb.h "../../../ZoneServer"
XCOPY /Y Struct.pb.cc "../../../ZoneServer"
XCOPY /Y Protocol.pb.h "../../../ZoneServer"
XCOPY /Y Protocol.pb.cc "../../../ZoneServer"
XCOPY /Y ServerPacketHandler.h "../../../ZoneServer"

XCOPY /Y Enum.pb.h "../../../ChatServer"
XCOPY /Y Enum.pb.cc "../../../ChatServer"
XCOPY /Y Struct.pb.h "../../../ChatServer"
XCOPY /Y Struct.pb.cc "../../../ChatServer"
XCOPY /Y Protocol.pb.h "../../../ChatServer"
XCOPY /Y Protocol.pb.cc "../../../ChatServer"
XCOPY /Y ServerPacketHandler.h "../../../ChatServer"

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h