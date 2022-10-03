

function (compile_verilog)


    set(singleTokenArgs DIMX DIMY TARGET)
    set(multiTokenArgs SOURCES)
    cmake_parse_arguments(TEST_CONFIGS
        ""
        "${singleTokenArgs}"
        "${multiTokenArgs}"
        ${ARGN})

    if (NOT TEST_CONFIGS_TARGET)
        message(FATAL_ERROR "TARGET is required")
    endif()
    if (NOT TEST_CONFIGS_SOURCES)
        message(FATAL_ERROR "SOURCES is required")
    endif()
    if (NOT TEST_CONFIGS_DIMX)
        message(FATAL_ERROR "DIMX is required")
    endif()

    if (NOT TEST_CONFIGS_DIMY)
        message(FATAL_ERROR "DIMY is required")
    endif()


    set(TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TEST_CONFIGS_TARGET})

    if (NOT DEFINED MASM)
        message(WARNING "Please set MASM for testing with cmake -DMASM=... to point to manticore compiler")
    endif()


    add_custom_command(
        OUTPUT ${TARGET_DIR}/manifest.json
        COMMAND ${MASM}
            -x ${TEST_CONFIGS_DIMX}
            -y ${TEST_CONFIGS_DIMY}
            -o ${TARGET_DIR}
            --dump-ascii --dump-register-file --dump-scratch-pad
            ${TEST_CONFIGS_SOURCES}
        COMMENT "Compiling ${TEST_CONFIGS_SOURCES} for ${TEST_CONFIGS_DIMX}x${TEST_CONFIGS_DIMY}"
    )


    add_custom_target(
        ${TEST_CONFIGS_TARGET}
        DEPENDS ${TARGET_DIR}/manifest.json
    )
    # set(
    #     ${TEST_CONFIGS_TARGET}_manifest
    #     ${TARGET_DIR}/manifest.json
    #     PARENT_SCOPE
    # )

endfunction()



function (create_test)

    set(FPGA_PLATFORM xilinx_u200_gen3x16_xdma_2_202110_1)

    set(HW_EMU_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hw_emu)

    set(options USE_GUI)
    set(singleTokenArgs
        GRID
        MANIFEST
        TIMEOUT
        TEST_NAME
        LOGGER
        OUTPUT
        CHECK_OUTPUT
        DEPENDS
        )
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

    if (TEST_CONFIGS_OUTPUT)
        list(APPEND TEST_ARGS --output ${TEST_CONFIGS_OUTPUT})
    elseif(TEST_CONFIGS_CHECK_OUTPUT)
        message(FATAL_ERROR "CHECK_OUTPUT requires OUTPUT")
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
        NAME ${TEST_CONFIGS_TEST_NAME}_run
        COMMAND manticore ${TEST_ARGS}
    )

    set (TEST_ENV XCL_EMULATION_MODE=hw_emu
                  EMCONFIG_PATH=${CMAKE_CURRENT_SOURCE_DIR}/)



    if(TEST_CONFIGS_USE_GUI)
        list(APPEND TEST_ENV XRT_INI_PATH=${CMAKE_CURRENT_SOURCE_DIR}/xrt_gui.ini)
    endif()



    set_property(
        TEST ${TEST_CONFIGS_TEST_NAME}_run
        PROPERTY ENVIRONMENT ${TEST_ENV}
    )

    if (TEST_CONFIGS_DEPENDS)
        set_tests_properties(
            ${TEST_CONFIGS_TEST_NAME}_run PROPERTIES DEPENDS ${TEST_CONFIGS_DEPENDS}
        )
    endif()

    if (TEST_CONFIGS_CHECK_OUTPUT)

        add_test(
            NAME ${TEST_CONFIGS_TEST_NAME}
            COMMAND diff ${TEST_CONFIGS_OUTPUT} ${TEST_CONFIGS_CHECK_OUTPUT}
        )

        set_tests_properties(
            ${TEST_CONFIGS_TEST_NAME} PROPERTIES
            DEPENDS ${TEST_CONFIGS_TEST_NAME}_run
        )

    endif()


endfunction()


