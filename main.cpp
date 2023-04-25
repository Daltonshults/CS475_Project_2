#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Global Variable
unsigned int seed = 0;

// Global State
int NowYear;  // 2023 - 2028
int NowMonth; // 0 - 11

float NowPrecip;   // inches of rain per month
float NowTemp;     // temperature this month
float NowHeight;   // rye grass height in inches
int NowNumRabbits; // number of rabbits in the current population

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

int Rabbits()
{
  return 0;
}

int RyeGrass()
{
  return 0;
}

int Watcher()
{
  return 0;
}

int MyAgent()
{
  return 0;
}

void function()
{
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
      MyAgent(); // your own
    }
  } // implied barrier -- all functions must return in order
    // to allow any of them to get past here
}

int main(void)
{
  omp_set_num_threads(4); // same as # of sections
  printf("main working");
  return 0;
}