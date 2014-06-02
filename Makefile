multi_away.so: multi_away.cpp
	znc-buildmod multi_away.cpp

install: multi_away.so
	cp multi_away.so ~/.znc/modules/
