add_subdirectory(printing)


function (create_test)

    set(FPGA_PLATFORM xilinx_u200_gen3x16_xdma_2_202110_1)

    set(HW_EMU_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hw_emu)

    set(options USE_GUI)
    set(singleTokenArgs GRID MANIFEST TIMEOUT TEST_NAME LOGGER)
    cmake_parse_arguments(TEST_CONFIGS
    "${options}"
    "${singleTokenArgs}"
    ""
    ${ARGN})

    set(TEST_ARGS "")

    if (NOT TEST_CONFIGS_TEST_NAME)
        message(FATAL_ERROR "TEST_NAME not specified!")
    endif()

    if (NOT TEST_CONFIGS_GRID)
        message(FATAL_ERROR "GRID not specified!")
    else()
        list(APPEND TEST_ARGS --xclbin ${HW_EMU_DIR}/${TEST_CONFIGS_GRID}/ManticoreKernel.hw_emu.${FPGA_PLATFORM}.xclbin)
    endif()

    if (TEST_CONFIGS_LOGGER)
        list(APPEND TEST_ARGS --log ${TEST_CONFIGS_LOGGER})
    endif()


    if (TEST_CONFIGS_TIMEOUT)
        list(APPEND TEST_ARGS --timeout ${TEST_CONFIGS_TIMEOUT})
    endif()

    if (NOT TEST_CONFIGS_MANIFEST)
        message(FATAL_ERROR "MANIFEST not specified!")
    else()
        list(APPEND TEST_ARGS ${TEST_CONFIGS_MANIFEST})
    endif()

    add_test(
        NAME ${TEST_CONFIGS_TEST_NAME}
        COMMAND manticore ${TEST_ARGS}
    )

    set (TEST_ENV XCL_EMULATION_MODE=hw_emu
                  EMCONFIG_PATH=${CMAKE_CURRENT_SOURCE_DIR}/)



    if(TEST_CONFIGS_USE_GUI)
        list(APPEND TEST_ENV XRT_INI_PATH=${CMAKE_CURRENT_SOURCE_DIR}/xrt_gui.ini)
    endif()


    set_property(
        TEST ${TEST_CONFIGS_TEST_NAME}
        PROPERTY ENVIRONMENT ${TEST_ENV}
    )


endfunction()



create_test(
    TEST_NAME even_odd
    GRID 2x2
    MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/binaries/even_odd/manifest.json
    LOGGER even_odd.log
    USE_GUI
)