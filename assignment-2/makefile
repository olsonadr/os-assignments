# variables
ONID=olsonn2
SRC_DIR=.
OUT_DIR=.
FLAGS=-g3


# special
.PHONY: clean cleanexe cleanrooms build adventure buildrooms
default: $(ONID).buildrooms $(ONID).adventure

# easy
buildrooms:	$(ONID).buildrooms
adventure: $(ONID).adventure


# helpful
build: $(ONID).buildrooms $(ONID).adventure
	./$(ONID).buildrooms

start:
	./$(ONID).adventure


# specifics
$(ONID).buildrooms:	$(ONID).buildrooms.c
	gcc -o $(ONID).buildrooms $(ONID).buildrooms.c $(FLAGS)

$(ONID).adventure:	$(ONID).adventure.c
	gcc -o $(ONID).adventure $(ONID).adventure.c -lpthread $(FLAGS)


# cleanup
clean: cleanexe cleanrooms cleantime

cleanexe:
	rm -f $(ONID).buildrooms
	rm -f $(ONID).adventure

cleanrooms:
	rm -f -r olsonn2.rooms*

cleantime:
	rm -f currentTime.txt
