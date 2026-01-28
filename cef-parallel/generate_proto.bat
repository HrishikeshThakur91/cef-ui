@echo off
REM Proto generation script for cef-parallel
REM This script generates C++ proto files from cef_service.proto

echo [Proto Gen] Generating C++ proto files...

REM Check if protoc is available
where protoc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [Proto Gen] WARNING: protoc not found in PATH
    echo [Proto Gen] Skipping proto generation
    echo [Proto Gen] Install Protocol Buffers compiler: https://github.com/protocolbuffers/protobuf/releases
    exit /b 0
)

REM Check if grpc_cpp_plugin is available
where grpc_cpp_plugin >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [Proto Gen] WARNING: grpc_cpp_plugin not found in PATH
    echo [Proto Gen] Skipping proto generation
    echo [Proto Gen] Install gRPC tools: https://grpc.io/docs/languages/cpp/quickstart/
    exit /b 0
)

REM Navigate to proto directory
cd /d "%~dp0proto"

REM Generate C++ files
protoc --cpp_out=../src/grpc --grpc_out=../src/grpc --plugin=protoc-gen-grpc=grpc_cpp_plugin cef_service.proto

if %ERRORLEVEL% EQU 0 (
    echo [Proto Gen] SUCCESS: Generated cef_service.pb.h, cef_service.pb.cc
    echo [Proto Gen] SUCCESS: Generated cef_service.grpc.pb.h, cef_service.grpc.pb.cc
) else (
    echo [Proto Gen] ERROR: Proto generation failed
    exit /b 1
)

exit /b 0
