all: wc main map

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

clean:
	rm -rf *o main wc map check

check:
	make clean
	make main wc
	test "`echo | ./main | grep 'stack size: '`" = "stack size: `echo | ulimit -Hs`"