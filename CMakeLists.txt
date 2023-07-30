cmake_minimum_required(VERSION 3.26)
project(VoiceTuber)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED CONFIG)
find_package(imgui REQUIRED CONFIG)
find_package(SDL2_ttf REQUIRED CONFIG)
find_package(glm REQUIRED CONFIG)
find_package(stb REQUIRED CONFIG)
find_package(pocketsphinx REQUIRED CONFIG)
find_package(cpptoml REQUIRED CONFIG)
find_package(libuv REQUIRED CONFIG)
find_package(CURL REQUIRED CONFIG)
find_package(OpenGL REQUIRED MODULE)


add_library(warnings INTERFACE)
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(warnings INTERFACE -Werror=all -Werror=extra)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang*")
    target_compile_options(warnings INTERFACE -Werror=all -Werror=extra -Weverything)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    target_compile_options(warnings INTERFACE /W4)
endif()
add_library(sanitizers INTERFACE)
target_link_libraries(sanitizers INTERFACE -fsanitize=address -fsanitize=undefined)
target_compile_options(sanitizers INTERFACE -fsanitize=address -fsanitize=undefined)

add_library(ser)
file(GLOB_RECURSE SER_SOURCE_FILES CONFIGURE_DEPENDS "3rd-party/ser/*.cpp")
target_sources(ser PRIVATE ${SER_SOURCE_FILES})
set_target_properties(ser PROPERTIES CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)

add_library(json)
file(GLOB JSON_SOURCE_FILES CONFIGURE_DEPENDS "3rd-party/json/*.cpp")
target_sources(json PRIVATE ${JSON_SOURCE_FILES})
set_target_properties(json PROPERTIES CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)

add_library(imgui_bindings)
target_include_directories(imgui_bindings PUBLIC ${imgui_RES_DIR}/bindings)
target_sources(imgui_bindings PRIVATE
    ${imgui_RES_DIR}/bindings/imgui_impl_opengl3.cpp
    ${imgui_RES_DIR}/bindings/imgui_impl_sdl.cpp)
target_link_libraries(imgui_bindings PUBLIC imgui::imgui SDL2::SDL2)

add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON CXX_STANDARD 23)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/**")
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/3rd-party)
target_link_libraries(${PROJECT_NAME} PRIVATE warnings sanitizers ser json imgui_bindings OpenGL::GL SDL2::SDL2 imgui::imgui SDL2_ttf::SDL2_ttf glm::glm stb::stb pocketsphinx::pocketsphinx cpptoml uv CURL::libcurl)

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)    
endif()


# destinations are relative to the install prefix
install(TARGETS ${PROJECT_NAME} DESTINATION ".")
install(DIRECTORY assets DESTINATION ".")