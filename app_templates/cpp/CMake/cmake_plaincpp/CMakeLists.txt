cmake_minimum_required(VERSION 2.6)
project(%{APPNAMELC})

add_executable(%{APPNAMELC} main.cpp)

install(TARGETS %{APPNAMELC} RUNTIME DESTINATION bin)
