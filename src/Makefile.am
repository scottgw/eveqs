lib_LTLIBRARIES = libeveqs.la

libeveqs_la_SOURCES = \
	eveqs.cpp \
	global.cpp \
	global.hpp \
	internal.hpp \
	private_queue.cpp \
	private_queue.hpp \
	processor.cpp \
	processor.hpp \
	qoq.cpp \
	req_grp.cpp \
	req_grp.hpp

BUILT_SOURCES = config.h

# we use pkginclude because it'll put these in a separate directory
# undef the $(includedir).
pkginclude_HEADERS = eveqs.h

FLAGS = -pthread -U_FORTIFY_SOURCE -DENABLE_LOG \
	-I${ISE_EIFFEL}/studio/spec/${ISE_PLATFORM}/include
	-Wall -Wno-unused-value -Wno-unused-variable -Werror -march=native
libeveqs_la_CFLAGS = -std=gnu99 $(FLAGS) $(DEPS_CFLAGS)
libeveqs_la_CXXFLAGS = -std=c++0x $(FLAGS) $(DEPS_CFLAGS)
libeveqs_la_LIBADD =
