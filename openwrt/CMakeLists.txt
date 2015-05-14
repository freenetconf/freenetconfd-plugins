CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(freenetconfd-plugins)

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build)

#SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/modules")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib/freenetconfd)

ADD_LIBRARY(openwrt_ietf_system MODULE ietf-system.c)
SET_TARGET_PROPERTIES(openwrt_ietf_system PROPERTIES OUTPUT_NAME ietf-system PREFIX "")

INSTALL(
	TARGETS
		openwrt_ietf_system
	LIBRARY DESTINATION lib/freenetconfd/
)

INSTALL(
	FILES
		yang/ietf-system@2014-08-06.yang
	DESTINATION etc/yang/
)
