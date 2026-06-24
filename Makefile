CFLAGS = -O2 -Wall

fanctl: fanctl.c
	gcc $(CFLAGS) -o fanctl fanctl.c

install: fanctl
	cp fanctl /usr/local/bin/
	chown root /usr/local/bin/fanctl
	chmod u+s /usr/local/bin/fanctl

uninstall:
	rm -f /usr/local/bin/fanctl

clean:
	rm -f fanctl
