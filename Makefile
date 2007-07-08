CC=gcc
CFLAGS=-x c++ -funsigned-char -ggdb3 -DRESOURCEPATH=\"$(INSTALLRESOURCEPATH)\" \
	-DSCOREPATH=\"$(INSTALLHISCORES)\"
LIBS=-lm -lSDL -lSDL_mixer -lstdc++

RESOURCES=data/Resources data/Ego data/Psyche data/Id data/Voices

INSTALLBIN=/usr/local/games/asylum
INSTALLRESOURCEPATH=/usr/share/games/asylum
INSTALLHISCORES=/var/games/asylum

INSTALLGROUP=games


default: build

$(INSTALLBIN): asylum Makefile
	cp asylum $(INSTALLBIN)
	chgrp $(INSTALLGROUP) $(INSTALLBIN)
	chmod g+s $(INSTALLBIN)
	chmod a+x $(INSTALLBIN)

$(INSTALLRESOURCEPATH): $(RESOURCES) Makefile
	mkdir -p $(INSTALLRESOURCEPATH)
	cp -r $(RESOURCES) $(INSTALLRESOURCEPATH)/
	chgrp -R $(INSTALLGROUP) $(INSTALLRESOURCEPATH)/
	chmod -R a+rx $(INSTALLRESOURCEPATH)/

$(INSTALLHISCORES): Makefile
	mkdir -p $(INSTALLHISCORES)
	chgrp $(INSTALLGROUP) $(INSTALLHISCORES)
	chmod g+rwx,o-rwx $(INSTALLHISCORES)

install-binary: $(INSTALLBIN)

install-hiscores: $(INSTALLHISCORES)

install-resources: $(INSTALLRESOURCEPATH)

install: install-resources install-hiscores install-binary

uninstall:
	rm -rf $(INSTALLBINARY) $(INSTALLRESOURCEPATH) $(INSTALLHISCORES)

build: asylum

asylum: asylum.c asylum.h Makefile
	$(CC) $(CFLAGS) -o asylum asylum.c $(LIBS)

clean:
	rm asylum
