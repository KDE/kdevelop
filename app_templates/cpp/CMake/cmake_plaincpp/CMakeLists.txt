cmake_minimum_required(VERSION 3.10)

project(%{APPNAMELC})

add_executable(%{APPNAMELC} main.cpp)

install(TARGETS %{APPNAMELC} RUNTIME DESTINATION bin)
