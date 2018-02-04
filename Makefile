default: mergesort par_mergesort

mergesort: mergesort.c
	gcc -g -O2 --std=c99 -Wall -o mergesort mergesort.c

par_mergesort: par_mergesort.c
	mpicc -g -O2 --std=c99 -Wall -o par_mergesort par_mergesort.c

clean:
	rm -f mergesort par_mergesort
