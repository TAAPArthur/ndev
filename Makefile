all: config.h ndev

ndev: ndev.o
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

config.h:
	cp config.def.h $@

install: ndev
	install -D -t $(DESTDIR)/bin ndev

clean:
	rm -f ndev *.o
