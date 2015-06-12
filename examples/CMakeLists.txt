CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(freenetconfd-plugins)

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib/freenetconfd)

ADD_LIBRARY(example_black_book MODULE black-book.c)
SET_TARGET_PROPERTIES(example_black_book PROPERTIES OUTPUT_NAME black-book PREFIX "")

ADD_LIBRARY(example_filer MODULE filer.c)
SET_TARGET_PROPERTIES(example_filer PROPERTIES OUTPUT_NAME filer PREFIX "")

ADD_LIBRARY(example_house_lockdown MODULE house-lockdown.c)
SET_TARGET_PROPERTIES(example_house_lockdown PROPERTIES OUTPUT_NAME house-lockdown PREFIX "")

ADD_LIBRARY(example_sandwich MODULE sandwich.c)
SET_TARGET_PROPERTIES(example_sandwich PROPERTIES OUTPUT_NAME sandwich PREFIX "")

ADD_LIBRARY(example_shopping_list MODULE shopping-list.c)
SET_TARGET_PROPERTIES(example_shopping_list PROPERTIES OUTPUT_NAME shopping-list PREFIX "")

ADD_LIBRARY(example_vehicle MODULE vehicle.c)
SET_TARGET_PROPERTIES(example_vehicle PROPERTIES OUTPUT_NAME vehicle PREFIX "")

ADD_LIBRARY(example_choice MODULE choice.c)
SET_TARGET_PROPERTIES(example_choice PROPERTIES OUTPUT_NAME choice PREFIX "")

ADD_LIBRARY(example_hello MODULE hello.c)
SET_TARGET_PROPERTIES(example_hello PROPERTIES OUTPUT_NAME hello PREFIX "")

INSTALL(
	TARGETS
		example_black_book
		example_filer
		example_house_lockdown
		example_sandwich
		example_shopping_list
		example_vehicle
		example_choice
		example_hello
	LIBRARY DESTINATION lib/freenetconfd/
)

INSTALL(
	FILES
		yang/black-book@2014-11-28.yang
		yang/filer@2014-11-28.yang
		yang/house-lockdown@2014-11-28.yang
		yang/sandwich@2014-11-28.yang
		yang/shopping-list@2014-11-28.yang
		yang/vehicle@2015-01-02.yang
		yang/choice@2015-02-10.yang
		yang/hello@2015-02-10.yang
	DESTINATION etc/yang/
)
