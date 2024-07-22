#project(ncm_class_test)

add_library(ncm_class 
    struct.c
    net.c
    rx.c
    tx.c
)

target_include_directories(ncm_class PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

#add_executable(ncm_class_test main.c)

#target_link_libraries(ncm_class_test ncm_class)