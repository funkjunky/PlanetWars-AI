#ifndef GLOBALS_H

#define GLOBALS_H
#include "PlanetWars.h"
#include <fstream>
#include <map>

//this should be changed during submission.
#define tr1boost boost
//#define tr1boost std::tr1

//I need this for compatibility with tr1
namespace boost
{
	namespace placeholders
	{
	}
}

PlanetWars globalpw("");

unsigned totalTurnsPassed = 0;
unsigned startingAverageDistance = 0;
std::ofstream debug("bots/debug/last_statement");
std::ofstream debugdetails("bots/debug/placeholder");

#endif
