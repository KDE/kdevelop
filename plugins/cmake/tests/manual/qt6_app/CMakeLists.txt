project(qt6_app)

find_package(Qt6 REQUIRED Widgets)

add_executable(myprogram main.cpp)
target_link_libraries(myprogram Qt::Widgets)
