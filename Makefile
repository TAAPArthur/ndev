PREFIX          = /usr
DEBUG           = 0
VERBOSE         = $(DEBUG)
VERBOSE_FLAG_1  = -DVERBOSE
ENV_DUMP        = $(DEBUG)
ENV_DUMP_FLAG_1 = -DENV_DUMP
CPPFLAGS        = $(VERBOSE_FLAG_$(VERBOSE)) $(ENV_DUMP_FLAG_$(ENV_DUMP))
LDFLAGS         = -static

all: config.h ndev helpers/remapkeys

ndev.o: config.h

helpers/remapkeys.o: helpers/remapkeys.h

ndev: ndev.o
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

config.h:
	cp config.def.h $@

install: ndev helpers/remapkeys
	install -D -t $(DESTDIR)$(PREFIX)/bin $^

clean:
	rm -f ndev remapkeys *.o helpers/*.o
