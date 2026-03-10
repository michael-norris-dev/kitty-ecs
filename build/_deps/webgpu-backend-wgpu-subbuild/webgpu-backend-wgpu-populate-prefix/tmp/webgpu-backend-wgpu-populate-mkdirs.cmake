# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/manor/Documents/Software_Projects/graphics_engine/build/_deps/webgpu-backend-wgpu-src")
  file(MAKE_DIRECTORY "C:/Users/manor/Documents/Software_Projects/graphics_engine/build/_deps/webgpu-backend-wgpu-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/manor/Documents/Software_Projects/graphics_engine/build/_deps/webgpu-backend-wgpu-build"
  "C:/Users/manor/Documents/Software_Projects/graphics_engine/build/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix"
  "C:/Users/manor/Documents/Software_Projects/graphics_engine/build/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/tmp"
  "C:/Users/manor/Documents/Software_Projects/graphics_engine/build/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/src/webgpu-backend-wgpu-populate-stamp"
  "C:/Users/manor/Documents/Software_Projects/graphics_engine/build/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/src"
  "C:/Users/manor/Documents/Software_Projects/graphics_engine/build/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/src/webgpu-backend-wgpu-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/manor/Documents/Software_Projects/graphics_engine/build/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/src/webgpu-backend-wgpu-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/manor/Documents/Software_Projects/graphics_engine/build/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/src/webgpu-backend-wgpu-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
