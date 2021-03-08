# CMakeLists.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.18)
project ("MTMultiplex")

# Include sub-projects.
add_subdirectory ("MTMultiplex")
add_subdirectory ("MTMultiplex-Test")
