add_library(kbdcore
  scancode_map.c
  stats.c
)

target_include_directories(kbdcore PUBLIC ${CMAKE_CURRENT_LIST_DIR})
