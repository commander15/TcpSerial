add_tool(tcpserial main.cpp)

configure_file(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

target_link_libraries(tcpserial PRIVATE
    Qt5::Core Qt5::Network Qt5::SerialPort
)

install(TARGETS tcpserial
    TOOL DESTINATION bin
)
