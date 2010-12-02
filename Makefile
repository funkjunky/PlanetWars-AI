#CC=g++-4
CC=g++

BOTSFLDR = bots
BOTSRCS = $(shell ls $(BOTSFLDR) | grep .*\.cc)
BOTOBJS = $(BOTSRCS:%.cc=%.o)
BOTS = $(BOTSRCS:%.cc=%)

all: MyBot AllBots

clean:
	rm -rf *.o MyBot

MyBot: MyBot.o PlanetWars.o

AllBots: $(BOTOBJS) $(BOTSRCS)

$(BOTS): %: $(BOTSFLDR)/%.o PlanetWars.o
	$(CC) $^ -o $(BOTSFLDR)/$(@:%.cc=%)

$(BOTOBJS): %.o:
	$(CC) $^  -c -o $(BOTSFLDR)/$@
