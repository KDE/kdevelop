include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(kdeprojectsreadertest
    ../kdeprojectsreader.cpp
    ../kdeprojectsmodel.cpp
    kdeprojectsreadertest.cpp)
target_link_libraries(kdeprojectsreadertest Qt5::Test Qt5::Core Qt5::Gui Qt5::Network  ${KDE4_KDEUI_LIBS})

ecm_mark_as_test(kdeprojectsreadertest)
add_test(kdeprojectsreadertest kdeprojectsreadertest)
