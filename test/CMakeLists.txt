add_subdirectory(printing)

include(create_test.cmake)

create_test(
    TEST_NAME even_odd
    GRID 2x2
    MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/binaries/even_odd/manifest.json
    LOGGER even_odd.log
    # USE_GUI
)

create_test(
    TEST_NAME checksum
    GRID 2x2
    MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/binaries/checksum/manifest.json
    LOGGER checksum.log
    OUTPUT checksum_out.txt
    #  USE_GUI
)
