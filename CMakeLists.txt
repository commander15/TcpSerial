cmake_minimum_required(VERSION 3.20)

project(TcpSerial
    DESCRIPTION "Tcp to Serial Adapter"
    VERSION     1.0.0
    LANGUAGES   CXX C
)

find_package(Commander REQUIRED)
find_package(Qt5 REQUIRED Core Network SerialPort)

add_subdirectory(src)
