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
	debugPlanets(pw);
	debugAllFleetsAndCosts(pw);
	debug << "This is Turn " << totalTurnsPassed << "!" << std::endl;
	debugdetails << "This is Turn " << totalTurnsPassed << "!" << std::endl;

	//used by functions grabbing worth.
	setStartingAverageDistance(pw);

	//estimate of my current planets after all attacks.
	//TODO: reconsider this.
	PlanetWars futurepw(pw); 
	PlanetWars currentpw(pw); 
	afterFleetsAttackMyPlanets(futurepw);
	debug << "afterFleetsAttack... debugPlanets:"<<std::endl;
	debugPlanets(futurepw);

	//get defensive moves
	std::vector<Move> dMoves 
		= getDefenceMoves(currentpw, futurepw);
	
	debug << "before getRefinforcements... like wtf...:" << std::endl;
	debugPlanets(futurepw);
	//get reinforcement moves
	std::vector<Move> rMoves
		= getReinforcementMoves(currentpw, futurepw);
	debug << "after getRefinforcements... GRRRRR:" << std::endl;
	debugPlanets(futurepw);

	//attack
	std::vector<Planet> myplanets = futurepw.MyPlanets();
	std::vector<Planet> safemyplanets = safeMyPlanets(currentpw, futurepw);

	std::vector<Planet> notMyPlanets = futurepw.NotMyPlanets();
	std::vector<Planet> safenotmyplanets = safeNotMyPlanets(currentpw, futurepw);
	std::vector<Move> oMoves = closestOfOptimalMoves(safemyplanets.begin()
		, safemyplanets.end(), safenotmyplanets.begin(), safenotmyplanets.end(), currentpw);

	typedef std::vector<Move>::iterator mit;
	debug << "Issueing defensive orders!!" << std::endl;
	for(mit it = dMoves.begin(); it != dMoves.end(); ++it)
		it->IssueOrders(currentpw);
	debug << "Issueing reinforcement orders!!" << std::endl;
	for(mit it = rMoves.begin(); it != rMoves.end(); ++it)
		it->IssueOrders(currentpw);
	debug << "Issueing attack orders!!" << std::endl;
	for(mit it = oMoves.begin(); it != oMoves.end(); ++it)
		it->IssueOrders(currentpw);
	
	debug << "Done attack." << std::endl;
}

// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
int main(int argc, char *argv[]) {
	debug.close();
	debug.open("bots/debug/reinforcements.log");
	debugdetails.close();
	debugdetails.open("bots/debug_details/reinforcements.log");
  std::string current_line;
  std::string map_data;

  while (true) {
    int c = std::cin.get();
    current_line += (char)c;
    if (c == '\n') {
      if (current_line.length() >= 2 && current_line.substr(0, 2) == "go") {
        PlanetWars pw(map_data);
		  globalpw = pw;
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
  debugdetails.close();
  return 0;
}
