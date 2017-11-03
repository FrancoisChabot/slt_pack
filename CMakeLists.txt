cmake_minimum_required(VERSION 3.1)

project(slt_pack)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14 /W4 /wd4127")
  set(gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE)
  
  #Gtest workaround for MSVC2017
  add_definitions(-DGTEST_LANG_CXX11)
else()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -std=c++14 -ftemplate-backtrace-limit=0")
endif(MSVC)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
enable_testing()

add_subdirectory(third_party/googletest)
set_target_properties(gmock PROPERTIES FOLDER "gtest")
set_target_properties(gmock_main PROPERTIES FOLDER "gtest")
set_target_properties(gtest PROPERTIES FOLDER "gtest")
set_target_properties(gtest_main PROPERTIES FOLDER "gtest")

include_directories(
  ./include
  ${gtest_SOURCE_DIR}/include
)

macro(ADD_PACK_TEST TST_NAME)
  add_executable(${TST_NAME} tests/${TST_NAME}.cpp)
  target_link_libraries(${TST_NAME} gtest_main)
  add_test(${TST_NAME} ${TST_NAME})
endmacro(ADD_PACK_TEST)


ADD_PACK_TEST(simple_int_pack)
