# Makefile for the hello driver.
PROG=	mydriver
SRCS=	hello.c

DPADD+=	${LIBCHARDRIVER} ${LIBSYS} ${LIBBDEV}
LDADD+=	-lchardriver -lsys -lbdev

MAN=

BINDIR?= /usr/sbin

.include <minix.service.mk>
