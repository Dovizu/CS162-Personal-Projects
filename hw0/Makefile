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
	test "`wc main.c |  sed -e 's/^[ \t]*//' | sed -e "s/[[:space:]]\+/ /g"`" = "`./wc main.c |  sed -e 's/^[ \t]*//' | sed -e "s/[[:space:]]\+/ /g"`"
	test "`wc map.c |  sed -e 's/^[ \t]*//' | sed -e "s/[[:space:]]\+/ /g"`" = "`./wc map.c |  sed -e 's/^[ \t]*//' | sed -e "s/[[:space:]]\+/ /g"`"
	test "`wc wc.c |  sed -e 's/^[ \t]*//' | sed -e "s/[[:space:]]\+/ /g"`" = "`./wc wc.c |  sed -e 's/^[ \t]*//' | sed -e "s/[[:space:]]\+/ /g"`"

	ulimit -s 8192; test "`echo | ./main | grep "stack size: "`" = "stack size: `expr 8192 \* 1024`"
	ulimit -p 7874; test "`echo | ./main | grep "process limit: "`" = "process limit: 7874"
	ulimit -n 1024; test "`echo | ./main | grep "max file descriptors: "`" = "max file descriptors: 1024"