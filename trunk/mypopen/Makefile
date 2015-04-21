##
## @file Makefile
## Betriebssysteme mypopen makefile
## Beispiel 2
##
## @author Andrea Maierhofer <ic14b024@technikum-wien.at>
## @author Reinhard Mayr <ic14b030@technikum-wien.at>
## @author Thomas schmid <ic14b013@technikum-wien.at>
## @date 2015/04/17
##
## @version $Revision: 1 $
##

##
## ------------------------------------------------------------- variables --
##

CC=gcc
CFLAGS=-DDEBUG -Wall -pedantic -Werror -Wextra -Wstrict-prototypes -g -O3
CFLGS2=-Wall -Wextra -Werror -pedantic -o popentest mypopen.o -lpopentest
CFLGS3=-Wall -Wextra -Werror -pedantic -o test-pipe mypopen.o -ltest-pipe
CFLGS4=-o popentest mypopen.o -lpopentest
GREP=grep
CP=cp
CD=cd
MV=mv
GREP=grep
DOXYGEN=doxygen

OBJECTS=mypopen.o

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.o: %.c
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

all: mypopen

lib:
	$(CC) $(CFLGS4) 

mypopen: $(OBJECTS)
	$(CC) $(CFLGS2) && $(CC) $(CFLGS3) 

clean:
	$(RM) *.o *~ mypopen popentest test-pipe

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

##
## ---------------------------------------------------------- dependencies --
##

##
## =================================================================== eof ==
##
