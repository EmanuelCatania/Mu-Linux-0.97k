if(NOT MSVC)
  message(FATAL_ERROR "The client CMake build requires the Microsoft C++ compiler.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR
    "The client is Win32-only. Configure from an x86 MSVC environment created by vcvarsall.bat.")
endif()

enable_language(RC)

# CMake's Windows defaults define _WINDOWS for every target. The legacy console
# encoder does not use that macro, so targets opt in explicitly where required.
string(REPLACE "/D_WINDOWS" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

function(mu_configure_client_output target)
  set(output_directory "${CMAKE_BINARY_DIR}/bin/${target}")
  set_target_properties(${target} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${output_directory}"
    COMPILE_PDB_OUTPUT_DIRECTORY "${output_directory}"
    LIBRARY_OUTPUT_DIRECTORY "${output_directory}"
    PDB_OUTPUT_DIRECTORY "${output_directory}"
    RUNTIME_OUTPUT_DIRECTORY "${output_directory}")
endfunction()

add_subdirectory(Main)
add_subdirectory(InfoEncoder)
