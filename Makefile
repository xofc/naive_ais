all: naive_ais

CFLAGS=		-O

OFILES=		aivdm.o crc.o demod.o log.o main.o opt.o tcp.o
CFILES=		aivdm.c crc.c demod.c log.c main.c opt.c tcp.c
HFILES=		naive_ais.h

naive_ais : $(OFILES)
	$(CC) -o naive_ais $(OFILES) -lm -lrtlsdr

$(OFILES):	$(HFILES)

clean:
	rm $(OFILES) naive_ais

tar naive_ais.tgz: $(CFILES)
	tar cvfz naive_ais.tgz Makefile $(HFILES) $(CFILES)
