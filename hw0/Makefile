all: wc

wc: wc.o
	gcc wc.o -o wc
hw0: wc.o
	gcc wc.o -o hw0
main.o: main.c
	gcc -c -g main.c
wc.o: wc.c
	gcc -c -g wc.c
clean:
	rm -rf *o main