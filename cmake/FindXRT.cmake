# Find Xilinx XRT library using the XILINX_XRT environment variable

find_path(
    XRT_PATH
    NAMES include/xrt.h
    PATHS ${XILINX_XRT} ENV XILINX_XRT
)

if(NOT EXISTS ${XRT_PATH})
    set(XRT_FOUND FALSE)
    if (${XRT_FIND_REQUIRED})
        message(FATAL_ERROR "Can not find XRT. Set XILINX_XRT environment variable or provide it via -DXILINX_XRT")
    else()
        message(WARNING "Can not find XRT. Set XILINX_XRT environment variable or provide it via -DXILINX_XRT")
    endif()
else()
    message(STATUS "XRT_PATH=${XRT_PATH}")
    set(XRT_FOUND TRUE)
    set(XRT_INCLUDE_DIR ${XRT_PATH}/include)
    set(XRT_LIB_DIR ${XRT_PATH}/lib)
    find_library(XRT_LIBRARIES xrt_coreutil PATHS ${XRT_LIB_DIR})

endif()
