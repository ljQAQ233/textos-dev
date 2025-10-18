all: ld ldbase add.so

ld: ld.c
	gcc -static -nostdlib $^ -o $@

ldbase: ldbase.c
	gcc $^ -o $@

dep.so: dep.c
	gcc -shared -nostdlib $^ -o $@ -fPIC
	cp $@ ../../build/root

add.so: add.c dep.so
	gcc -shared -nostdlib $^ -o $@ -fPIC
	cp $@ ../../build/root

clean:
	rm -f ld ldbase add.so dep.so

.PHONY: clean copy
