cmake_minimum_required(VERSION 4.0)
project(AsteroidsSolare LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum policy version for all subprojects")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_library(common INTERFACE)
target_compile_features(common INTERFACE cxx_std_17)
target_compile_options(common INTERFACE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wall>
  $<$<CXX_COMPILER_ID:MSVC>: /W4>
)

# Tappa 01: finestra e Sole
add_executable(Tappa01 Tappa01/main.cpp)
target_link_libraries(Tappa01 PRIVATE SFML::Graphics common)

# Tappa 02: navicella che ruota
add_executable(Tappa02 Tappa02/main.cpp)
target_link_libraries(Tappa02 PRIVATE SFML::Graphics common)

# Tappa 03: spinta e inerzia
add_executable(Tappa03 Tappa03/main.cpp)
target_link_libraries(Tappa03 PRIVATE SFML::Graphics common)

# Tappa 04: gravita' e primo pianeta
add_executable(Tappa04 Tappa04/main.cpp)
target_link_libraries(Tappa04 PRIVATE SFML::Graphics common)

# Tappa 05: piu' pianeti e gravita' sulla navicella
add_executable(Tappa05 Tappa05/main.cpp)
target_link_libraries(Tappa05 PRIVATE SFML::Graphics common)

# Tappa 06: camera che segue la navicella e zoom
add_executable(Tappa06 Tappa06/main.cpp)
target_link_libraries(Tappa06 PRIVATE SFML::Graphics common)

# Tappa 07: proiettili
add_executable(Tappa07 Tappa07/main.cpp)
target_link_libraries(Tappa07 PRIVATE SFML::Graphics common)

# Tappa 08: asteroidi e collisioni
add_executable(Tappa08 Tappa08/main.cpp)
target_link_libraries(Tappa08 PRIVATE SFML::Graphics common)

# Tappa 09: frammenti, punteggio, vite e testo
add_executable(Tappa09 Tappa09/main.cpp)
target_link_libraries(Tappa09 PRIVATE SFML::Graphics common)