set(INCLUDE_DIR "${voxel-io_SOURCE_DIR}/include")
set(TEST_INCLUDE_DIR "${INCLUDE_DIR}/voxelio/test")
set(ASSET_DIR "${voxel-io_SOURCE_DIR}/test/assets")

add_executable(voxelio-test
               main.cpp
               test.cpp
               io.cpp
               random.cpp
               voxels.cpp

               test_bits.cpp
               test_color.cpp
               test_stringmanip.cpp
               test_stream.cpp
               test_hextree.cpp
               test_palette.cpp
               test_deflate.cpp

               test_binvox.cpp
               test_qubicle.cpp
               test_vl32.cpp
               test_vox.cpp

               "${TEST_INCLUDE_DIR}/test.hpp"
               "${TEST_INCLUDE_DIR}/io.hpp"
               "${TEST_INCLUDE_DIR}/random.hpp"
               "${TEST_INCLUDE_DIR}/voxels.hpp")

target_compile_definitions(voxelio-test PUBLIC VXIO_TEST_ASSET_PATH=${ASSET_DIR})
target_include_directories(voxelio-test PUBLIC "${INCLUDE_DIR}")

target_link_libraries(voxelio-test PRIVATE voxelio)
