#include <iostream>

#include <boost/progress.hpp>

#include "globals.h"
#include "PlanetWars.h"
#include "GeneralTools.h"
#include "AITools.h"

// The DoTurn function is where your code goes. The PlanetWars object contains
// the state of the game, including information about all planets and fleets
// that currently exist. Inside this function, you issue orders using the
// pw.IssueOrder() function. For example, to send 10 ships from planet 3 to
// planet 8, you would say pw.IssueOrder(3, 8, 10).
//
// There is already a basic strategy in place here. You can use it as a
// starting point, or you can throw it out entirely and replace it with your
// own. Check out the tutorials and articles on the contest website at
// http://www.ai-contest.com/resources.

//NOTES:
// 200 turns is apparently the current maximum numnber of turns
// 1 second per turn... hard to tell how much this would be on their servers.

void DoTurn(const PlanetWars& pw) {
	boost::progress_timer pt(debug);
	if(totalTurnsPassed == 1)
		debugPotentialWorthAndCost(pw.MyPlanets()[0], pw);
	debugMyPlanets(pw);

	//gt the starting average distance so that long distance loss doesn't happen again =(.
	setStartingAverageDistance(pw);

	std::vector<Planet> myplanets = pw.MyPlanets();
	//estimate of my current planets after all attacks.
	afterFleetsAttackMyPlanets(myplanets.begin(), myplanets.end(), pw);

	//Better Defence
	std::vector<Move> dMoves 
		= getDefenceMoves(myplanets.begin(), myplanets.end(), pw);

	Planet currentPlanet = *(myplanets.begin());

	//slightly less simple Attack
	std::vector<Planet>::iterator bestit;
	//We take the best planet if it's 3 times the average size
	//this is meant to combat the effect that we have a planet not being attacked accumulating, and it rarely attacks. It will attack much more often.
	if((bestit = pShipsMax(myplanets.begin(), myplanets.end()))->NumShips()*3
			  > pShipsSum(myplanets.begin(), myplanets.end()) / myplanets.size())
		currentPlanet = *bestit;
	else
	{
		unsigned currentIndex = totalTurnsPassed % pw.MyPlanets().size();
		//Less Simple Defence. (if we have no ships to attack with after 
		//	estimates, then don't use this planet)
		unsigned count = 0;
		bool noattack = false;
		//less than 10, this way i don't murder my self so easily.
		while(myplanets[currentIndex].NumShips() <= 10
				  	&& ++count <= pw.MyPlanets().size())
		{
				  debug << "skipping a planet, cause it's too low with ships" << std::endl;
				  noattack = true;
			currentIndex = (currentIndex+1) % pw.MyPlanets().size();
		}
		//
		if(noattack)
		{
			debug << "No planets attacking this turn..." << std::endl;
			return;
		}

		currentPlanet = pw.MyPlanets()[currentIndex];
	}

	debug << "Attacking with planet " << currentPlanet.PlanetID()
		<< ", with " << currentPlanet.NumShips() << " ships: " <<std::endl;

	std::vector<Planet> notMyPlanets = NotGoingToBeMyPlanets(currentPlanet, pw);
	std::vector<Planet> peas = nicestPlanets
			(currentPlanet, notMyPlanets.begin()
				, notMyPlanets.end(), pw)();

	std::vector<Move> oMoves;
	for(std::vector<Planet>::iterator it = peas.begin(); it!=peas.end(); ++it)
	{
		debug << "planet id: " << it->PlanetID() << std::endl;
		debug << "planet ships sent: " << getPotentialCost(*it, planetTurnDistance(*it, currentPlanet),pw) << std::endl;

		oMoves.push_back(
		Move(currentPlanet 
		  , getPotentialCost(*it, planetTurnDistance(*it, currentPlanet),pw)
		  , *it));
	}

	debug << "Issueing orders!!" << std::endl;
	typedef std::vector<Move>::iterator mit;
	for(mit it = dMoves.begin(); it != dMoves.end(); ++it)
		it->IssueOrders(pw);
	for(mit it = oMoves.begin(); it != oMoves.end(); ++it)
		it->IssueOrders(pw);
	
	debug << "Done attack." << std::endl;
}

// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
int main(int argc, char *argv[]) {
	debug.close();
	debug.open("bots/debug/maxRouletteDef.log");
  std::string current_line;
  std::string map_data;

  while (true) {
    int c = std::cin.get();
    current_line += (char)c;
    if (c == '\n') {
      if (current_line.length() >= 2 && current_line.substr(0, 2) == "go") {
        PlanetWars pw(map_data);
        map_data = "";
		  //print total turns thus far
			++totalTurnsPassed;
	 		 debug << "Turn " << totalTurnsPassed << ": " << std::endl;
		//the heart. Actually run the turn.
        DoTurn(pw);
			pw.FinishTurn();
      } else {
        map_data += current_line;
      }
      current_line = "";
    }
  }
  debug.close();
  return 0;
}
