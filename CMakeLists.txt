cmake_minimum_required(VERSION 2.6)
PROJECT(freenetconfd-plugins)
ADD_DEFINITIONS(-Os -Wall --std=gnu11 -Wmissing-declarations -D_GNU_SOURCE)
INCLUDE_DIRECTORIES(include)

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/freenetconfd)

ADD_LIBRARY(example_black_book MODULE black-book.c)
SET_TARGET_PROPERTIES(example_black_book PROPERTIES OUTPUT_NAME black-book PREFIX "")

ADD_LIBRARY(example_house_lockdown MODULE house-lockdown.c)
SET_TARGET_PROPERTIES(example_house_lockdown PROPERTIES OUTPUT_NAME house-lockdown PREFIX "")

ADD_LIBRARY(example_sandwich MODULE sandwich.c)
SET_TARGET_PROPERTIES(example_sandwich PROPERTIES OUTPUT_NAME sandwich PREFIX "")

ADD_LIBRARY(example_shopping_list MODULE shopping-list.c)
SET_TARGET_PROPERTIES(example_shopping_list PROPERTIES OUTPUT_NAME shopping-list PREFIX "")

INSTALL(
	TARGETS 
		example_black_book
		example_house_lockdown
		example_sandwich
		example_shopping_list
	LIBRARY DESTINATION lib/
)
