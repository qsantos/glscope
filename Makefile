CC := gcc
CFLAGS  = -O3 -std=c99 -Wall -Wextra -Wpedantic -Wformat -Wshadow -Wconversion
LDFLAGS = -O3 -lGL -lglut -lpthread -lm
TARGETS = simplescope singen unbin catrate

all: $(TARGETS)

simplescope:
singen:
unbin:
catrate:

destroy:
	rm -f $(TARGETS)

rebuild: destroy all

.PHONE: all destroy rebuild
