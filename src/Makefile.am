lib_LTLIBRARIES = libeveqs.la

libeveqs_la_SOURCES = \
	eif_utils.cpp \
	eif_utils.hpp \
	eveqs.cpp \
	processor_registry.cpp \
	processor_registry.hpp \
	internal.hpp \
	notify_token.hpp \
	notify_token.cpp \
	private_queue.cpp \
	private_queue.hpp \
	processor.cpp \
	processor.hpp \
	mpmc.hpp \
	qoq.hpp \
	queue_cache.hpp \
	queue_cache.cpp \
	req_grp.cpp \
	req_grp.hpp \
	spsc.hpp

BUILT_SOURCES = config.h

include_HEADERS = eveqs.h

FLAGS = -pthread \
	-DSCOOPQS -DEIF_THREADS -DEIF_POSIX_THREADS \
	-I${HOME}/local/include \
	-I${ISE_EIFFEL}/studio/spec/${ISE_PLATFORM}/include \
	-Wall -Wno-unused-value -Wno-unused-variable -march=native
libeveqs_la_CFLAGS = -std=gnu99 $(FLAGS) $(DEPS_CFLAGS)
libeveqs_la_CXXFLAGS = -std=c++0x $(FLAGS) $(DEPS_CFLAGS)
libeveqs_la_LIBADD =
