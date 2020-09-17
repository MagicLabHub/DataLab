#include <stdio.h>
#include <stdlib.h>
//#include <fstream>

#ifdef WIN32
#include <windows.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

/* Routines for using cycle counter */

struct tms{
	clock_t tms_utime;  // User CPU time
	clock_t tms_stime;  // System CPU time
	clock_t tms_cutime; // User CPU time of terminated child processes
	clock_t tms_cstime; // System CPU time of terminated child processes
};

clock_t times(struct tms *t);

/* Start the counter */
void start_counter();

/* Get # cycles since counter started */
double get_counter();


/* Measure overhead for counter */
double ovhd();

/* Determine clock rate of processor */
double mhz(int verbose);

/* Determine clock rate of processor, having more control over accuracy */
double mhz_full(int verbose, int sleeptime);

/** Special counters that compensate for timer interrupt overhead */

void start_comp_counter();

double get_comp_counter();

