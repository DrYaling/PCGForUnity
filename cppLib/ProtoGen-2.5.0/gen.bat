@echo off & color 0A

:: protoc程序名
set "PROTOC_EXE=protoc.exe"
:: .proto文件名
set "PROTOC_FILE_NAME=punch.proto"

set "PROTOC_PATH=%cd%"
set "CPP_OUT_PATH=%cd%"

::生成.h和.cc
"%PROTOC_PATH%\%PROTOC_EXE%" --proto_path="%PROTOC_PATH%" --cpp_out="%CPP_OUT_PATH%" "%PROTOC_PATH%\%PROTOC_FILE_NAME%"
pause