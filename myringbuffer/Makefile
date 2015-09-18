##
## @file Makefile
## Betriebssysteme sender, receiver, ringpuffer File
## Beispiel 3
##
## @author Andrea Maierhofer	    (1410258024)		ic14b024@technikum-wien.at
## @author Reinhard Mayr	    (1410258030)		ic14b030@technikum-wien.at
## @author Thomas Schmid	    (1410258013)		ic14b013@technikum-wien.at
## @date 2014/05/05
##
## @version $Revision: 2.1 $
##
## @todo nothing right now
##
## Last Modified: $Author: Thomas Schmid $
##

##
## ------------------------------------------------------------- variables --
##

CC=gcc
CFLAGS=-DDEBUG -Wall -Werror -Wextra -Wstrict-prototypes -pedantic -fno-common -g -O3
CD=cd
CP=cp
MV=mv
GREP=grep
DOXYGEN=doxygen

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.o: %.c
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

all: sender receiver

sender: sender.o ringbuffer.o
	$(CC) $(OPTFLAGS) sender.o ringbuffer.o -lsem182 -o sender

receiver: receiver.o ringbuffer.o
	$(CC) $(OPTFLAGS) receiver.o ringbuffer.o -lsem182 -o receiver

clean:
	$(RM) *.o *~  sender receiver

distclean: clean
	$(RM) -r doc

doc: html pdf

html:
	$(DOXYGEN) doxygen.dcf

pdf: html
	$(CD) doc/pdf && \
	$(MV) refman.tex refman_save.tex && \
	$(GREP) -v $(EXCLUDE_PATTERN) refman_save.tex > refman.tex && \
	$(RM) refman_save.tex && \
	make && \
	$(MV) refman.pdf refman.save && \
	$(RM) *.pdf *.html *.tex *.aux *.sty *.log *.eps *.out *.ind *.idx \
	      *.ilg *.toc *.tps Makefile && \
	$(MV) refman.save refman.pdf
