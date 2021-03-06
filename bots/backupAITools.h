#ifndef AITOOLS_H
#define AITOOLS_H

#include "globals.h"
#include "PlanetWars.h"
#include "GeneralTools.h"

#include <map>
#include <iterator>	//jsut used for debugging!~
#include <algorithm>
#include <boost/bind.hpp>


template <typename T> std::vector<Planet> myNicestPlanets
(const T& begin, const T& end, const PlanetWars& pw);

unsigned getMyPlanetScore(const Planet& p, const PlanetWars& pw);

bool myp1GTp2(const Planet& p1, const Planet& p2, const PlanetWars& pw);

template <typename T> unsigned _bestRecursive(const T& begin, const T& end, unsigned fleetsLeft, unsigned wantedTurnBenefit, const PlanetWars& pw, unsigned currentIndex, const Planet& home);

/////
//sorting planets according to niceness
/////
unsigned getMyPlanetScore(const Planet& p, const PlanetWars& pw)
{
	using namespace std;
	unsigned score = 0;
	vector<Planet> others = pw.NotMyPlanets();

	//potential fleetsize * planet distance
	for(vector<Planet>::iterator it = others.begin(); it!= others.end();++it)
		score += getPotentialCost(*it, planetTurnDistance(*it, p),pw) * planetTurnDistance(p, *it);

	return score;
}
bool myp1GTp2(const Planet& p1, const Planet& p2, const PlanetWars& pw)
{
	//not, because higher score is bad.
	return !(getMyPlanetScore(p1, pw) > getMyPlanetScore(p2, pw));
}
template <typename T> 
std::vector<Planet> myNicestPlanets(const T& begin, const T& end
													, const PlanetWars& pw)
{
	using namespace tr1boost::placeholders;
	std::vector<Planet> sortedPlanets(begin, end);
	std::sort(sortedPlanets.begin(), sortedPlanets.end()
					, tr1boost::bind(myp1GTp2, _1, _2, pw));

	return sortedPlanets;
}

class nicestPlanets
{
	public:
	nicestPlanets(const Planet& planet, const std::vector<Planet>::iterator& begin, const std::vector<Planet>::iterator& end, const PlanetWars& pw)
		:singleCache(), singleCachePlanets(), finalCacheIndex(0)
			, bestRecursiveCount(0)
	{
		using namespace std;
		//This is definately not optimal... consideration should be made on the safety of the planet instead. For example a planet may be far away from me, but further from my enemy, and therefore it would be a great place to start by taking.
		unsigned wantedTurnBenefit = startingAverageDistance + 3 + (totalTurnsPassed/10);

		//get the list of planets
		vector<Planet> otherPlanets = pw.NotMyPlanets();

		int availableShips = std::min(getOriginalPlanet(planet, pw).NumShips()
												, planet.NumShips());
		debug << "ships used in bestRecursive: " << availableShips << std::endl;
		_bestRecursive(begin, end, availableShips, wantedTurnBenefit, pw, 0, planet);
		debug << "NicestPlanets, recursion iterations: " << bestRecursiveCount
				<< std::endl;
		debug << "size of cache is: " << singleCache.size() << std::endl;
		debug << "final cache planets: " << singleCachePlanets[finalCacheIndex] << std::endl;
	
		result = getPlanetsFromBinaryMapping(
						singleCachePlanets[finalCacheIndex], begin, end);
	}
	unsigned _bestRecursive(const std::vector<Planet>::iterator& begin
								, const std::vector<Planet>::iterator& end
								, unsigned fleetsLeft
								, unsigned wantedTurnBenefit, const PlanetWars& pw,
								unsigned currentIndex, const Planet& home)
	{
	 //we have any more planets to check?
	 if(begin != end)
	 {
		//Check cache, and return cache, if the cache exists.
		std::map<unsigned, unsigned>::iterator it;
		if((it = singleCache.find(getCacheIndex(currentIndex, fleetsLeft))) 
					 != singleCache.end())
				return it->second;
		//
	
		//get the potential cost of taking over the *begin planet.
		unsigned turnsAway = planetTurnDistance(*begin, home);
 		unsigned potentialCost = getPotentialCost(*begin, turnsAway, pw);
		//potentialCost = distancePotentialCost(potentialCost, *begin, home
	//														, turnsAway, pw);
		unsigned firstFleet, secondFleet, firstWorth, secondWorth;

		//Recursion
		//secondWorth: the best worth if we exclude the *begin planet.
		secondWorth = _bestRecursive(begin+1, end
					, secondFleet = fleetsLeft
					, wantedTurnBenefit, pw, currentIndex+1, home);

		//if we have enough fleets, then compare worth, else return excluding
		if(fleetsLeft >= potentialCost)
			firstWorth = xTurnBenefit(wantedTurnBenefit
					, turnsAway, *begin) 
						+ _bestRecursive(begin+1, end
							, firstFleet = fleetsLeft - potentialCost
							, wantedTurnBenefit, pw, currentIndex+1, home);
		else
			return setCacheAndReturn(currentIndex, fleetsLeft, secondWorth);
		//

		//choose the best worth, then cache it and return
		if(firstWorth > secondWorth)
			return setCacheAndReturn(currentIndex, fleetsLeft, firstWorth
						, twoPowerOfX(currentIndex), potentialCost);
		else
			return setCacheAndReturn(currentIndex, fleetsLeft, secondWorth);
		//Note: if we want this effeciency, then we need to sort the planets 
		//by numOfShips, from least to greatest.
	 }

	 //we always return if both ifs are true, so otherwise, we return 0.
	 //this means we are out of planets.
	 return 0;
	}

	std::vector<Planet> Result() const
	{
		return result;
	}

	std::vector<Planet> operator()() const
	{
		return Result();
	}

	unsigned NumberOfIterations() const
	{
		return bestRecursiveCount;
	}

	private:
	std::vector<Planet> result;

	unsigned bestRecursiveCount;

	//for knapsack problem
	//key: 2 parts: 
	//			A (# / 100000):	last planet processed
	//			B (# % A):			remaining fleets
	//value:
	//			cache worth
	std::map<unsigned, unsigned> singleCache;
	//key: same as above
	//value: binary interpretation of indexed planets;
	std::map<unsigned, unsigned> singleCachePlanets;
	//last cacheIndex, used.
	unsigned finalCacheIndex;

	int distancePotentialCost(int potentialCost, const Planet& planet
										, const Planet& myPlanet, unsigned turnsAway
										, const PlanetWars& pw)
	{
		using namespace tr1boost::placeholders;
		int turnsInTheFuture = 5;
		//if we are winning, then screw it, we'll attack anyways.
		//Keep pressure!
		// also obviously if we are closer, then we will win.
		std::vector<Planet> enemyPlanets = pw.EnemyPlanets();
 
 		//remove all enemy planets further, than my attacking planet.
		std::vector<Planet>::iterator fend = std::remove_if(enemyPlanets.begin()
											, enemyPlanets.end()
											, tr1boost::bind(closerP, myPlanet, _1
												, planet));

			debug << "their are " << std::distance(enemyPlanets.begin(), fend) << " planets closer than mine" << std::endl;
		//if their are no enemy planets closer than my planet,
		//	or we are winning (Keep Pressure!). Go for it!
		if(fend == enemyPlanets.begin() || weAreWinning(turnsInTheFuture, pw))
			return potentialCost;

		enemyPlanets.erase(fend, enemyPlanets.end());
		//Note: this will have to do, but it is horribly ineffecient.
		//this planets costs enough ships for all enemy planets closer, and 
		//their growth rate for the TOTAL turns it will take for our
		//planet to reach this planet.
		return potentialCost + pShipsSum(enemyPlanets.begin(), enemyPlanets.end()) + (pGrowthSum(enemyPlanets.begin(), enemyPlanets.end()) * turnsAway);
	}

	unsigned setCacheAndReturn(unsigned currentIndex, unsigned fleetsLeft
							, unsigned returnWorth, unsigned newCachePlanets = 0
							, unsigned potentialCost = 0)
	{
			unsigned prevCacheIndex 
				= getCacheIndex(currentIndex+1, fleetsLeft - potentialCost);
			unsigned prevCachePlanets
				= singleCachePlanets[prevCacheIndex];

			unsigned cacheIndex = finalCacheIndex = 
					getCacheIndex(currentIndex, fleetsLeft);
			singleCache[cacheIndex]				= returnWorth;
			singleCachePlanets[cacheIndex]	
				= prevCachePlanets | newCachePlanets;

			return returnWorth;
	}
};

template <typename T>
std::vector<Move> getDefenceMoves(T begin, T end, const PlanetWars& pw)
{
	using namespace tr1boost::placeholders;
	//the move list we use to store the moves for this idea.
	std::vector<Move> moves;

	//seperate planets into safe and under attack.
	std::vector<Planet> dangerPeas;
	std::vector<Planet> safePeas;
	for(T it = begin; it != end; ++it)
	{
		if(it->NumShips() < 0)
		{
			dangerPeas.push_back(*it);
			debug << "planet in danger id("<< it->PlanetID() <<"), NumShips: " 
				<< it->NumShips() << std::endl;
		}
		else
			safePeas.push_back(*it);
	}

	//sort in danger by highest growth (nicest planets basically)
	std::sort(dangerPeas.begin(), dangerPeas.end(), growthGT);

	//for each in danger planet
	for(T it = dangerPeas.begin(); it != dangerPeas.end(); ++it)
	{
		debug << "testing endangered planet(" << it->PlanetID() << ")"
			<<std::endl;
		//get the vector of fleets attacking the planet.
		std::vector<Fleet> incomingFleets = fleetsIncomingToPlanet(*it, pw);

		//sort by turnDistance (closest first)
		std::sort(incomingFleets.begin(), incomingFleets.end(), fDistLT);
		debugFleets(incomingFleets.begin(), incomingFleets.end());

		//store planetShipsRemaining defendingplanet->NumShips
		std::vector<Planet> minetemp = pw.MyPlanets();
		int planetShipsRemaining = getOriginalPlanet(*it, pw).NumShips();

		//sort safe planets by closeness to current in danger planet.
		std::sort(safePeas.begin(), safePeas.end(), tr1boost::bind(closerP, _1, _2, *it));

		//create a new move that targets the defending planet
		Move currentMove(*it);

		std::vector<Planet>::iterator safeit = safePeas.begin();
		debug << "safe planets to grab from: " << safePeas.size() << std::endl;
		debug << "incomingFleets.size: " << incomingFleets.size() << std::endl;
		debug << "incomingFleets turns remaining: " <<
					incomingFleets.begin()->TurnsRemaining() << std::endl;
		debug << "safe planets turn distance: " << planetTurnDistance(*safeit, *it) <<std::endl;
		//continue until
			//we defeat all fleets.
			//we have no more ships to defend with
			//our remaining planets are too far to defend against.

		//store current growthGain, starting at 0.
		unsigned growthGain = 0;
		unsigned attackingShips = 0;
		while(!(incomingFleets.empty()
				  || safeit == safePeas.end()))
		{
			std::vector<Fleet>::iterator afit = incomingFleets.begin();
			//attackingShips = 0, means we will have a new fleet next.
			if(attackingShips == 0)
				attackingShips = afit->NumShips();

			//add in growthrate of planet
			planetShipsRemaining += (afit->TurnsRemaining() - growthGain)
											* it->GrowthRate();

			growthGain = afit->TurnsRemaining();
			//
			int availableShips 
				= std::min(getOriginalPlanet(*safeit, pw).NumShips()
								, safeit->NumShips());


			if(afit->Owner() == 1)
			{
				planetShipsRemaining += attackingShips;
				attackingShips = 0;
				incomingFleets.erase(afit);
			}
			else 
			{

				//if the planet can handle it, just subtract.
				if(planetShipsRemaining >= attackingShips)
				{
					planetShipsRemaining -= attackingShips;

					//remove incomingFleets.begin from incomingFleets.
					incomingFleets.erase(afit);
					attackingShips = 0;
				}
				else 
				{
					if(incomingFleets.begin()->TurnsRemaining() < 
							  planetTurnDistance(*safeit, *it))
						break;
					if(attackingShips < planetShipsRemaining + availableShips)
					{
						//add to move with fleets enough to protect planet.
						currentMove.addFleet(*safeit
							, attackingShips - planetShipsRemaining);
						debug << "sending fleet: id, destID, ships: "
							<< safeit->PlanetID() << ", " << it->PlanetID()
							<< ", " << attackingShips << std::endl;
						attackingShips = 0;

						//remove incomingFleets.begin from incomingFleets.
						incomingFleets.erase(afit);
						attackingShips = 0;
					}
					else
					{
						currentMove.addFleet(*safeit, availableShips);
						attackingShips -= planetShipsRemaining + availableShips;
						planetShipsRemaining = 0;
						++safeit;
					}
				}
			}
		}
		//if we are actually defending, then add move to Moves vector
		if(incomingFleets.empty())
		{
			moves.push_back(currentMove);
			//we must update the ships every turn, so we are choosing
			//	our defences with the correct num of ships.
			updatePlanetsByMove(safePeas.begin(), safePeas.end(), currentMove);
		}
		else
			debug << "not enough to defend i guess? Even with numships = " 
				<< myTotalShips(pw) << std::endl;
	}
	//we must then update the planets on the more global scale, so we choose
	// our attacking correctly.
	for(std::vector<Move>::iterator it = moves.begin(); it != moves.end();
			++it)
		updatePlanetsByMove(begin, end, *it);

	return moves;
}

//////
//////
//////

#endif
