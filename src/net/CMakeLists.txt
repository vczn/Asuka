CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(net_srcs
	acceptor.cpp
	buffer.cpp
	channel.cpp
	connector.cpp
	default_poller.cpp
	epoller.cpp
	event_loop.cpp
	event_loop_thread.cpp
	event_loop_thread_pool.cpp
	ip_port.cpp
	poller.cpp
	poller_base.cpp
	socket.cpp
	tcp_client.cpp
	tcp_connection.cpp
	tcp_server.cpp
	timer.cpp
	timer_queue.cpp
)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_LIBRARY(asuka_net ${net_srcs})
TARGET_LINK_LIBRARIES(asuka_net asuka_util)

INSTALL(TARGETS asuka_net DESTINATION lib)
FILE(GLOB headers "*.hpp")
INSTALL (FILES ${headers} DESTINATION include/net)
