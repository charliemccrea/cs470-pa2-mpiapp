default: mergesort par_mergesort

mergesort: mergesort.c
	gcc -g -O2 --std=c99 -Wall -o mergesort.out mergesort.c

par_mergesort: par_mergesort.c
	mpicc -g -O2 --std=c99 -Wall -o par_mergesort.out par_mergesort.c

clean:
	rm -f mergesort.out par_mergesort.out
