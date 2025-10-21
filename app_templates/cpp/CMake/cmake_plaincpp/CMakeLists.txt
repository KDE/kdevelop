cmake_minimum_required(VERSION 3.16)

project(%{APPNAMELC})

add_executable(%{APPNAMELC} main.cpp)

install(TARGETS %{APPNAMELC} RUNTIME DESTINATION bin)
