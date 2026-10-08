CC     ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic

first_follow: first_follow.c
	$(CC) $(CFLAGS) -o $@ $<

run: first_follow
	./first_follow

clean:
	rm -f first_follow

.PHONY: run clean
