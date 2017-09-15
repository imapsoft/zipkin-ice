cmake_minimum_required(VERSION 3.9)
project(zipkin-ice
        VERSION 0.0.1
        DESCRIPTION "zipkin instrumented library for ice"
        LANGUAGES CXX)

find_path(CPPREST_INCLUDE cpprest/http_client.h)
find_library(CPPREST_LIB NAMES cpprest_2_9d cpprest_2_9 cpprestd cpprest)
find_package(Boost REQUIRED COMPONENTS random system thread filesystem chrono atomic date_time regex locale)
find_package(OpenSSL 1.0.0 REQUIRED)

add_compile_options(-DZIPKIN_ICE_EXPORT)
add_library(zipkin_ice SHARED zipkin_ice.cpp)
target_include_directories(zipkin_ice PUBLIC ${CPPREST_INCLUDE})
target_link_libraries(zipkin_ice
  ${CPPREST_LIB}
  Boost::boost
  Boost::random
  Boost::system
  Boost::thread
  Boost::filesystem
  Boost::chrono
  Boost::atomic
  Boost::date_time
  Boost::regex
  Boost::locale
  OpenSSL::SSL
)

add_executable(zipkin_ice_test test.cpp)
target_include_directories(zipkin_ice_test PUBLIC ${CPPREST_INCLUDE})
target_link_libraries(zipkin_ice_test zipkin_ice)
target_link_libraries(zipkin_ice_test
  ${CPPREST_LIB}
  Boost::boost
  Boost::random
  Boost::system
  Boost::thread
  Boost::filesystem
  Boost::chrono
  Boost::atomic
  Boost::date_time
  Boost::regex
  Boost::locale
  OpenSSL::SSL
)
