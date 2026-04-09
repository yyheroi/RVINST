cmake_minimum_required(VERSION 3.25...3.27)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

project(INST
    VERSION "0.0.1"
    HOMEPAGE_URL "https://github.com/Baiyi27/RVOS.git"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

include(CheckIPOSupported)
include(CodeCheck)

# check_ipo_supported()
check_ipo_supported(RESULT _ipo_supported)
if(_ipo_supported)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-flto=full)
  else()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  endif()
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

add_compile_options(
    -ffunction-sections
    -fdata-sections
    -funroll-loops
    -fomit-frame-pointer
    "$<$<CONFIG:Debug>:-ggdb3;-DDEBUG>"
    $<$<CONFIG:Release,MinSizeRel>:-DNDEBUG>
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_|amd|AMD)64$")
    add_compile_options(
        -march=native
        -mtune=native
        -mprefer-vector-width=256
    )
endif()

add_link_options(
    -Wl,--gc-sections
    -lm
    $<$<CONFIG:Release,MinSizeRel>:-s>
)

include_directories(
    inc
    Logger
)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Logger/CMakeLists.txt")
    message(FATAL_ERROR
        "Logger submodule is missing (no Logger/CMakeLists.txt).\n"
        "From the repository root run:\n"
        "  git submodule update --init --recursive\n"
        "or clone with: git clone --recurse-submodules <url>")
endif()

add_subdirectory(Logger)
set(backend ${PROJECT_NAME}_backend)
add_library(${backend} SHARED)
add_executable(
    ${PROJECT_NAME}
    tests/INST.cc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".exe"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${backend}
    $<$<CONFIG:Debug>:Logger>
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resource)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/InstStyle.css
    ${CMAKE_CURRENT_BINARY_DIR}/resource/InstStyle.css
    COPYONLY
)

set(_inst_gui_common_src
    src/Gui/AsmMnemonicWidget.cc
    src/Gui/BinaryFieldWidget.cc
    src/Gui/InstFormatUI.cc
    src/Gui/RISCVInstructionWindow.cc
)
set(_inst_gui_common_hdr
    inc/Gui/InsEntryParse.hh
    inc/Gui/InstructionSummaryText.hh
    inc/Gui/InstCommST.hh
    inc/Gui/AsmMnemonicWidget.hh
    inc/Gui/BinaryFieldWidget.hh
    inc/Gui/InstFormatUI.hh
    inc/Gui/RISCVInstructionWindow.hh
)

add_library(${PROJECT_NAME}_gui_common OBJECT ${_inst_gui_common_src} ${_inst_gui_common_hdr})
target_include_directories(${PROJECT_NAME}_gui_common SYSTEM PRIVATE ${GTKMM_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME}_gui_common PRIVATE
    CSS_FILE_PATH="${CMAKE_CURRENT_BINARY_DIR}/resource/InstStyle.css"
)

add_executable(${PROJECT_NAME}_gui
    src/Gui/Main.cc
    src/Gui/RISCVApplication.cc
    $<TARGET_OBJECTS:${PROJECT_NAME}_gui_common>
)
target_sources(${PROJECT_NAME}_gui PRIVATE inc/Gui/RISCVApplication.hh)

add_executable(${PROJECT_NAME}_gui_rv32i_demo
    src/Gui/Rv32iDemoMain.cc
    $<TARGET_OBJECTS:${PROJECT_NAME}_gui_common>
)
target_sources(${PROJECT_NAME}_gui_rv32i_demo PRIVATE inc/Gui/Rv32iInstructionCases.hh)

foreach(_t ${PROJECT_NAME}_gui ${PROJECT_NAME}_gui_rv32i_demo)
    target_link_libraries(${_t} PRIVATE ${backend} ${GTKMM_LIBRARIES})
    target_include_directories(${_t} SYSTEM PRIVATE ${GTKMM_INCLUDE_DIRS})
    set_target_properties(${_t} PROPERTIES SUFFIX ".exe")
endforeach()

set_target_properties(${backend} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}

    # OUTPUT_NAME "backEnd"
)

target_sources(${backend} PRIVATE
    inc/ISA/Regs.hpp
    src/ISA/Regs.cc

    inc/Util/BiLookupTable.hpp

    inc/Core/IBaseInstType.hh
    src/Core/IBaseInstType.cc

    inc/ISA/InstFormat.hh
    src/ISA/InstFormat.cc

    inc/Core/RType.hh
    src/Core/RType.cc

    inc/Core/IType.hh
    src/Core/IType.cc

    inc/Core/JType.hh
    src/Core/JType.cc

    inc/Core/UType.hh
    src/Core/UType.cc

    inc/Core/SType.hh
    src/Core/SType.cc

    inc/Core/BType.hh
    src/Core/BType.cc

    inc/Core/InstTypeFactory.hh
    src/Core/InstTypeFactory.cc

    inc/Core/Instruction.hh
    src/Core/Instruction.cc

)

SetupCodeChecks(${PROJECT_NAME} ${backend} ${PROJECT_NAME}_gui_common ${PROJECT_NAME}_gui ${PROJECT_NAME}_gui_rv32i_demo)

DefineCodeChecksTargets("fmt" "tidy")
get_filename_component(builddir ${CMAKE_CURRENT_BINARY_DIR} NAME)
if(NOT DEFINED rootBuilddir OR rootBuilddir STREQUAL "")
    set(rootBuilddir "${CMAKE_CURRENT_BINARY_DIR}")
endif()
get_filename_component(root ${rootBuilddir} NAME)

if(ClangFormat_FOUND OR ClangTidy_FOUND)
    message(">> Optional: build the target 'tidy' or 'fmt' ('cmake --build ${root}/${builddir} -t help' to know more) before building 'all' target")
endif()
