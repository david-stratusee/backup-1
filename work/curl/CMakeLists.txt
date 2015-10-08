cmake_minimum_required(VERSION 2.8)
project(multi_test)

include(${PROJECT_SOURCE_DIR}/CMakeLists_common.txt)

set(source_list 
	${CMAKE_CURRENT_SOURCE_DIR}/data_struct.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/string_s.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/timestamp.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/ssl_lock.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/cfg_parse.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/cfg_parse.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/setsignal.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/setsignal.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/atomic_def.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/str_def.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/optimize.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/cplus_define.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/misc.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/umemory.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/string_s.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/file_op.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/misc.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/types.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/file_op.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/lock_def.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/ssl_lock.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/timestamp.h
	${CMAKE_CURRENT_SOURCE_DIR}/multi_test.c
	${CMAKE_CURRENT_SOURCE_DIR}/util.c
	${CMAKE_CURRENT_SOURCE_DIR}/util.h
)

include_directories(${PROJECT_SOURCE_DIR}/common)

add_executable(multi_test ${source_list})
target_link_libraries(multi_test pthread)
target_link_libraries(multi_test curl)
target_link_libraries(multi_test crypto)
target_link_libraries(multi_test jemalloc)
#target_link_libraries(multi_test c_p)
