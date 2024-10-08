cmake_minimum_required(VERSION 3.26)
project(VoiceTuber)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

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
find_package(scn REQUIRED CONFIG)
find_package(fmt REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(RapidJSON REQUIRED CONFIG)


add_library(warnings INTERFACE)
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(warnings INTERFACE -Werror=all -Werror=extra)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang*")
    target_compile_options(warnings INTERFACE -Werror=all -Werror=extra -Weverything)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    target_compile_options(warnings INTERFACE /W4)
endif()

add_library(sanitizers INTERFACE)
if ((${CMAKE_CXX_COMPILER_ID} MATCHES "GNU") OR (${CMAKE_CXX_COMPILER_ID} MATCHES ".*Clang"))
    target_link_libraries(sanitizers INTERFACE -fsanitize=address,undefined -static-libasan -static-libubsan)
    target_compile_options(sanitizers INTERFACE -fsanitize=address,undefined)
endif()

add_library(ser OBJECT)
file(GLOB_RECURSE SER_SOURCE_FILES CONFIGURE_DEPENDS "3rd-party/ser/*.cpp")
target_sources(ser PRIVATE ${SER_SOURCE_FILES})
set_target_properties(ser PROPERTIES CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)

add_library(imgui_bindings OBJECT)
target_include_directories(imgui_bindings PUBLIC ${imgui_RES_DIR}/bindings)
target_sources(imgui_bindings PRIVATE
    ${imgui_RES_DIR}/bindings/imgui_impl_opengl3.cpp
    ${imgui_RES_DIR}/bindings/imgui_impl_sdl2.cpp)
target_link_libraries(imgui_bindings PUBLIC imgui::imgui SDL2::SDL2)

add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON CXX_STANDARD 23)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/**")
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/3rd-party)
target_link_libraries(${PROJECT_NAME} PRIVATE warnings sanitizers ser imgui_bindings OpenGL::GL SDL2::SDL2 imgui::imgui SDL2_ttf::SDL2_ttf glm::glm stb::stb pocketsphinx::pocketsphinx cpptoml uv CURL::libcurl scn::scn fmt::fmt spdlog::spdlog rapidjson)

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)    
endif()

if ((${CMAKE_CXX_COMPILER_ID} MATCHES "GNU") OR (${CMAKE_CXX_COMPILER_ID} MATCHES ".*Clang"))
    target_link_libraries(sanitizers INTERFACE -static-libgcc -static-libstdc++)
    target_compile_options(sanitizers INTERFACE -static-libgcc -static-libstdc++)
endif()

set_target_properties(VoiceTuber PROPERTIES INSTALL_RPATH "\${ORIGIN}/lib")

# destinations are relative to the install prefix
install(TARGETS ${PROJECT_NAME} DESTINATION "." RUNTIME_DEPENDENCY_SET RuntimeDependencies)
install(RUNTIME_DEPENDENCY_SET RuntimeDependencies DESTINATION "lib" PRE_EXCLUDE_REGEXES "^/lib.*" "^/usr.*" POST_EXCLUDE_REGEXES "^/lib.*" "^/usr.*")
install(DIRECTORY assets DESTINATION ".")
