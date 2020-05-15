#TODO: replace ID with your own IDS, for example: 123456789_123456789
SUBMITTERS := <315184259>_<208136176>‏
COMPILER := g++
COMPILER_FLAGS := --std=c++11
SRCS := Commands.cpp signals.cpp smash.cpp
HDRS := Commands.h signals.h

SMASH_BIN := smash

$(SMASH_BIN):
	$(COMPILER) $(COMPILER_FLAGS) $^ $(SRCS) -o $@

zip: $(SRCS) $(HDRS)
	zip $(SUBMITTERS).zip $^ submitters.txt Makefile

clean:
	rm *.o
	rm -rf $(SMASH_BIN)
	rm -rf $(SUBMITTERS).zip
