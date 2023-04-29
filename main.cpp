
#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Global Variable
unsigned int seed = 0;

// Global State
int NowYear;  // 2023 - 2028
int NowMonth; // 0 - 11

float NowPrecip;    // inches of rain per month
float NowTemp;      // temperature this month
float NowHeight;    // rye grass height in inches
int NowNumRabbits;  // number of rabbits in the current population
int NowDeadRabbits; // The number of rabbits that have died since the last simulated month
int NowNumInfected; // The number of infected rabbits that have resurected from their "cordycep infection"
int NowNumHunted;

// Parameters

const float RYEGRASS_GROWS_PER_MONTH = 20.0;
const float ONE_RABBITS_EATS_PER_MONTH = 1.0;

const float AVG_PRECIP_PER_MONTH = 12.0; // average
const float AMP_PRECIP_PER_MONTH = 4.0;  // plus or minus
const float RANDOM_PRECIP = 2.0;         // plus or minus noise

const float AVG_TEMP = 60.0;    // average
const float AMP_TEMP = 20.0;    // plus or minus
const float RANDOM_TEMP = 10.0; // plus or minus noise

const float MIDTEMP = 60.0;
const float MIDPRECIP = 14.0;

//-----------------------
// Used for InitBarrier
omp_lock_t Lock;
volatile int NumInThreadTeam;
volatile int NumAtBarrier;
volatile int NumGone;
//-----------------------

// specify how many threads will be in the barrier:
//	(also init's the Lock)

void InitBarrier(int n)
{
  NumInThreadTeam = n;
  NumAtBarrier = 0;
  omp_init_lock(&Lock);
}

// have the calling thread wait here until all the other threads catch up:

void WaitBarrier()
{
  omp_set_lock(&Lock);
  {
    NumAtBarrier++;
    if (NumAtBarrier == NumInThreadTeam)
    {
      NumGone = 0;
      NumAtBarrier = 0;
      // let all other threads get back to what they were doing
      // before this one unlocks, knowing that they might immediately
      // call WaitBarrier( ) again:
      while (NumGone != NumInThreadTeam - 1)
        ;
      omp_unset_lock(&Lock);
      return;
    }
  }
  omp_unset_lock(&Lock);

  while (NumAtBarrier != 0)
    ; // this waits for the nth thread to arrive

#pragma omp atomic
  NumGone++; // this flags how many threads have returned
}

// Ranf
float Ranf(unsigned int *seedp, float low, float high)
{
  float r = (float)rand_r(seedp); // 0 - RAND_MAX

  return (low + r * (high - low) / (float)RAND_MAX);
}

// Temperature and percipitation
void temperature_and_percipitation()
{
  float ang = (30. * (float)NowMonth + 15.) * (M_PI / 180.);

  float temp = AVG_TEMP - AMP_TEMP * cos(ang);
  NowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);

  float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
  NowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);
  if (NowPrecip < 0.)
    NowPrecip = 0.;
}

//--------------------------------------------------------
// Structure of simulation functinos
//
// while( NowYear < 2029 )
// {
// 	// compute a temporary next-value for this quantity
// 	// based on the current state of the simulation:
// 	. . .

// 	// DoneComputing barrier:
// 	WaitBarrier( );	-- or --   #pragma omp barrier;
// 	. . .

// 	// DoneAssigning barrier: SHOULD BE COPYING THE NEXT STATE INTO THE NOW VARIABLES
// 	WaitBarrier( );	-- or --   #pragma omp barrier;
// 	. . .

// 	// DonePrinting barrier:
// 	WaitBarrier( );	-- or --   #pragma omp barrier;
// 	. . .
// }
//--------------------------------------------------------

float Sqr(float x)
{
  return x * x;
}

int Rabbits()
{
  while (NowYear < 2029)
  {
    int nextDeadRabbits = NowNumRabbits;
    
    // Subtracting the number of rabbits that were hunted by the infected
    int nextNumRabbits = NowNumRabbits - NowNumHunted; 
    int carryingCapacity = (int)(NowHeight);

    if (nextNumRabbits < carryingCapacity)
      nextNumRabbits++;
    else if (nextNumRabbits > carryingCapacity)
      nextNumRabbits--;

    //nextNumRabbits -= NowNumHunted;

    if (nextNumRabbits < 0)
      nextNumRabbits = 0;

    nextDeadRabbits = nextDeadRabbits - nextNumRabbits;

    if (nextDeadRabbits < 0)
      nextDeadRabbits = 0;
    // Done Calculating
    WaitBarrier();

    NowDeadRabbits = nextDeadRabbits;
    NowNumRabbits = nextNumRabbits;

    // Done Assigning
    WaitBarrier();

    // Done Printing
    WaitBarrier();
  }
  return 0;
}

void RyeGrass()
{
  while (NowYear < 2029)
  {
    float tempFactor = exp(-Sqr((NowTemp - MIDTEMP) / 10.));
    float precipFactor = exp(-Sqr((NowPrecip - MIDPRECIP) / 10.));

    float nextHeight = NowHeight;
    nextHeight += tempFactor * precipFactor * RYEGRASS_GROWS_PER_MONTH;
    nextHeight -= (float)NowNumRabbits * ONE_RABBITS_EATS_PER_MONTH;

    if (nextHeight < 0)
    {
      nextHeight = 0;
    }
    // Done computing
    WaitBarrier();

    NowHeight = nextHeight;

    // Done Assigning
    WaitBarrier();

    // Done Printing
    WaitBarrier();
  }
}

void Watcher()
{
  while (NowYear < 2029)
  {
    // Done Computing
    WaitBarrier();

    // Done Assigning
    WaitBarrier();

    // printf("NowMonth: %d\n", NowMonth);
    // printf("NowHeight: %f\n", NowHeight);
    // printf("NowNumRabbits: %d\n", NowNumRabbits);
    // printf("NowDeadRabbits: %d\n", NowDeadRabbits);
    // printf("NowNumInfected: %d\n", NowNumInfected);
    // printf("NowNumHunted: %d\n\n", NowNumHunted);

    fprintf(stderr, "%d, %f, %d, %d, %f, %f\n", NowMonth, NowHeight, NowNumRabbits, NowNumInfected, NowTemp, NowPrecip);

    NowMonth += 1;
    if ((NowMonth % 12) == 0)
    {
      NowYear += 1;
    }

    // {
    //   NowMonth += 1;
    // }

    temperature_and_percipitation();

    // Done printing barrier
    WaitBarrier();
  }
}

int Infected()
{
  while (NowYear < 2029)
  {
    int selectInfected = NowDeadRabbits;
    int nextNumInfected = 0;
    int selectHuntedRabbits = NowNumRabbits;
    int nextNumHunted = 0;

    // 20% chance of a dead rabbit becoming an infected a rabbit 
    // infected with cordyceps and will resurrect within the moth
    while (selectInfected > 0)
    {

      if (rand() % 100 < 20)
      {
        nextNumInfected += 1;
      }

      selectInfected -= 1;
    }

    while (selectHuntedRabbits > 0)
    {
      if (rand() % 100 < NowNumInfected * 2)
      {
        nextNumHunted += 1;
      }

      selectHuntedRabbits -= 1;
    }
    // Done Calculating
    WaitBarrier();

    NowNumInfected += nextNumInfected;
    NowNumHunted = nextNumHunted;

    // Done Assigning
    WaitBarrier();

    // Done Printing
    WaitBarrier();
  }
  return 0;
}

// void function()
// {
// #pragma omp parallel sections
//   {
// #pragma omp section
//     {
//       Rabbits();
//     }

// #pragma omp section
//     {
//       RyeGrass();
//     }

// #pragma omp section
//     {
//       Watcher();
//     }

// #pragma omp section
//     {
//       Infected(); // your own
//     }
//   } // implied barrier -- all functions must return in order
//     // to allow any of them to get past here
// }

int main(void)
{

  // Might want to add to main
  NowMonth = 0;
  NowYear = 2023;
  srand(time(NULL));
  // starting state (feel free to change this if you want):
  NowNumRabbits = 1;
  NowHeight = 5.;
  NowDeadRabbits = 0; // Initialize to avoid having undefined behavior.
  NowNumInfected = 0;

  // Initialize weather and percipitation
  temperature_and_percipitation();

  omp_set_num_threads(4); // same as # of sections USE 4 after adding your own agent
  InitBarrier(4);         // Use 4 after adding your own agent
#pragma omp parallel sections
  {
#pragma omp section
    {
      Rabbits();
    }

#pragma omp section
    {
      RyeGrass();
    }

#pragma omp section
    {
      Watcher();
    }

#pragma omp section
    {
      Infected(); // your own
    }
  } // implied barrier -- all functions must return in order
    // to allow any of them to get past here
  return 0;
}