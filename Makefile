# TODO: this is incomplete, just as a template for inclusion higher up

CXXFLAGS=-std=c++1y
LDFLAGS=-lboost_regex
# -lgmp -lgmpxx

${LDIR}/modules.cpp.gen: ${MDIR}/*.hpp
	${BDIR}/makemods
${ODIR}/modules.o: ${LDIR}/modules.cpp ${LDIR}/modules.cpp.gen
	${CXX} -c -o $@ $< ${CXXFLAGS}

clean:
	rm -rf ${ODIR}/*.o ${LDIR}/modules.cpp.gen

