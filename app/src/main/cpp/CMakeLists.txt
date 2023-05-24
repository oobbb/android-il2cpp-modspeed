cmake_minimum_required(VERSION 3.18.1)

project(Yuri)

set(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)


message("Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 20)

set(LINKER_FLAGS "-ffixed-x18 -Wl,--hash-style=both -w -s -Wno-error=format-security -fvisibility=hidden -fpermissive -fexceptions -Wl,-Bsymbolic")

set(C_FLAGS "-Werror=format -fdata-sections -ffunction-sections")

set(CXX_FLAGS "${CXX_FLAGS} -fno-exceptions -fno-rtti -w -s -Wno-error=format-security -fvisibility=hidden -Werror -Wno-error=c++11-narrowing -fpermissive -Wall -fexceptions")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(C_FLAGS "${C_FLAGS} -O2 -fvisibility=hidden -fvisibility-inlines-hidden")
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all")
else ()
    set(C_FLAGS "${C_FLAGS} -O0")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS} ${CXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}")

include_directories(${SOURCE_PATH})
include_directories(include)
include_directories(ImGui)
include_directories(ImGui/backends)



aux_source_directory(xDL xdl-src)
aux_source_directory(ImGui ImGui-src)
aux_source_directory(ImGui/backends backends-src)


add_library(${PROJECT_NAME} SHARED main.cpp
        ${xdl-src}
        ${ImGui-src}
        ${backends-src}
         )


set(DOBBY_LIB ${CMAKE_CURRENT_SOURCE_DIR}/Dobby/${CMAKE_ANDROID_ARCH_ABI}/libdobby.a)
target_link_libraries(${PROJECT_NAME} log EGL GLESv3 android ${DOBBY_LIB})




if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all --remove-section=.comment "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${PROJECT_NAME}.so")
endif ()
