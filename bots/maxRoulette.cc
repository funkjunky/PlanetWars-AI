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

	unsigned currentIndex = totalTurnsPassed % pw.MyPlanets().size();
	std::vector<Planet> myplanets = pw.MyPlanets();

	//Simple Defence
	unsigned count = 0;
	bool noattack = false;
	debug << "fucking numb of planets: " << pw.MyPlanets().size() << std::endl;
	while(enemyIsAttacking(myplanets[currentIndex], pw) 
			  	&& ++count <= pw.MyPlanets().size())
	{
			  debug << "skip&ping a planet, cause it's being attacked" << std::endl;
			  noattack = true;
		currentIndex = (currentIndex+1) % pw.MyPlanets().size();
	}
	//
	if(noattack)
	{
		debug << "No planets attacking this turn..." << std::endl;
		return;
	}
	else
		debug << "Planet " << myplanets[currentIndex].PlanetID() 
			<< " is attacking this turn..." << std::endl;


	Planet currentPlanet = pw.MyPlanets()[currentIndex];

	debug << "Attacking with planet " << currentPlanet.PlanetID()
		<< ", with " << currentPlanet.NumShips() << " ships: " <<std::endl;

	std::vector<Planet> notMyPlanets = NotGoingToBeMyPlanets(currentPlanet, pw);
	std::vector<Planet> peas = nicestPlanets
			(currentPlanet, notMyPlanets.begin()
				, notMyPlanets.end(), pw);

	for(std::vector<Planet>::iterator it = peas.begin(); it!=peas.end(); ++it)
	{
		debug << "planet id: " << it->PlanetID() << std::endl;
		debug << "planet numships: " << it->NumShips() << std::endl;
	   pw.IssueOrder(currentPlanet.PlanetID(), it->PlanetID()
									, getPotentialCost(*it, planetTurnDistance(*it, currentPlanet),pw));
	}
	debug << "Done attack." << std::endl;
}

// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
int main(int argc, char *argv[]) {
	debug.close();
	debug.open("bots/debug/maxRoulette.log");
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
