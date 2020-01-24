# variables
ONID=olsonn2
SRC_DIR=.
OUT_DIR=.
FLAGS=-g3


# special
.PHONY: clean cleanall $(ONID).buildrooms
default: $(ONID).buildrooms


# easy
buildrooms:	$(ONID).buildrooms
adventure:	$(ONID).adventure


# specifics
$(ONID).buildrooms:	$(ONID).buildrooms.c
	gcc -o $(ONID).buildrooms $(ONID).buildrooms.c $(FLAGS)

$(ONID).adventure:	$(ONID).adventure.c
	gcc -o $(ONID).adventure $(ONID).adventure.c -lpthread $(FLAGS)


# cleanup
clean:
	rm -f $(ONID).buildrooms

cleanall: clean
	rm -f -r rooms*
