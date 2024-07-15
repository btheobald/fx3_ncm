fx3_add_target(Fx3CmakeSample
    main.c
    ncm_desc.c
    cyfx/cyfx_tx.cpp
    cyfx/cyfx_gcc_startup.S
    cyfx/cyfxcpp_syscall.cpp
)

target_include_directories(Fx3CmakeSample PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/cyfx/include
)
