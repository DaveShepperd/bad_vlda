default: bad_vlda

badvlda: bad_vlda.c Makefile
	gcc -o $@ $<

clean:
	rm -f bad_vlda
