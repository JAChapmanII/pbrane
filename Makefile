PDIR=pbrane

LSRC=
LSRC+=${PDIR}/expression.cpp ${PDIR}/parser.cpp ${PDIR}/permission.cpp
LSRC+=${PDIR}/pvm.cpp ${PDIR}/regex.cpp ${PDIR}/variable.cpp
LSRC+=${PDIR}/varstore.cpp ${PDIR}/modules.cpp

CXXFLAGS=-std=c++14
# LDFLAGS=-lboost_regex # linked by linkee?
# -lgmp -lgmpxx

default: libpbrane.so

libpbrane.so: ${LSRC}
	${CXX} ${CXXFLAGS} -shared -fPIC $^ -o $@

# TODO: this is incomplete, just as a template for inclusion higher up

${LDIR}/modules.cpp.gen: ${MDIR}/*.hpp
	${BDIR}/makemods
${ODIR}/modules.o: ${LDIR}/modules.cpp ${LDIR}/modules.cpp.gen
	${CXX} -c -o $@ $< ${CXXFLAGS}

clean:
	rm -rf ${ODIR}/*.o ${LDIR}/modules.cpp.gen

