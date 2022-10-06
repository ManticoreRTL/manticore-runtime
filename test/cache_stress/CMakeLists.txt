

compile_verilog(
    TARGET fifo_small_stress_compile
    DIMX 1 DIMY 1
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/fifo.v ${CMAKE_CURRENT_SOURCE_DIR}/fifo_small.v
)

compile_verilog(
    TARGET fifo_large_stress_compile
    DIMX 1 DIMY 1
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/fifo.v ${CMAKE_CURRENT_SOURCE_DIR}/fifo_large.v
)

compile_verilog(
    TARGET fifo_extra_large_stress_compile
    DIMX 1 DIMY 1
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/fifo.v ${CMAKE_CURRENT_SOURCE_DIR}/fifo_extra_large.v
)




compile_verilog(
    TARGET random_access_small_stress_compile
    DIMX 1 DIMY 1
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/random_access.v ${CMAKE_CURRENT_SOURCE_DIR}/random_access_small.v
)

compile_verilog(
    TARGET random_access_large_stress_compile
    DIMX 1 DIMY 1
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/random_access.v ${CMAKE_CURRENT_SOURCE_DIR}/random_access_large.v
)

compile_verilog(
    TARGET random_access_extra_large_stress_compile
    DIMX 1 DIMY 1
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/random_access.v ${CMAKE_CURRENT_SOURCE_DIR}/random_access_extra_large.v
)

add_custom_target(
    cache_stress_ubench_compile
)

add_dependencies(
    cache_stress_ubench_compile

    fifo_small_stress_compile
    fifo_large_stress_compile
    fifo_extra_large_stress_compile
    random_access_small_stress_compile
    random_access_large_stress_compile
    random_access_extra_large_stress_compile

)