cmake_minimum_required(VERSION 3.27)

project(W32Calc)

file(GLOB SRC "src/*.h" "src/*.cpp")
add_executable(W32Calc ${SRC})

target_link_libraries(W32Calc dwmapi)

#if(MSVC)
#    target_link_options(W32Calc PRIVATE "/SUBSYSTEM:CONSOLE")
#endif()