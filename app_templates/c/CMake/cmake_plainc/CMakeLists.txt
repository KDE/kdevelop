cmake_minimum_required(VERSION 3.10)

project(%{APPNAMELC} LANGUAGES C)

add_executable(%{APPNAMELC} main.c)

install(TARGETS %{APPNAMELC} RUNTIME DESTINATION bin)
