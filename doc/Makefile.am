SRC = private_queue.hpp \
	processor.hpp \
	queue_cache.hpp \
	req_grp.hpp \
	spsc.hpp

FULL_SRC = $(addprefix $(top_srcdir)/src/, ${SRC})
CXXFLAGS = -I${HOME}/local/Eiffel_14.05/studio/spec/linux-x86-64/include \
	-std=c++11

docdir = $(datadir)/doc/@PACKAGE@
dist_pkgdata_DATA: index.html

index.html: ${FULL_SRC}
	cldoc generate ${CXXFLAGS} -- --basedir ../ --output ./ ${FULL_SRC}

all-local: index.html

