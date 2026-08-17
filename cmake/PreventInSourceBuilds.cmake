function(prevent_in_source_builds)
  get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
  get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

  if("${srcdir}" STREQUAL "${bindir}")
    message(FATAL_ERROR "
      [ERROR] In-source builds are forbidden.
      Please create a dedicated build directory:
        cmake -B build
        cmake --build build
    ")
  endif()
endfunction()

prevent_in_source_builds()