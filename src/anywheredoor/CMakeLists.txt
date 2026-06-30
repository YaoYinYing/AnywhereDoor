cmake_minimum_required(VERSION 3.16)
project(anywhere_door VERSION 2.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# --- Find dependencies ---

find_package(CURL REQUIRED)
find_package(Curses REQUIRED)

# libyaml doesn't ship a cmake config — find it via pkg-config or manual paths
find_library(YAML_LIBRARY yaml)
if(NOT YAML_LIBRARY)
    # Homebrew on Apple Silicon
    find_library(YAML_LIBRARY yaml PATHS /opt/homebrew/lib /usr/local/lib)
endif()

find_path(YAML_INCLUDE_DIR yaml.h)
if(NOT YAML_INCLUDE_DIR)
    find_path(YAML_INCLUDE_DIR yaml.h PATHS /opt/homebrew/include /usr/local/include)
endif()

if(NOT YAML_LIBRARY OR NOT YAML_INCLUDE_DIR)
    message(FATAL_ERROR "libyaml not found. Install with: brew install libyaml (macOS) or apt install libyaml-dev (Linux)")
endif()

message(STATUS "YAML library: ${YAML_LIBRARY}")
message(STATUS "YAML include: ${YAML_INCLUDE_DIR}")

# --- Source files ---

set(SOURCES
    main.c
    utils.c
    proxy.c
    config.c
    test.c
    dnsleak.c
    tui.c
)

# --- Executable ---

add_executable(anywhere_door ${SOURCES})

target_include_directories(anywhere_door PRIVATE
    ${YAML_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    ${CURSES_INCLUDE_DIRS}
)

target_link_libraries(anywhere_door PRIVATE
    ${YAML_LIBRARY}
    CURL::libcurl
    ${CURSES_LIBRARIES}
)

# --- Version injection ---
if(VERSION)
    target_compile_definitions(anywhere_door PRIVATE VERSION="${VERSION}")
endif()

# --- Platform-specific ---

if(APPLE)
    # Find Homebrew ncurses (wider character support)
    find_library(NCURSESW_LIBRARY ncursesw PATHS /opt/homebrew/lib /usr/local/lib)
    if(NCURSESW_LIBRARY)
        target_link_libraries(anywhere_door PRIVATE ${NCURSESW_LIBRARY})
    endif()
endif()

# --- Install ---

install(TARGETS anywhere_door DESTINATION bin)
