#include <iostream>

#include <boost/progress.hpp>

#include "globals.h"
#include "PlanetWars.h"
#include "GeneralTools.h"
#include "AITools.h"

//NOTES:
// 200 turns is apparently the current maximum numnber of turns
// 1 second per turn... hard to tell how much this would be on their servers

void DoTurn(const PlanetWars& pw) {
	boost::progress_timer pt(debug);
	if(totalTurnsPassed==1)debugPotentialWorthAndCost(pw.MyPlanets()[0], pw);
	debugMyPlanets(pw);

	//used by functions grabbing worth.
	setStartingAverageDistance(pw);

	std::vector<Planet> myplanets = pw.MyPlanets();
	//estimate of my current planets after all attacks.
	//TODO: reconsider this.
	afterFleetsAttackMyPlanets(myplanets.begin(), myplanets.end(), pw);

	//get defensive moves
	std::vector<Move> dMoves 
		= getDefenceMoves(myplanets.begin(), myplanets.end(), pw);

	Planet currentPlanet = *(myplanets.begin());

	//attack
	std::vector<Planet>::iterator bestit;
	//We take the best planet if it's 3 times the average size
	//this is meant to combat the effect that we have a planet not being attacked accumulating, and it rarely attacks. It will attack much more often.
	int averageShips = pShipsSum(myplanets.begin(), myplanets.end()) / myplanets.size();
	if((bestit = pShipsMax(myplanets.begin(), myplanets.end()))->NumShips()*3
			  > averageShips)
		currentPlanet = *bestit;
	else
	{
		unsigned currentIndex = totalTurnsPassed % pw.MyPlanets().size();
		//Less Simple Defence. (if we have no ships to attack with after 
		//	estimates, then don't use this planet)
		unsigned count = 0;
		bool noattack = false;
		//less than a third of the average, means this planet has too few of ships.
		while(myplanets[currentIndex].NumShips() <= (averageShips/3)
				  	&& ++count <= pw.MyPlanets().size())
		{
				  debug << "skipping a planet, cause it's too low with ships" << std::endl;
			currentIndex = (currentIndex+1) % pw.MyPlanets().size();
		}
		//
		if(myplanets[currentIndex].NumShips() > (averageShips/3)
				  	|| ++count > pw.MyPlanets().size())
		{
			debug << "No planets attacking this turn..." << std::endl;
			return;
		}

		//we are attacking with THIS planet. Using the original NumShips.
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
	debug.open("bots/debug/cleanedUp.log");
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
