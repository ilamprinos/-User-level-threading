cmake_minimum_required(VERSION 3.21)
project(threads_scheduling C)
set(CMAKE_BUILD_TYPE Debug)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/atomics)
# set(CMAKE_C_COMPILER "gcc-9")

# project(threading_libt)
set(THREAD_HEADER


)
# add_compile_definitions(USEDEBUG)
add_compile_definitions(METRICS)
add_compile_definitions(HAND_TO_HAND)

add_compile_definitions(RECYCLE_THREADS)
#  add_compile_definitions(USE_LOCAL_QUEUES)

set(HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/atomics/atomic.h
${CMAKE_CURRENT_SOURCE_DIR}/src/threads.h
${CMAKE_CURRENT_SOURCE_DIR}/src/deque.h
${CMAKE_CURRENT_SOURCE_DIR}/src/sem.h
)
set(SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/src/threads.c
${CMAKE_CURRENT_SOURCE_DIR}/src/deque.c
${CMAKE_CURRENT_SOURCE_DIR}/src/sem.c
)
# set(TYPES ${CMAKE_CURRENT_SOURCE_DIR}/src/types.h )

# set(TEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_manos.c)
set(TEST_FILE_2 ${CMAKE_CURRENT_SOURCE_DIR}/tests/main.c)
set(TEST_FILE_3 ${CMAKE_CURRENT_SOURCE_DIR}/tests/matmul_multilevel.c)
set(TEST_FILE_4 ${CMAKE_CURRENT_SOURCE_DIR}/tests/matmul.c)
set(TEST_FILE_5 ${CMAKE_CURRENT_SOURCE_DIR}/tests/matmul_yield.c)



add_executable(test_a ${TEST_FILE_2} ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries(test_a -lpthread )
target_compile_options(test_a PRIVATE  -Wall -Wextra)

target_include_directories(test_a PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/src)


add_executable(test_b ${TEST_FILE_3} ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries(test_b -lpthread )
target_compile_options(test_b PRIVATE  -Wall -Wextra)
target_include_directories(test_b PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(test_c ${TEST_FILE_4} ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries(test_c -lpthread )
target_compile_options(test_c PRIVATE  -Wall -Wextra)

target_include_directories(test_c PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(test_d ${TEST_FILE_5} ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries(test_d -lpthread )
target_compile_options(test_d PRIVATE  -Wall -Wextra)

target_include_directories(test_d PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/src)





# add_executable(test ${TEST_FILE}  )
# add_dependencies(test ${PROJECT_NAME})
# target_link_libraries(test -lpthread lib)
# target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
