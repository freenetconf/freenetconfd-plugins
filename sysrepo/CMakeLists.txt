CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(freenetconfd-plugins)

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib/freenetconfd)

ADD_LIBRARY(sysrepo_ifstat MODULE ifstat.c)
SET_TARGET_PROPERTIES(sysrepo_ifstat PROPERTIES OUTPUT_NAME sysrepo-ifstat PREFIX "")


TARGET_LINK_LIBRARIES(sysrepo_ifstat roxml)

INSTALL(
	TARGETS
		sysrepo_ifstat
	LIBRARY DESTINATION lib/freenetconfd
)

INSTALL(
	FILES
		yang/ifstat@2015-03-11.yang
	DESTINATION etc/yang/
)
