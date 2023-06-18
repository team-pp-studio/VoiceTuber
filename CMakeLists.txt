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

add_library(ser)
file(GLOB_RECURSE SER_SOURCE_FILES CONFIGURE_DEPENDS "3rd-party/ser/*.cpp")
target_sources(ser PRIVATE ${SER_SOURCE_FILES})

add_library(json)
file(GLOB JSON_SOURCE_FILES CONFIGURE_DEPENDS "3rd-party/json/*.cpp")
target_sources(json PRIVATE ${JSON_SOURCE_FILES})

add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON CXX_STANDARD 23)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/**")
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/3rd-party)
target_link_libraries(${PROJECT_NAME} PRIVATE ser json SDL2::SDL2 imgui::imgui SDL2_ttf::SDL2_ttf glm::glm stb::stb pocketsphinx::pocketsphinx cpptoml uv CURL::libcurl)

# TODO: use conanfile.py instead to get access to this directory in a more portable way
set(imgui_RES_DIR ${CMAKE_CURRENT_LIST_DIR}/full_deploy/host/imgui/cci.20230105+1.89.2.docking/${CMAKE_BUILD_TYPE}/x86_64/res/)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${imgui_RES_DIR}/bindings)
target_sources(${PROJECT_NAME} PRIVATE
    ${imgui_RES_DIR}/bindings/imgui_impl_opengl3.cpp
    ${imgui_RES_DIR}/bindings/imgui_impl_sdl.cpp)

# destinations are relative to the install prefix
install(TARGETS ${PROJECT_NAME} DESTINATION ".")
install(DIRECTORY assets DESTINATION ".")