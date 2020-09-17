
#include "clock.h"
static LARGE_INTEGER m_TickFreq; 
static LARGE_INTEGER m_BegCounter;
static LARGE_INTEGER m_EndCounter;

double getTickFreq()
{
	double result = 0.0;
	QueryPerformanceFrequency(&m_TickFreq);
	result = (double)m_TickFreq.QuadPart;
	return result;
}

void start_counter()
{
	QueryPerformanceFrequency(&m_TickFreq);
	QueryPerformanceCounter(&m_BegCounter);
}

double get_counter()
{
	double result = 0.0;
	QueryPerformanceCounter(&m_EndCounter);
	result = (double)(m_EndCounter.QuadPart - m_BegCounter.QuadPart);
	if(result<=0) result = 0.5;
	return result;
}

int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year   = wtm.wYear - 1900;
	tm.tm_mon   = wtm.wMonth - 1;
	tm.tm_mday   = wtm.wDay;
	tm.tm_hour   = wtm.wHour;
	tm.tm_min   = wtm.wMinute;
	tm.tm_sec   = wtm.wSecond;
	tm. tm_isdst  = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return (0);
}

clock_t times(struct tms *t){
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year   = wtm.wYear - 1900;
	tm.tm_mon   = wtm.wMonth - 1;
	tm.tm_mday   = wtm.wDay;
	tm.tm_hour   = wtm.wHour;
	tm.tm_min   = wtm.wMinute;
	tm.tm_sec   = wtm.wSecond;
	tm. tm_isdst  = -1;
	clock = mktime(&tm);
	t->tms_utime  = clock/2U;
	t->tms_stime  = clock/2U;
	t->tms_cutime = 0; // vxWorks is lacking the concept of a child process!
	t->tms_cstime = 0; // -> Set the wait times for childs to 0
	return clock;
}




double ovhd()
{
  /* Do it twice to eliminate cache effects */
  int i;
  double result;
  for (i = 0; i < 2; i++) {
    start_counter();
    result = get_counter();
  }
  return result;
}

/* Determine clock rate by measuring cycles
   elapsed while sleeping for sleeptime seconds */
double mhz_full(int verbose, int sleeptime)
{
  double rate;
  double result;
  start_counter();
  Sleep(sleeptime);
  rate = get_counter();
  result = getTickFreq();
  if (verbose) 
    printf("Processor Clock Rate ~= %.1f MHz\n", rate);
  return result;
}

/* Version using a default sleeptime */
double mhz(int verbose)
{
  return mhz_full(verbose, 1000);
}

/** Special counters that compensate for timer interrupt overhead */

static double cyc_per_tick = 0.0;

#define NEVENT 100
#define THRESHOLD 1000
#define RECORDTHRESH 3000

/* Attempt to see how much time is used by timer interrupt */
static void callibrate(int verbose)
{
  double oldt;
  struct tms t;
  clock_t oldc;
  int e = 0;
  times(&t);
  oldc = t.tms_utime;
  start_counter();
  oldt = get_counter();
  while (e <NEVENT) {
    double newt = get_counter();
    if (newt-oldt >= THRESHOLD) {
      clock_t newc;
      times(&t);
      newc = t.tms_utime;
      if (newc > oldc) {
	double cpt = (newt-oldt)/(newc-oldc);
	if ((cyc_per_tick == 0.0 || cyc_per_tick > cpt) && cpt > RECORDTHRESH)
	  cyc_per_tick = cpt;
	/*
	if (verbose)
	  printf("Saw event lasting %.0f cycles and %d ticks.  Ratio = %f\n",
		 newt-oldt, (int) (newc-oldc), cpt);
	*/
	e++;
	oldc = newc;
      }
      oldt = newt;
    }
  }
  if (verbose)
    printf("Setting cyc_per_tick to %f\n", cyc_per_tick);
}

static clock_t start_tick = 0;

void start_comp_counter() {
  struct tms t;
  if (cyc_per_tick == 0.0)
    callibrate(1);
  times(&t);
  start_tick = t.tms_utime;
  start_counter();
}

double get_comp_counter() {
  double time = get_counter();
  double ctime;
  struct tms t;
  clock_t ticks;
  times(&t);
  ticks = t.tms_utime - start_tick;
  ctime = time - ticks*cyc_per_tick;
  /*
  printf("Measured %.0f cycles.  Ticks = %d.  Corrected %.0f cycles\n",
	 time, (int) ticks, ctime);
  */
  return ctime;
}
