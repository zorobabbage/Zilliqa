set(CRYPTOUTILS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/depends/cryptoutils)
set(CRYPTOUTILS_BINARY_DIR ${CMAKE_BINARY_DIR}/src/depends/cryptoutils)
set(CRYPTOUTILS_INSTALL_DIR ${CMAKE_BINARY_DIR}/cryptoutils)
set(CRYPTOUTILS_INSTALL_LOG ${CMAKE_BINARY_DIR}/install_cryptoutils.log)

message(STATUS "Building and installing cryptoutils")

include(ProcessorCount)
ProcessorCount(N)

# update submodule to the latest commit
execute_process(
    COMMAND git submodule update --init --recursive --remote src/depends/cryptoutils
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE CRYPTOUTILS_INSTALL_RET
    OUTPUT_FILE ${CRYPTOUTILS_INSTALL_LOG}
    ERROR_FILE ${CRYPTOUTILS_INSTALL_LOG}
)

if(NOT "${CRYPTOUTILS_INSTALL_RET}" STREQUAL "0")
    message(FATAL_ERROR "Error when building and installing cryptoutils (1), see more in log ${CRYPTOUTILS_INSTALL_LOG}")
endif()


# # generate build directory
# execute_process(
#     COMMAND ${CMAKE_COMMAND}
#         -H${CRYPTOUTILS_SOURCE_DIR}
#         -B${CRYPTOUTILS_BINARY_DIR}
#         -DCMAKE_INSTALL_PREFIX=${CRYPTOUTILS_INSTALL_DIR}
#         -DCMAKE_BUILD_TYPE:STRING=Release
#         -Wno-dev
#     RESULT_VARIABLE CRYPTOUTILS_INSTALL_RET
#     OUTPUT_FILE ${CRYPTOUTILS_INSTALL_LOG}
#     ERROR_FILE ${CRYPTOUTILS_INSTALL_LOG}
# )

# if(NOT "${CRYPTOUTILS_INSTALL_RET}" STREQUAL "0")
#     message(FATAL_ERROR "Error when building and installing cryptoutils (2), see more in log ${CRYPTOUTILS_INSTALL_LOG}")
# endif()

# # build and install cryptoutils
# execute_process(
#     COMMAND ${CMAKE_COMMAND} --build ${CRYPTOUTILS_BINARY_DIR} -- -j${N}
#     RESULT_VARIABLE CRYPTOUTILS_INSTALL_RET
#     OUTPUT_FILE ${CRYPTOUTILS_INSTALL_LOG}
#     ERROR_FILE ${CRYPTOUTILS_INSTALL_LOG}
# )

# if(NOT "${CRYPTOUTILS_INSTALL_RET}" STREQUAL "0")
#     message(FATAL_ERROR "Error when building and installing cryptoutils (3), see more in log ${CRYPTOUTILS_INSTALL_LOG}")
# endif()

# execute_process(
#     COMMAND ${CMAKE_COMMAND} --build ${CRYPTOUTILS_BINARY_DIR} --target install
#     RESULT_VARIABLE CRYPTOUTILS_INSTALL_RET
#     OUTPUT_FILE ${CRYPTOUTILS_INSTALL_LOG}
#     ERROR_FILE ${CRYPTOUTILS_INSTALL_LOG}
# )

# if(NOT "${CRYPTOUTILS_INSTALL_RET}" STREQUAL "0")
#     message(FATAL_ERROR "Error when building and installing cryptoutils (4), see more in log ${CRYPTOUTILS_INSTALL_LOG}")
# endif()

# list(APPEND CMAKE_PREFIX_PATH ${CRYPTOUTILS_INSTALL_DIR})
# list(APPEND CMAKE_MODULE_PATH "${CRYPTOUTILS_INSTALL_DIR}/lib/cmake/cryptoutils")
# link_directories(${CRYPTOUTILS_INSTALL_DIR}/lib)
