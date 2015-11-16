PDIR=pbrane

LSRC=
LSRC+=${PDIR}/expression.cpp ${PDIR}/parser.cpp ${PDIR}/permission.cpp
LSRC+=${PDIR}/pvm.cpp ${PDIR}/regex.cpp ${PDIR}/variable.cpp
LSRC+=${PDIR}/varstore.cpp ${PDIR}/modules.cpp

CXXFLAGS=-std=c++14
# LDFLAGS=-lboost_regex # linked by linkee?
# -lgmp -lgmpxx

default: libpbrane.so

libpbrane.so: ${LSRC} ${PDIR}/modules/math.cpp
	${CXX} ${CXXFLAGS} -shared -fPIC $^ -o $@

# see PINKSERV2 project for example of using library

clean:
	rm -rf libpbrane.so

