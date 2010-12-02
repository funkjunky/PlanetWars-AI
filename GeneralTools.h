#ifndef GENERALTOOLS_H
#define GENERALTOOLS_H

#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <string>

#include <boost/bind.hpp>

#include "PlanetWars.h"

class Move
{
	public:
	Move()
		:fleetsUsed(), destination(9999,9999,9999,9999,9999,9999)
	{}
	Move(const std::map<Planet, unsigned>& fleets, const Planet& dest)
		:fleetsUsed(fleets), destination(dest)
	{}

	Move(const Planet& planet, unsigned numOfShips, const Planet& dest)
		:fleetsUsed(), destination(dest)
	{
		fleetsUsed[planet] = numOfShips;
	}

	Move(const Planet& dest)
		:fleetsUsed(), destination(dest)
	{}

	void addFleet(const Planet& p, unsigned fleets)
	{
		fleetsUsed[p] = fleets;
	}

	Planet Destination() const
	{
		return destination;
	}
	std::map<Planet, unsigned> FleetsUsed() const
	{
		return fleetsUsed;
	}
	Planet PlanetOne() const
	{
		return fleetsUsed.begin()->first;
	}
	unsigned PlanetOneShips() const
	{
		return fleetsUsed.begin()->second;
	}

	unsigned fleetsTotalSent() const
	{
		typedef std::map<Planet, unsigned>::const_iterator puIter;

		unsigned count = 0;
		for(puIter it = fleetsUsed.begin(); it != fleetsUsed.end(); ++it)
			count += it->second;

		return count;
	}

	void IssueOrders(const PlanetWars& pw) const
	{
		debugMove();
		for(std::map<Planet, unsigned>::const_iterator it = fleetsUsed.begin();
					it != fleetsUsed.end(); ++it)
			pw.IssueOrder(it->first.PlanetID(), destination.PlanetID()
									, it->second);
	}

	void debugMove() const
	{
		for(std::map<Planet, unsigned>::const_iterator it = fleetsUsed.begin();
					it != fleetsUsed.end(); ++it)
			debug << "sending [from, to, numships]: ["
						<< it->first.PlanetID() << ", "
						<< destination.PlanetID() << ", "
						<< it->second << "]" << std::endl;
	}

	private:
	Planet destination;
	std::map<Planet, unsigned> fleetsUsed; 
};

bool operator==(const Planet& a, const Planet& b)
{	return a.PlanetID() == b.PlanetID();	}

unsigned getProductionOfPlanets(std::vector<Planet> planets);

bool weAreWinning(unsigned turnsBenefit, const PlanetWars& pw, double modifier);

template <typename T>
int pShipsSum(T begin, T end);

template <typename T>
int pGrowthSum(T begin, T end);

int getPotentialCost
(const Planet& planet, unsigned turnsAway, const PlanetWars& pw);

unsigned xTurnBenefit
(unsigned xTurns, unsigned turnsTillTakeOver, const Planet& planet);

bool numshipsLT(const Fleet& a, const Fleet& b);

bool enemyIsAttacking(const Planet& p, const PlanetWars& pw);

//based on empty cells having -1 in them.
int indexRoundDown(const std::vector<int>& v, unsigned index);

template <typename T> void updatePlanetsByMove
(T begin, T end, const Move& move);

int myTotalShips(const PlanetWars& pw);

Planet getOriginalPlanet(const Planet& p, const PlanetWars& pw);

template <typename T> void debugFleets(const T begin, const T end);

void debugAllFleets(const PlanetWars& pw);

bool destNEQplanet(const Fleet& f, const Planet& p);

bool fDistLT(const Fleet& a, const Fleet& b);

bool idEQ(const Planet& a, const Planet& b);

bool growthGT(const Planet& a, const Planet& b);

bool closerP(const Planet& a, const Planet& b, const Planet& ref);

std::vector<Fleet> fleetsIncomingToPlanet(const Planet& p, const PlanetWars& pw);

unsigned planetTurnDistance(const Planet& p1, const Planet& p2);

int shipsAfterFleetsAttack(const Planet& planet, const PlanetWars& pw);
void shipsAfterFleetsAttack(Planet& planet, const PlanetWars& pw);

int _shipsAfterFleetsAttack(const Planet& planet, const PlanetWars& pw, int& owner, bool maxNotMin = false);

template <typename T>
std::vector<Planet> sortAndIntersect(T begin1, T end1, T begin2, T end2);

bool isOnFringe(const Planet& p, const PlanetWars& pw, const PlanetWars& futurepw);

bool closestToAnEnemy(const Planet& p, const PlanetWars& pw, const PlanetWars& futurepw);

bool closerToEnemyThanFriend(const Planet& p, const PlanetWars& pw);

bool neutralPlanet(const Planet& p);

void debugRealPlanet(const std::string& prefix, const Planet& p);

//gets the growth rate total of a vector of planets
unsigned getProductionOfPlanets(std::vector<Planet> planets)
{
	unsigned total = 0;
	for(std::vector<Planet>::iterator it = planets.begin();
			it != planets.end();
			++it)
		total += it->GrowthRate();

	return total;
}

//TODO: cache
//This should be phased out...
int getPotentialCost(const Planet& planet, unsigned turnsAway, const PlanetWars& pw)
{
	int potentialCost = shipsAfterFleetsAttack(planet, pw);
	if(planet.Owner() == 1)
		potentialCost *= -1;

/*
	int owner = planet.Owner();
	int potentialCost;
	if(owner == 1)
		potentialCost = -planet.NumShips();
	else
		potentialCost = planet.NumShips(); //you need MORE ships, then p
	//debug << "ID("<<planet.PlanetID()<<") potentialCost: " << potentialCost << std::endl;

	//get and order all fleets coming to this planet.
	std::vector<Fleet> incomingFleets = fleetsIncomingToPlanet(planet, pw);
	std::sort(incomingFleets.begin(), incomingFleets.end(), fDistLT);

	//variables necessary in this for loop.
	unsigned growthIndex = 0;
	int bufferAgainst = 0;
	int bufferFor = 0;
	for(std::vector<Fleet>::iterator it = incomingFleets.begin();
			it != incomingFleets.end();
			++it)
	{
		//get growth cost.
		int growth=(it->TurnsRemaining() - growthIndex) * planet.GrowthRate();
		//update growthIndex
		growthIndex = it->TurnsRemaining();
		//debug << "fleet("
		//	<<it->SourcePlanet()<<","<<it->DestinationPlanet()
		//	<<") Owner("<<it->Owner()<<") size("<<it->NumShips()<<") t-("<<it->TurnsRemaining()<<")"<< "~ planet owner: " 
		//	<<owner << std::endl;

		//add growth cost
		if(owner == 1)
			bufferFor += growth;
		else if(owner == 2)
			bufferAgainst += growth;

		//Add incoming fleets to their respective buffers.
		if(it->Owner() == 1)
			bufferFor += it->NumShips();
		else
			bufferAgainst += it->NumShips();


		if((it+1) == incomingFleets.end() 
				|| (it+1)->TurnsRemaining() - growthIndex)
		{
			debugdetails << "we are in the application part of potentialCost" << std::endl;
			debugdetails << "application. bufferFor: " <<bufferFor << ", bufferAgainst: " << bufferAgainst << std::endl;
			//if the planet is simply owned by me or the enemy.
			if(owner != 0)
			{
				int prevPotentialCost = potentialCost;
				//apply the new potentialCost;
				potentialCost += bufferAgainst - bufferFor;

				//if we went from positive to negative, we own the planet now
				if(prevPotentialCost > 0 && potentialCost < 0)
					owner = 1;
				//if we went from negative to positive,enemy owns the planet now
				else if(prevPotentialCost < 0 && potentialCost > 0)
					owner = 2;
			}
			//if the planet is owned by no one (neutral)
			else
			{
				//pay down the neutral cost
				potentialCost 
					= std::max(0, potentialCost - (bufferFor + bufferAgainst));

				//if... then someone is taking over the planet, or clashing to 0
				if(potentialCost < (bufferFor + bufferAgainst))
				{
					if(bufferFor < bufferAgainst)
					{
						owner = 2;
						potentialCost = bufferAgainst - bufferFor;
					}
					else if (bufferFor > bufferAgainst)
					{
						owner = 1;
						potentialCost = bufferAgainst - bufferFor;
					}
				}
			}
			bufferFor = 0;
			bufferAgainst = 0;
		}
	}
	
*/

//HACK TO GET growthIndex!//
//the same code is above... beware before uncommenting above code.//

	//get and order all fleets coming to this planet.
	std::vector<Fleet> incomingFleets = fleetsIncomingToPlanet(planet, pw);
	std::sort(incomingFleets.begin(), incomingFleets.end(), fDistLT);
	int growthIndex = (incomingFleets.begin() != incomingFleets.end())
							?(incomingFleets.end()-1)->TurnsRemaining()	:0;

//////////

	//the remaining growth
	if(turnsAway > growthIndex)
	{
		if(planet.Owner() == 1)
			potentialCost -= (turnsAway - growthIndex) * planet.GrowthRate();
		else if(planet.Owner() == 2)
			potentialCost += (turnsAway - growthIndex) * planet.GrowthRate();
	}

	if(planet.Owner() != 1)
		potentialCost += 1;	//you must OVERTAKE the planet, so add 1

	return potentialCost;
}

unsigned xTurnBenefit(unsigned xTurns, unsigned turnsTillTakeOver
								, const Planet& planet)
{
	//debug << "if positive xturnbenefit ID(" << planet.PlanetID() << "): "
	//	<< (xTurns - turnsTillTakeOver)*planet.GrowthRate();
	if(xTurns > turnsTillTakeOver)
		return (xTurns - turnsTillTakeOver)*planet.GrowthRate();
	else
		return 0;
}

unsigned turnDistance(double x1, double y1, double x2, double y2)
{
	const double turnsPerUnit = 1.0;
	double distance = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));

	return (distance * turnsPerUnit) + 1;
}

//returns the distance between two planets
//I should cache this once i get to more cpu intensive tasks.
unsigned planetTurnDistance(const Planet& p1, const Planet& p2)
{
	//debug << "turnDistance: " << turnDistance(p1.X(), p1.Y(), p2.X(), p2.Y()) << std::endl;
	return turnDistance(p1.X(), p1.Y(), p2.X(), p2.Y());
}

unsigned getCacheIndex(unsigned planetIndex, unsigned fleetSize)
{
	return (planetIndex * 100000) + fleetSize;
}

unsigned twoPowerOfX(unsigned x)
{
	static unsigned powsOf2[32] =
		{0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000,0x8000,0x10000,0x20000,0x40000,0x80000,0x100000,0x200000,0x400000,0x800000,0x1000000,0x2000000,0x4000000,0x8000000,0x10000000,0x20000000,0x40000000,0x80000000};
	return powsOf2[x];
}

template <typename T>
std::vector<Planet> getPlanetsFromBinaryMapping(unsigned bmapping, const T& begin, const T& end)
{
	static unsigned powsOf2[32] =
		{0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000,0x8000,0x10000,0x20000,0x40000,0x80000,0x100000,0x200000,0x400000,0x800000,0x1000000,0x2000000,0x4000000,0x8000000,0x10000000,0x20000000,0x40000000,0x80000000};
	std::vector<Planet> peas;
	unsigned i = 0;
	for(T it = begin; it != end; ++it, ++i)
		if(powsOf2[i] & bmapping)
			peas.push_back(*it);

	return peas;
}

//this takes out planets I'm already planning to own.
//I need this function, because if I'm already going to be owning 
// a planet, then the potentialCost function will return a negative 
// value on the planet, and break my nicestPlanets function.
//DEPRECATED
/*std::vector<Planet> NotGoingToBeMyPlanets(const Planet& refPlanet, const PlanetWars& pw)
{
	std::vector<Planet> peas;
	std::vector<Planet> NMpeas = pw.NotMyPlanets();

	for(std::vector<Planet>::iterator it = NMpeas.begin(); it!=NMpeas.end();
			++it)
		if(getPotentialCost(*it, planetTurnDistance(*it, refPlanet), pw) > 0)
			peas.push_back(*it);

	return peas;
}*/

//CURRENTLY not in use.
//returns whether if a planet is under attack by the enemy
bool enemyIsAttacking(const Planet& p, const PlanetWars& pw)
{
	std::vector<Fleet> enemyFleets = pw.EnemyFleets();
	for(std::vector<Fleet>::iterator it = enemyFleets.begin();
			it != enemyFleets.end(); ++it)
		if(it->DestinationPlanet() == p.PlanetID())
			return true;

	return false;
}

void debugPotentialWorthAndCost(const Planet& p, const PlanetWars& pw)
{
	typedef std::vector<Planet> Vec;
	typedef Vec::iterator iter;
	Vec otherPlanets = pw.NotMyPlanets();
	debug << "Planet ID\t12 turn worth\tpotentialCost from ID("<<p.PlanetID()<<")" << std::endl;
	debug << "---------\t-------------\t-------------" << std::endl;
	for(iter it = otherPlanets.begin(); it!=otherPlanets.end(); ++it)
	{
		debug << it->PlanetID() << "\t\t\t\t" 
			<< xTurnBenefit(12, planetTurnDistance(*it, p), *it) << "\t\t\t\t" 
			<< getPotentialCost(*it, planetTurnDistance(*it, p), pw)
			<< std::endl;
	}
	debug << "---" << std::endl;
	debug << "---" << std::endl;
}

void debugMyPlanets(const PlanetWars& pw)
{
	typedef std::vector<Planet> Vec;
	typedef Vec::iterator iter;
	Vec otherPlanets = pw.MyPlanets();
	debug << "Planet ID\tNumShips" << std::endl;
	debug << "---------\t--------" << std::endl;
	for(iter it = otherPlanets.begin(); it!=otherPlanets.end(); ++it)
	{
		debug << it->PlanetID() << "\t\t\t\t" 
			<< it->NumShips()
			<< std::endl;
	}
	debug << "---" << std::endl;
	debug << "---" << std::endl;
}

void debugPlanets(const PlanetWars& pw)
{
	typedef std::vector<Planet> Vec;
	typedef Vec::iterator iter;
	Vec otherPlanets = pw.Planets();
	debug << "Planet ID\tOwner\tNumShips" << std::endl;
	debug << "---------\t-----\t--------" << std::endl;
	for(iter it = otherPlanets.begin(); it!=otherPlanets.end(); ++it)
	{
		debug << it->PlanetID() << "\t\t\t\t" 
			<<it->Owner() << "\t\t\t"
			<< it->NumShips()
			<< std::endl;
	}
	debug << "---" << std::endl;
	debug << "---" << std::endl;
}

//this function alters the planets in the vector.
void afterFleetsAttackMyPlanets(PlanetWars& futurepw)
{
	typedef std::vector<Planet> pVec;

	pVec allPeas = futurepw.Planets();

	for(pVec::iterator it = allPeas.begin(); it != allPeas.end(); ++it)
		shipsAfterFleetsAttack(*it, futurepw);

	futurepw.Planets(allPeas);
}
bool numshipsLT(const Fleet& a, const Fleet& b)
{	return a.NumShips() < b.NumShips();	}
//used above. Depends on blank entries having -1.
int indexRoundDown(const std::vector<int>& v, unsigned index)
{
	++index;
	while(v[--index] == -1);
	return v[index];
}

//just gets new the ships size using the predicting style
int shipsAfterFleetsAttack(const Planet& planet, const PlanetWars& pw)
{
	int useless;
	return _shipsAfterFleetsAttack(planet, pw, useless, true);
}
//gets the new planet size and updates the planet owner.
void shipsAfterFleetsAttack(Planet& planet, const PlanetWars& pw)
{
	int useful;
	planet.NumShips(_shipsAfterFleetsAttack(planet, pw, useful));
	planet.Owner(useful);
}
int _shipsAfterFleetsAttack(const Planet& planet, const PlanetWars& pw, int& owner, bool maxNotMin)
{	
	owner = planet.Owner();
	int numShips = planet.NumShips();
	debugdetails << "_shipsAfterFleetsAttack ID("<< planet.PlanetID()
		<<"): start ships: " << numShips << std::endl;

	//get and order all fleets coming to this planet.
	std::vector<Fleet> incomingFleets = fleetsIncomingToPlanet(planet, pw);
	std::sort(incomingFleets.begin(), incomingFleets.end(), fDistLT);

	//variables necessary in this for loop.
	unsigned growthIndex = 0;
	int bufferAgainst = 0;
	int bufferFor = 0;
	int bufferMy = 0;
	int bufferEnemy = 0;
	int minimum = numShips;
	for(std::vector<Fleet>::iterator it = incomingFleets.begin();
			it != incomingFleets.end();
			++it)
	{
		//get growth cost.
		int growth=(it->TurnsRemaining() - growthIndex) * planet.GrowthRate();
		//update growthIndex
		growthIndex = it->TurnsRemaining();

		//add growth cost as long as the planet isn't neutral.
		if(owner != 0)
		{
			bufferFor += growth;
			if(owner == 1)
				bufferMy += growth;
			else
				bufferEnemy += growth;
		}

		//Add incoming fleets to their respective for and against buffers..
		if(it->Owner() == owner)
			bufferFor += it->NumShips();
		else
			bufferAgainst += it->NumShips();
		//add incoming fleets to their respect owner buffers.
		if(it->Owner() == 1)
			bufferMy += it->NumShips();
		else
			bufferEnemy += it->NumShips();


		if((it+1) == incomingFleets.end() 
				|| (it+1)->TurnsRemaining() - growthIndex)
		{
			//if, then the planet will change owners.
			if(numShips + bufferFor < bufferAgainst)
			{
				if(owner == 1)
					owner = 2;
				else if(owner == 2)
					owner = 1;
				else	//neutral
				{
					if(bufferMy > bufferEnemy)
						owner = 1;
					else if(bufferEnemy > bufferMy)
						owner = 2;
					//else they clash, and the planet remains neutral.
			
					numShips = abs(bufferMy - bufferEnemy);
				}
			}
			else if(owner == 0)
			{
				if(bufferMy > bufferEnemy)
					numShips -= bufferMy;
				else
					numShips -= bufferEnemy;
			}

			//abs, so if we overtake a planet, then the negative value, is
			//the current number of ships, so just take away the negative.
			if(owner != 0) //neutral sets numShips a different way. See above
				numShips += abs(bufferFor - bufferAgainst);

			if(numShips < minimum)
				minimum = numShips;
				
			bufferFor = 0;
			bufferAgainst = 0;
		}
	}

	if(maxNotMin || minimum == 999999)
		return numShips;
	else
		return minimum;
}

void updatePlanetsByMove(PlanetWars& futurepw, const Move& move)
{
	std::vector<Planet> peas = futurepw.Planets();

	typedef std::map<Planet, unsigned> puMap;
	typedef puMap::iterator puIter;
	typedef std::vector<Planet>::iterator pIter;
	puMap fleetsUsed = move.FleetsUsed();

	std::vector<Fleet> fleets;
	for(puIter it = fleetsUsed.begin(); it != fleetsUsed.end(); ++it)
	{
		//take away ships from the planets we are sending from.
		pIter pea = std::find(peas.begin(), peas.end(), it->first);
		//min 0 is a quick hack to stop the crashing.
		pea->NumShips(std::max(0, pea->NumShips() - (int)it->second));

		//add the fleets that will be straight to futurepw.
		int turnDistance = planetTurnDistance(it->first, move.Destination());
		fleets.push_back(Fleet(1, (int)it->second, it->first.PlanetID()
								, move.Destination().PlanetID()
								, turnDistance, turnDistance));
	}

	futurepw.Planets(peas);
	futurepw.AddFleets(fleets);
}

template <typename T>
//I know this isn't the most effecient method, but effeciency is yet necessary.
T pShipsMax(T begin, T end)
{
	int max = -99999;
	T best = end;
	for(T it = begin; it != end; ++it)
		if(it->NumShips() > max)
			max = (best = it)->NumShips();

	return best;
}
template <typename T>
int pShipsSum(T begin, T end)
{
	int sum = 0;

	for(T it = begin; it != end; ++it)
		sum += it->NumShips();

	return sum;
}

template <typename T>
int pGrowthSum(T begin, T end)
{
	int sum = 0;

	for(T it = begin; it != end; ++it)
		sum += it->GrowthRate();

	return sum;
}

int myTotalShips(const PlanetWars& pw)
{
	int sum = 0;
	std::vector<Planet> mypeas = pw.MyPlanets();
	for(std::vector<Planet>::iterator it = mypeas.begin();
			it != mypeas.end();
			++it)
		sum += it->NumShips();

	return sum;
}

Planet getOriginalPlanet(const Planet& p, const PlanetWars& pw)
{
		std::vector<Planet> peas = pw.Planets();
		return *(std::find_if(peas.begin(),
										peas.end(),
										tr1boost::bind(idEQ, _1, p)));
}

void debugAllFleets(const PlanetWars& pw)
{
	typedef std::vector<Fleet> Vec;
	Vec otherPlanets = pw.Fleets();
	
	debugFleets(otherPlanets.begin(), otherPlanets.end());
}

template <typename T>
void debugFleets(const T begin, const T end)
{
	debug << "Owner\tTo\tTurns\tNumShips" << std::endl;
	debug << "-----\t--\t-----\t--------" << std::endl;
	for(T it = begin; it!=end; ++it)
	{
		debug << it->Owner() << "\t\t" 
			<< it->DestinationPlanet() << "\t"
			<< it->TurnsRemaining() << "\t\t"
			<< it->NumShips()
			<< std::endl;
	}
	debug << "---" << std::endl;
	debug << "---" << std::endl;
}

void setStartingAverageDistance(const PlanetWars& pw)
{
	unsigned count = 0;
	unsigned sum = 0;
	std::vector<Planet> otherPlanets = pw.NotMyPlanets();
	Planet first = pw.MyPlanets()[0];

	for(std::vector<Planet>::iterator it = otherPlanets.begin();
				it != otherPlanets.end(); ++it, ++count)
		sum += planetTurnDistance(first, *it);

	//so this way the planets in the middle get counted as if their were 3 turns for them to accumulate.
	startingAverageDistance = (sum / count);
}	
bool destNEQplanet(const Fleet& f, const Planet& p)
{	return f.DestinationPlanet() != p.PlanetID();	}
bool fDistLT(const Fleet& a, const Fleet& b)
{	return a.TurnsRemaining() < b.TurnsRemaining();	}
bool idEQ(const Planet& a, const Planet& b)
{	return a.PlanetID() == b.PlanetID();	}
bool growthGT(const Planet& a, const Planet& b)
{	return a.GrowthRate() > b.GrowthRate();	}
bool closerP(const Planet& a, const Planet& b, const Planet& ref)
{	return planetTurnDistance(a, ref) <= planetTurnDistance(b, ref);	}

std::vector<Fleet> fleetsIncomingToPlanet(const Planet& p, const PlanetWars& pw)
{
	using namespace tr1boost::placeholders;
	std::vector<Fleet> incomingFleets = pw.Fleets();
	std::vector<Fleet>::iterator fend
		= std::remove_if(incomingFleets.begin(), incomingFleets.end()
								, tr1boost::bind(destNEQplanet, _1, p));
	incomingFleets.erase(fend, incomingFleets.end());

	return incomingFleets;
}

bool weAreWinning(unsigned turnsBenefit, const PlanetWars& pw, double modifier)
{
	std::vector<Planet> myPlanets = pw.MyPlanets();
	std::vector<Planet> enemyPlanets = pw.MyPlanets();

	int myScore = pShipsSum(myPlanets.begin(), myPlanets.end())
						+ pGrowthSum(myPlanets.begin(), myPlanets.end())
							* turnsBenefit;
	int enemyScore = pShipsSum(enemyPlanets.begin(), enemyPlanets.end())
							+ pGrowthSum(enemyPlanets.begin(), enemyPlanets.end())
								* turnsBenefit;

	debugdetails << "weAreWinning. myScore: " <<myScore << ", enemyScore: " << enemyScore << std::endl;
	return (modifier * myScore) > enemyScore;
}

unsigned hackedMoveTurnsAway(const Move& a)
{
	return planetTurnDistance(a.PlanetOne(), a.Destination());
}

std::vector<Move> mapValues(std::map<int, Move>::iterator begin
										, std::map<int, Move>::iterator end)
{
	std::vector<Move> result;
	for(std::map<int, Move>::iterator it = begin; it != end; ++it)
		result.push_back(it->second);

	return result;
}

int secondMax(int a, int b, int c)
{
	if(a < b)
		if(b < c)
			return b;

	if(b < a)
		if(a < c)
			return a;

	return c;
}

std::vector<Planet> safeMyPlanets(const PlanetWars& cur
												, const PlanetWars& fut)
{
	typedef std::vector<Planet> pvec;
	pvec myplanets1 = cur.MyPlanets();
	pvec myplanets2 = fut.MyPlanets();

	return sortAndIntersect(myplanets1.begin(), myplanets1.end(),
										myplanets2.begin(), myplanets2.end());
}

std::vector<Planet> safeNotMyPlanets(const PlanetWars& cur
												, const PlanetWars& fut)
{
	typedef std::vector<Planet> pvec;
	pvec myplanets1 = cur.NotMyPlanets();
	pvec myplanets2 = fut.NotMyPlanets();

	return sortAndIntersect(myplanets1.begin(), myplanets1.end(),
										myplanets2.begin(), myplanets2.end());
}

template <typename T>
std::vector<Planet> sortAndIntersect(T begin1, T end1, T begin2, T end2)
{
	typedef std::vector<Planet> pvec;
	typedef pvec::iterator pit;
	std::sort(begin1, end1);
	std::sort(begin2, end2);

	pvec result(50);
	pit fend = std::set_intersection(begin1, end1, begin2, end2, result.begin());
	result.erase(fend, result.end());

	//T end = myintersection(begin1, end1, begin2, end2, begin);

	return result;
}

template <typename T>
int iteratorCount(T begin, T end)
{
	int count = 0;
	for(T it = begin; it != end; ++it, ++count);
	return count;
}

//my intersection mangles up the second container... keep in mind.
template <typename T>
T myintersection(T begin1, T end1, T begin2, T end2, T result)
{
	T it = begin1;
	while(it != end1)
	{
		result = std::remove(begin2, end2, *it);
		++it;
	}

	return end2;
}

std::vector<Planet> mySetDifference(const std::vector<Planet>& first, const std::vector<Planet>& second)
{
debugdetails << "first size: " << first.size() << std::endl;	
	std::vector<Planet> result(first.size());
	std::vector<Planet>::iterator fend = std::set_difference(first.begin(), first.end()
								, second.begin(), second.end(), result.begin());
debugdetails << "past set_different..." << std::endl;
	result.erase(fend, result.end());
debugdetails << "result size: " << result.size() << std::endl;	
	return result;
}

std::vector<Planet> getNotFringePlanets(const PlanetWars& pw, const PlanetWars& futurepw)
{
	using namespace tr1boost::placeholders;
	std::vector<Planet> mypeas = safeMyPlanets(pw, futurepw);

	std::vector<Planet>::iterator fend
		= std::remove_if(mypeas.begin(), mypeas.end()
			, tr1boost::bind(isOnFringe, _1, pw, futurepw));
	mypeas.erase(fend, mypeas.end());
	return mypeas;
}

bool isOnFringe(const Planet& p, const PlanetWars& pw, const PlanetWars& futurepw)
{
	if(!closestToAnEnemy(p, pw, futurepw) /*|| closerToEnemyThanFriend(p, futurepw)*/)
			debugdetails << "IS NOT! ON THE FRINGE!" << std::endl;
	return closestToAnEnemy(p, pw, futurepw);// || closerToEnemyThanFriend(p, futurepw);
}
bool closestToAnEnemy(const Planet& p, const PlanetWars& pw, const PlanetWars& futurepw)
{
	using namespace tr1boost::placeholders;
	std::vector<Planet> mypeas = safeMyPlanets(pw, futurepw);
	debugdetails << "closestToAnEnemy ID("<<p.PlanetID()<<"): mysafepeas.size: "<<mypeas.size() << std::endl;
	for(std::vector<Planet>::iterator it = mypeas.begin(); it != mypeas.end()
			  ; ++it)
		debugdetails << "mysafepeas [id, numships]: [" << it->PlanetID() << ", " << it->NumShips() << "]" << std::endl;
	std::vector<Planet> enemyPeas = futurepw.EnemyPlanets();
	debugdetails << "closestToAnEnemy ID("<<p.PlanetID()<<"): enemyPeas.size: "<<enemyPeas.size() << std::endl;

	typedef std::vector<Planet>::iterator pIter;
	for(pIter it = enemyPeas.begin(); it != enemyPeas.end(); ++it)
	{
		debugdetails << "minimum distance: " << std::min_element(mypeas.begin(), mypeas.end(), tr1boost::bind(closerP, _1, _2, *it))->PlanetID() << std::endl;

		if(std::min_element(mypeas.begin(), mypeas.end()
					, tr1boost::bind(closerP, _1, _2, *it))->PlanetID()
				== p.PlanetID())
			return true;
	}

	return false;
}
bool closerToEnemyThanFriend(const Planet& p, const PlanetWars& pw)
{
	using namespace tr1boost::placeholders;
	std::vector<Planet> peas = pw.Planets();

	//the closest planet, not including neutral planets.
	return 2 == std::min_element(peas.begin(), peas.end()
			, tr1boost::bind(closerP, _1, _2, p) 
				&& !tr1boost::bind(neutralPlanet, _1))->Owner();
}
bool neutralPlanet(const Planet& p)
{
	return p.Owner() == 0;
}

void debugRealPlanet(const std::string& prefix, const Planet& p)
{
	const Planet realp = getOriginalPlanet(p, globalpw);
	debug << prefix << "[ID, NumShips]: " << "["<<realp.PlanetID()<<", "<<realp.NumShips()<<"]" << std::endl;
}

#endif
