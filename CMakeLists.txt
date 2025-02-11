cmake_minimum_required(VERSION 3.10)
project(Eventing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(Eventing src/EventingTest.cpp)
target_include_directories(Eventing PRIVATE "../include")
set(SOURCES "src/EventingTest.cpp")


# Enable AddressSanitizer and Undefined Behavior Sanitizer only in Debug and RelWithDebInfo modes on non-Windows platforms
if(NOT WIN32)
    if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
        set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address,undefined")
    endif()
else()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
        message("Detected Clang on MSVC")
        set(CLANG_ON_MSVC TRUE)
     endif()
    
     set(CMAKE_SUPPRESS_REGENERATION true)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Eventing)
    target_compile_options(Eventing PRIVATE /W4)
    message("Have set startup project in MSVC to Eventing")


    if (CLANG_ON_MSVC)
        message("Address Sanitizer doesn't work with Clang on MSVC")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT")
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
       
    else()
        message("Address Sanitizer enabled in MSVC")
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fsanitize=address")
    endif()
endif()



# Add a new target for StackFnTest.cpp
# add_executable(StackFnTest src/StackFnTest.cpp)

include(CTest)
enable_testing()