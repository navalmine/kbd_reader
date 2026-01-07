add_executable(test_scancode test_scancode.c)
target_link_libraries(test_scancode PRIVATE kbdcore)
add_test(NAME test_scancode COMMAND test_scancode)

add_executable(test_stats test_stats.c)
target_link_libraries(test_stats PRIVATE kbdcore)
add_test(NAME test_stats COMMAND test_stats)
