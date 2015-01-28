all: wc main map check

check: check.o
	gcc check.o -o check
map: map.o
	gcc map.o -o map
main: main.o
	gcc main.o -o main
wc: wc.o
	gcc wc.o -o wc

main.o: main.c
	gcc -c -g main.c
wc.o: wc.c
	gcc -c -g wc.c
map.o: map.c
	gcc -c -g map.c
check.o: check.c
	gcc -c -g check.c

clean:
	rm -rf *o main wc map check