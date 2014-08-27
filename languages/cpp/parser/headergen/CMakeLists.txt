add_executable(headergen TEST main.cpp)
target_link_libraries(headergen Qt5::Xml  ${KDEVPLATFORM_LANGUAGE_LIBRARIES} kdevcpprpp kdevcppparser)
