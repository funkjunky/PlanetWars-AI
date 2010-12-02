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
	if(totalTurnsPassed == 1)
	{
		debugPotentialWorthAndCost(pw.MyPlanets()[0], pw);

		std::vector<Planet> bleh = pw.NotMyPlanets();
		std::vector<Planet> peas = nicestPlanets
			(pw.MyPlanets()[0], bleh.begin()
				, bleh.end(), pw);
			debug << "done recursion." << peas.begin()[0].NumShips() << std::endl;

		for(std::vector<Planet>::iterator it = peas.begin(); it!=peas.end(); ++it)
		{
			debug << "planet id: " << it->PlanetID() << std::endl;
			debug << "planet numships: " << it->NumShips() << std::endl;
	   	pw.IssueOrder(pw.MyPlanets()[0].PlanetID(), it->PlanetID()
									, it->NumShips()+1);
		}
		debug << "done." << std::endl;
	}
	else
	{
		boost::progress_timer pt(debug);
	  // (1) If we currently have a fleet in flight, just do nothing.
	  int numFleets = 0;
	  if(getProductionOfPlanets(pw.MyPlanets()) 
				 >= getProductionOfPlanets(pw.EnemyPlanets())*2)
			numFleets = 1;
		else if(getProductionOfPlanets(pw.MyPlanets()) 
				 >= getProductionOfPlanets(pw.EnemyPlanets())*1.5)
			numFleets = 3;
		else if(getProductionOfPlanets(pw.MyPlanets()) 
				 >= getProductionOfPlanets(pw.EnemyPlanets()))
			numFleets = 4;
		else
			numFleets = 6; 
	
	  if (pw.MyFleets().size() >= numFleets) {
	    return;
	  }
	  // (2) Find my strongest planet.
	  int source = -1;
	  double source_score = -999999.0;
	  int source_num_ships = 0;
	  std::vector<Planet> my_planets = pw.MyPlanets();
	  for (int i = 0; i < my_planets.size(); ++i) {
	    const Planet& p = my_planets[i];
	    double score = (double)p.NumShips() / (1 + p.GrowthRate());
	    if (score > source_score) {
	      source_score = score;
	      source = p.PlanetID();
	      source_num_ships = p.NumShips();
 	   }
	  }
	  // (3) Find the weakest enemy or neutral planet.
	  int dest = -1;
	  double dest_score = -999999.0;
	  std::vector<Planet> not_my_planets = pw.NotMyPlanets();
	  for (int i = 0; i < not_my_planets.size(); ++i) {
	    const Planet& p = not_my_planets[i];
	    double score = (double)(1 + p.GrowthRate()) / p.NumShips();
	    if (score > dest_score) {
	      dest_score = score;
	      dest = p.PlanetID();
	    }
	  }
	  // (4) Send half the ships from my strongest planet to the weakest
	  // planet that I do not own.
	  if (source >= 0 && dest >= 0) {
	    int num_ships = source_num_ships / 2;
	    pw.IssueOrder(source, dest, num_ships);
	  }
	}
}

// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
int main(int argc, char *argv[]) {
	debug.close();
	debug.open("bots/debug/onlyFirstTurn.log");
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
