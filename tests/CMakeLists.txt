set (compose_tests_srcs
    compose/stable_operation.cpp
    compose/unstable_operation.cpp
    compose/inplace_forward.cpp
    compose/lean_tuple.cpp)

add_library(prebuilt-asio prebuilt_asio.cpp)
target_compile_definitions(prebuilt-asio PUBLIC
    BOOST_ASIO_SEPARATE_COMPILATION
)
target_link_libraries(prebuilt-asio PUBLIC Boost::system)

function (compose_add_test test_file)
    get_filename_component(target_name ${test_file} NAME_WE)
    add_executable(${target_name} ${test_file})
    target_link_libraries(${target_name} core prebuilt-asio)
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -pedantic -std=c++14)
    target_compile_definitions(${target_name} PRIVATE)

    add_test(NAME "${target_name}_tests"
             COMMAND ${target_name})
endfunction(compose_add_test)

foreach(test_src_name IN ITEMS ${compose_tests_srcs})
    compose_add_test(${test_src_name})
endforeach()
