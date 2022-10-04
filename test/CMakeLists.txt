# add_subdirectory(printing)

include(create_test.cmake)


file(GLOB VERILOG_UNITS "${CMAKE_CURRENT_SOURCE_DIR}/verilog/*.v")


foreach(vfile IN ITEMS ${VERILOG_UNITS})
    # message ("${vfile}")
    get_filename_component(target ${vfile} NAME_WE)
    compile_verilog(
        TARGET ${target}_compile
        SOURCES ${vfile}
        DIMX 2 DIMY 2
    )
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/reference/${target}.txt)
        create_test(
            TEST_NAME ${target}
            GRID 2x2
            MANIFEST ${target}_compile/manifest.json
            OUTPUT ${target}_out.txt
            LOGGER ${target}_runtime.log
            CHECK_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/reference/${target}.txt
            DEPENDS ${target_compile}
        )
    else()
        create_test(
            TEST_NAME ${target}
            GRID 2x2
            MANIFEST ${target}_compile/manifest.json
            OUTPUT ${target}_out.txt
            LOGGER ${target}_runtime.log
            DEPENDS ${target}_compile
        )
    endif()

endforeach()



