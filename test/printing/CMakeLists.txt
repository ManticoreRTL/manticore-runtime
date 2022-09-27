


add_executable(printing_test printing.cpp)
target_link_libraries(printing_test PUBLIC mantilib)


add_test(
    Name printing_test
    COMMAND printing_test ${CMAKE_CURRENT_SOURCE_DIR}/config1.json
)