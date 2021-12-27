ifdef DEBUG
	VERBOSE=1
	ENV_DUMP=1
endif
ifdef VERBOSE
	CFLAGS += -DVERBOSE
endif
ifdef ENV_DUMP
	CFLAGS += -DENV_DUMP
endif

all: config.h ndev helpers/remapkeys

ndev.o: config.h

helpers/remapkeys.o: helpers/remapkeys.h

ndev: ndev.o
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

config.h:
	cp config.def.h $@

install: ndev helpers/remapkeys
	install -D -t $(DESTDIR)/usr/bin $^

clean:
	rm -f ndev remapkeys *.o helpers/*.o
