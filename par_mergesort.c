/*
 * par_mergesort.c
 *
 * CS 470 Project 2 (MPI)
 * Parallel version.
 *
 * Name(s): Charlie McCrea, Bikash Adhikari
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// histogram bins
#define BINS 10

// maximum random number
#define RMAX 100

// enable debug output
/*#define DEBUG*/

// timing macros (must first declare "struct timeval tv")
#define START_TIMER(NAME) gettimeofday(&tv, NULL); \
	double NAME ## _time = tv.tv_sec+(tv.tv_usec/1000000.0);
#define STOP_TIMER(NAME) gettimeofday(&tv, NULL); \
	NAME ## _time = tv.tv_sec+(tv.tv_usec/1000000.0) - (NAME ## _time);
#define GET_TIMER(NAME) (NAME##_time)

// "count_t" used for number counts that could become quite high
typedef unsigned long count_t;

int *nums;			// random numbers
count_t  global_n;	// global "nums" count
count_t  shift_n;	// global left shift offset
count_t *hist;		// histogram (counts of "nums" in bins)

/*
 * Parse and handle command-line parameters. Returns true if parameters were
 * valid; false if not.
 */
bool parse_command_line(int argc, char *argv[])
{
	// read command-line parameters
	if (argc != 3)
	{
		printf("Usage: %s <n> <shift>\n", argv[0]);
		return false;
	}
	else
	{
		global_n = strtol(argv[1], NULL, 10);
		shift_n  = strtol(argv[2], NULL, 10);
	}

	// check shift offset
	if (shift_n > global_n)
	{
		printf("ERROR: shift offset cannot be greater than N\n");
		return false;
	}

	return true;
}

/*
 * Allocate and initialize number array and histogram.
 */
void initialize_data_structures()
{
	// initialize local data structures
	nums = (int*)calloc(global_n, sizeof(int));
	if (nums == NULL)
	{
		fprintf(stderr, "Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	hist = (count_t*)calloc(BINS, sizeof(count_t));
	if (hist == NULL)
	{
		fprintf(stderr, "Out of memory!\n");
		exit(EXIT_FAILURE);
	}
}

/*
 * Compares two ints. Suitable for calls to standard qsort routine.
 */
int cmp(const void* a, const void* b)
{
	return *(int*)a - *(int*)b;
}

/*
 * Print contents of an int list.
 */
void print_nums(int *a, count_t n)
{
	for (count_t i = 0; i < n; i++)
	{
		printf("%d ", a[i]);
	}
	printf("\n");
}

/*
 * Print contents of a count list (i.e., histogram).
 */
void print_counts(count_t *a, count_t n)
{
	for (count_t i = 0; i < n; i++)
	{
		printf("%lu ", a[i]);
	}
	printf("\n");
}

/*
 * Merge two sorted lists ("left" and "right) into "dest" using temp storage.
 */
void merge(int left[], count_t lsize, int right[], count_t rsize, int dest[])
{
	count_t dsize = lsize + rsize;
	int *tmp = (int*)malloc(sizeof(int) * dsize);
	if (tmp == NULL)
	{
		fprintf(stderr, "Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	count_t l = 0, r = 0;
	for (count_t ti = 0; ti < dsize; ti++)
	{
		if (l < lsize && (left[l] <= right[r] || r >= rsize))
		{
			tmp[ti] = left[l++];
		}
		else
		{
			tmp[ti] = right[r++];
		}
	}
	memcpy(dest, tmp, dsize*sizeof(int));
	free(tmp);
}

/*
 * Generate random integers for "nums".
 */
void randomize()
{
	srand(42);
	for (count_t i = 0; i < global_n; i++)
	{
		nums[i] = rand() % RMAX;
	}
}

/*
 * Calculate histogram based on contents of "nums".
 */
void histogram()
{
	for (count_t i = 0; i < global_n; i++)
	{
		hist[nums[i] % BINS]++;
	}
}

/*
 * Shift "nums" left by the given number of slots. Anything shifted off the left
 * side should rotate around to the end, so no numbers are lost.
 */
void shift_left()
{
	// preserve first shift_n values
	int *tmp = (int*)malloc(sizeof(int) * shift_n);
	for (count_t i = 0; i < shift_n; i++)
	{
		tmp[i] = nums[i];
	}

	// perform shift
	for (count_t i = 0; i < global_n-shift_n; i++)
	{
		nums[i] = nums[(i + shift_n) % global_n];
	}
	for (count_t i = 0; i < shift_n; i++)
	{
		nums[i + global_n - shift_n] = tmp[i];
	}
	free(tmp);
}

/*
 * Merge sort helper (shouldn't be necessary in parallel version).
 */
void merge_sort_helper(int *start, count_t len)
{
	if (len < 100)
	{
		qsort(start, len, sizeof(int), cmp);
	}
	else
	{
		count_t mid = len/2;
		merge_sort_helper(start, mid);
		merge_sort_helper(start+mid, len-mid);
		merge(start, mid, start+mid, len-mid, start);
	}
}

/*
 * Sort "nums" using the mergesort algorithm.
 */
void merge_sort()
{
	merge_sort_helper(nums, global_n);
}

int main(int argc, char *argv[])
{
	// utility struct for timing calls
	struct timeval tv;

	if (!parse_command_line(argc, argv))
	{
		exit(EXIT_FAILURE);
	}

	initialize_data_structures();

	// initialize random numbers
	START_TIMER(rand)
	randomize();
	STOP_TIMER(rand)

#   ifdef DEBUG
	printf("global orig list: "); print_nums(nums, global_n);
#   endif

	// compute histogram
	START_TIMER(hist)
	histogram();
	STOP_TIMER(hist)

	// print histogram
	printf("GLOBAL hist: "); print_counts(hist, BINS);

	// perform left shift
	START_TIMER(shft)
	shift_left();
	STOP_TIMER(shft)

#   ifdef DEBUG
	printf("global shft list: "); print_nums(nums, global_n);
#   endif

	// perform merge sort
	START_TIMER(sort)
	merge_sort();
	STOP_TIMER(sort)

	// print global results
#   ifdef DEBUG
	printf("GLOBAL list: "); print_nums(nums, global_n);
#   endif
	printf("RAND: %.4f  HIST: %.4f  SHFT: %.4f  SORT: %.4f\n",
			GET_TIMER(rand), GET_TIMER(hist), GET_TIMER(shft), GET_TIMER(sort));

	// clean up and exit
	free(nums);
	free(hist);
	return EXIT_SUCCESS;
}
