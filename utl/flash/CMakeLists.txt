cmake_minimum_required(VERSION 3.5)
project(download_fx3)
add_executable(download_fx3 download_fx3.cpp)
target_include_directories(download_fx3 PUBLIC include)
target_link_libraries(download_fx3 cyusb)
