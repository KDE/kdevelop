project(%{APPNAMELC})

add_executable(%{APPNAMELC} main.cpp)

install(TARGETS %{APPNAMELC} RUNTIME DESTINATION bin)
