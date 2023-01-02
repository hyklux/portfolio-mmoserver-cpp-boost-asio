pushd %~dp0

GenProcs.exe --path=../../GameServer/GameDB.xml --output=GenProcedures.h

IF ERRORLEVEL 1 PAUSE

XCOPY /Y GenProcedures.h "../../GameServer"

DEL /Q /F *.h

PAUSE