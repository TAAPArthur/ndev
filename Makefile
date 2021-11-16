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

all: config.h ndev

ndev.o: config.h
ndev: ndev.o
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

config.h:
	cp config.def.h $@

install: ndev
	install -D -t $(DESTDIR)/usr/bin ndev

clean:
	rm -f ndev *.o
