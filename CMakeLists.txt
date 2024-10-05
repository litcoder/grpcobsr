cmake_minimum_required(VERSION 3.15)

project(obsrgrpc)

set(OBSR_LIB obsr)

# C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-g -O0")

# Options
option(BUILD_UNITTEST "Build unittests" on)

include(FetchContent)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.4.1
)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "")

# gRPC
FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc
    GIT_TAG v1.66.1
)
set(FETCHCONTENT_QUIET OFF)
set(ABSL_ENABLE_INSTALL ON)

FetchContent_MakeAvailable(spdlog gRPC)

# Source code
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
file(GLOB OBSR_LIB_SRC "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB OBSR_TEST_SRC "${PROJECT_SOURCE_DIR}/test/*.cpp")
file(GLOB OBSR_HEADER "${PROJECT_INCLUDE_DIR}/obsr/*.hpp")

# Proto file
set(PROTO_FILE_NAME "stockprice")
get_filename_component(
    OBSR_PROTO_FILE
    "${PROJECT_SOURCE_DIR}/proto/${PROTO_FILE_NAME}.proto"
    ABSOLUTE)
get_filename_component(
    OBSR_PROTO_DIR
    "${OBSR_PROTO_FILE}"
    PATH)

# Generate gRPC source
set(OBSR_PROTOGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/obsr_gen")
set(OBSR_PB_SRC "${OBSR_PROTOGEN_DIR}/${PROTO_FILE_NAME}.pb.cc")
set(OBSR_PB_HDR "${OBSR_PROTOGEN_DIR}/${PROTO_FILE_NAME}.pb.h")
set(OBSR_GRPC_SRC "${OBSR_PROTOGEN_DIR}/${PROTO_FILE_NAME}.grpc.pb.cc")
set(OBSR_GRPC_HDR "${OBSR_PROTOGEN_DIR}/${PROTO_FILE_NAME}.grpc.pb.h")

set(PROTOC_EXE $<TARGET_FILE:protobuf::protoc>)
set(GRPC_CPP_PLUGIN $<TARGET_FILE:grpc_cpp_plugin>)
file(MAKE_DIRECTORY ${OBSR_PROTOGEN_DIR})

add_custom_command(
    OUTPUT "${OBSR_PB_SRC}" "${OBSR_PB_HDR}" "${OBSR_GRPC_SRC}" "${OBSR_GRPC_HDR}"
    COMMAND ${PROTOC_EXE}
    ARGS --proto_path "${grpc_SOURCE_DIR}/third_party/bloaty/third_party/protobuf/src/"
    --grpc_out=generate_mock_code=true:"${OBSR_PROTOGEN_DIR}"
    --cpp_out "${OBSR_PROTOGEN_DIR}"
    -I "${OBSR_PROTO_DIR}"
    --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
    "${OBSR_PROTO_FILE}"
    DEPENDS "${OBSR_PROTO_FILE}")

include_directories(
    ${PROJECT_INCLUDE_DIR}
    ${OBSR_PROTOGEN_DIR}
)

link_libraries(
    spdlog::spdlog_header_only
    grpc++
)

add_executable(server example/stockserver.cpp)
add_executable(client example/stockclient.cpp)
target_link_libraries(server ${OBSR_LIB})
target_link_libraries(client ${OBSR_LIB})

# Target library.
add_library(
    ${OBSR_LIB}
    SHARED
    ${OBSR_LIB_SRC}
    ${OBSR_PB_SRC}
    ${OBSR_GRPC_SRC}
)

# Unit tests
if(${BUILD_UNITTEST})
    set(UNITTEST_EXE test${CMAKE_PROJECT_NAME})

    # Google test package on the system.
    find_package(GTest REQUIRED)

    add_executable(
        ${UNITTEST_EXE}
        ${OBSR_TEST_SRC}
    )

    target_link_libraries(
        ${UNITTEST_EXE}
        ${OBSR_LIB}
        GTest::gtest
        GTest::gmock
        pthread
    )

    # Allow ctest to discover unittests.
    enable_testing()
    include(GoogleTest)
    gtest_discover_tests(
        ${UNITTEST_EXE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

install(
    TARGETS
    server
    client
    DESTINATION bin
)

install(
    TARGETS ${OBSR_LIB}
    LIBRARY DESTINATION lib
)

install(
    FILES ${OBSR_HEADER}
    DESTINATION include/obsr
)