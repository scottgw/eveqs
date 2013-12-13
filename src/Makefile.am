lib_LTLIBRARIES = libeveqs.la

libeveqs_la_SOURCES = \
	eif_utils.hpp \
	eveqs.cpp \
	global.cpp \
	global.hpp \
	internal.hpp \
	private_queue.cpp \
	private_queue.hpp \
	processor.cpp \
	processor.hpp \
	qoq.hpp \
	req_grp.cpp \
	req_grp.hpp \
	spsc.hpp

BUILT_SOURCES = config.h

include_HEADERS = eveqs.h

FLAGS = -pthread \
	-DEIF_THREADS -DEIF_POSIX_THREADS \
	-I${HOME}/local/include \
	-I${ISE_EIFFEL}/studio/spec/${ISE_PLATFORM}/include \
	-Wall -Wno-unused-value -Wno-unused-variable -Werror -march=native
libeveqs_la_CFLAGS = -std=gnu99 $(FLAGS) $(DEPS_CFLAGS)
libeveqs_la_CXXFLAGS = -std=c++0x $(FLAGS) $(DEPS_CFLAGS)
libeveqs_la_LIBADD =
