all: ld add.so

ld: ld.c
	gcc -static -nostdlib $^ -o $@

dep.so: dep.c
	gcc -shared -nostdlib $^ -o $@ -fPIC
	cp $@ ../../src/resource

add.so: add.c dep.so
	gcc -shared -nostdlib $^ -o $@ -fPIC
	cp $@ ../../src/resource

clean:
	rm -f ld add.so dep.so

.PHONY: clean copy
