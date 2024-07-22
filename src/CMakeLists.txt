fx3_add_target(Fx3CmakeSample
    main.c
    usb_desc.c
    cyfx/cyfx_tx.c
    cyfx/cyfx_gcc_startup.S
)

target_include_directories(Fx3CmakeSample PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/cyfx/include
)

target_link_libraries(Fx3CmakeSample
    ncm_class    
    lwip
)