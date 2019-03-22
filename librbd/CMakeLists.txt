cmake_minimum_required (VERSION 2.8.11)
project (rbdTest)
# The version number.
set (RbdTest_VERSION_MAJOR 1)
set (RbdTest_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
#configure_file (
#	"${PROJECT_SOURCE_DIR}/TutorialConfig.h.in"
#	"${PROJECT_BINARY_DIR}/TutorialConfig.h"
#	)

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}/include")

#target_link_libraries(LibsModule -lpthread)

# add the executable
add_executable(rbdTest rbdTest.cc)

target_link_libraries(rbdTest -lrbd -lrados)
