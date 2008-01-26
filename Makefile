CC=gcc
CFLAGS= -x c++ -O3 -funsigned-char \
	-DRESOURCEPATH=\"$(INSTALLRESOURCEPATH)\" \
	-DSCOREPATH=\"$(INSTALLHISCORES)\"
LIBS=-lm -lSDL -lSDL_mixer -lstdc++

RESOURCES=data/Resources data/Ego data/Psyche data/Id data/Voices

INSTALLBIN=/usr/games/asylum
INSTALLRESOURCEPATH=/usr/share/games/asylum
INSTALLHISCORES=/var/games/asylum

INSTALLGROUP=games
CHGRP=chgrp
CHMOD=chmod

# For a non-root install, try something like this:
#
#INSTALLBIN=/home/blotwell/bin/asylum
#INSTALLRESOURCEPATH=/home/blotwell/lib/asylum
#INSTALLHISCORES=/home/blotwell/.asylum-hiscores
#
#INSTALLGROUP=foo
#CHGRP=echo
#CHMOD=echo

default: build

$(INSTALLBIN): asylum Makefile
	cp asylum $(INSTALLBIN)
	$(CHGRP) $(INSTALLGROUP) $(INSTALLBIN)
	$(CHMOD) g+s $(INSTALLBIN)
	$(CHMOD) a+x $(INSTALLBIN)

install-resources: $(RESOURCES) Makefile
	mkdir -p $(INSTALLRESOURCEPATH)
	cp -r $(RESOURCES) $(INSTALLRESOURCEPATH)/
	$(CHGRP) -R $(INSTALLGROUP) $(INSTALLRESOURCEPATH)/
	$(CHMOD) -R a+rx $(INSTALLRESOURCEPATH)/

install-hiscores: Makefile
	mkdir -p $(INSTALLHISCORES)
	$(CHGRP) $(INSTALLGROUP) $(INSTALLHISCORES)
	$(CHMOD) g+rwx,o-rwx $(INSTALLHISCORES)

install-binary: $(INSTALLBIN)

install: install-resources install-hiscores install-binary

uninstall:
	rm -rf $(INSTALLBINARY) $(INSTALLRESOURCEPATH) $(INSTALLHISCORES)

oggs:
	bash -c 'pushd data; for i in */Music?; do pushd ..; ./asylum --dumpmusic $$i `if (echo \$$i|grep Resources.Music2>/dev/null); then echo -n --slower; fi`; \
	popd;\
	tail -c +33 $$i.au| \
	oggenc - --raw --raw-endianness=1 --raw-rate=22050 --artist="Andy Southgate" \
	--album="Background music for Asylum computer game" \
	>$$i.ogg;\
	rm $$i.au;\
	done; popd'

build: asylum

asylum: asylum.c asylum.h Makefile
	$(CC) $(CFLAGS) -o asylum asylum.c $(LIBS)

clean:
	rm asylum
