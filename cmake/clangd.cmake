# cmake/clangd.cmake
# Presets build into build/<preset>/, which clangd can't auto-discover (it only searches the
# file's dir, ./build, and ancestors). Mirror the active preset's compile_commands.json to the
# project root so the checked-in .clangd (CompilationDatabase: .) always reflects the last build.
# The Visual Studio and Xcode generators don't emit compile_commands.json — only Ninja/Makefiles —
# so only mirror for generators that produce it.
if(CMAKE_EXPORT_COMPILE_COMMANDS AND NOT CMAKE_GENERATOR MATCHES "Visual Studio|Xcode")
  add_custom_target(
    mirror_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/compile_commands.json"
    BYPRODUCTS "${CMAKE_SOURCE_DIR}/compile_commands.json"
    COMMENT "Mirroring compile_commands.json to project root for clangd"
    VERBATIM)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
  add_custom_target(
    fix_clangd_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
      "${CMAKE_SOURCE_DIR}/compile_commands.json"
      "${CMAKE_SOURCE_DIR}/compile_commands.json.bak"
    COMMAND powershell -Command 
      "(Get-Content '${CMAKE_SOURCE_DIR}/compile_commands.json' -Raw) -replace '/imsvc', '-I' | Out-File -Encoding ASCII '${CMAKE_SOURCE_DIR}/compile_commands.json'"
    DEPENDS mirror_compile_commands
    COMMENT "Converting clang-cl compile commands for clangd compatibility"
    VERBATIM)
endif()