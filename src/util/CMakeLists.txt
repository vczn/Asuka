CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(util_srcs
	config.cpp
	log_stream.cpp
	logger.cpp
	time_stamp.cpp
	util.cpp
)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_LIBRARY(asuka_util ${util_srcs})
FIND_LIBRARY(pthreadlib pthread)
TARGET_LINK_LIBRARIES(asuka_util ${pthreadlib})

INSTALL(TARGETS asuka_util DESTINATION lib)

FILE(GLOB headers "*.hpp")
INSTALL(FILES ${headers} DESTINATION include/util)
