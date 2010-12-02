import java.io.IOException;
import java.util.*;
import java.util.logging.*;

/**
 * Strategy ideas:
 * 
 * - evaluate each planet: owned by self, boost for defence owned by other,
 * attack to take away resources and gain them for self unowned, attack to gain
 * resources 
 *   - distance: farther away means it takes longer to get there father
 *               away from your opponent means it will take them longer to 
 *               attack you 
 *   - size: larger planets produce ships faster 
 *   - defense: for non-owned planets, higher numbers will mean more losses 
 *              for owned planets, lower numbers will mean it is vulnerable
 * 
 * - be sure to include ships en-route to a planet in its defence
 * 
 * - based on these evaluations, choose the weakest planet and attack/reinforce
 * it with ships from the strongest planet - should there be a threshold for the
 * strongest planet, to prevent it from becoming too weak?
 * 
 * @author Randy J. Fortier (aka SeniorArbitrary)
 * 
 */

public class MyBot {
	public static Logger logger;

	static {
		Handler fh = null;
		try {
			fh = new FileHandler("%t/MyBot.log");
		} catch (SecurityException e) {
		} catch (IOException e) {
		}
	    logger = Logger.getLogger("MyBot");
	    fh.setFormatter(new SuperSimpleFormatter());
	    logger.addHandler(fh);
	}
	
	public static void DoTurn(PlanetWars pw) {
		// the number of fleets should be proportional to the number of owned planets
/*
		int numFleets = pw.MyPlanets().size() * 2;
		if (pw.MyFleets().size() >= numFleets) {
			return;
		}
*/
		
		MyBot.chooseBestMove(pw);
	}

	public static void log(String msg) {
		if (DEBUG)
			logger.info(msg);
	}
	
	public static void main(String[] args) {
		String line = "";
		String message = "";
		int c;
		try {
			while ((c = System.in.read()) >= 0) {
				switch (c) {
				case '\n':
					if (line.equals("go")) {
						PlanetWars pw = new PlanetWars(message);
						DoTurn(pw);
						pw.FinishTurn();
						message = "";
					} else {
						message += line + "\n";
					}
					line = "";
					break;
				default:
					line += (char) c;
					break;
				}
			}
		} catch (Exception e) {
		}
	}

	// player IDs
	public static final int NOBODY = 0;
	public static final int ENEMY = 2;
	public static final int ME = 1;
	
	public static final float SOURCE_GROWTH_WEIGHT = 0.1f;
	public static final float SOURCE_FORCE_WEIGHT = 0.05f;
	public static final float TARGET_GROWTH_WEIGHT = 0.30f;
	public static final float TARGET_FORCE_WEIGHT = 0.35f;
	public static final float TARGET_OWNERSHIP_WEIGHT = 0.10f;
	public static final float DISTANCE_WEIGHT = 0.1f;

	public static boolean DEBUG = false;
	
	private static void printSource(PlanetWars pw, Planet s, int maxGrowth, int maxForce) {
		if (DEBUG) {
			float sourceGrowth = getSourceGrowthValue(s, maxGrowth)  * SOURCE_GROWTH_WEIGHT;
			float sourceForce = getSourceForceValue(pw, s, maxForce) * SOURCE_FORCE_WEIGHT;

			log("");
			log("Source data (ID: "+s.PlanetID()+"):");
			log("\tGrowth: "+s.GrowthRate() + " (value: "+sourceGrowth+")");
			log("\tForce:  "+s.NumShips()   + " (value: "+sourceForce+")");
		}
	}

	private static void printMaximums(int maxGrowth, int maxForce, float maxDistance) {
		if (DEBUG) {
			log("");
			log("Maximums:");
			log("\tmax growth:   "+maxGrowth);
			log("\tmax force:    "+maxForce);
			log("\tmax distance: "+maxDistance);
		}
	}
	
	private static void printTarget(Planet s, Planet t) {
		if (DEBUG) {
			log("");
			log("Target data (ID: "+t.PlanetID()+"):");
			log("\tGrowth:   "+t.GrowthRate());
			log("\tForce:    "+t.NumShips());
			String owner = "ME";
			if (t.Owner() == ENEMY)
				owner = "ENEMY";
			else if (t.Owner() == NOBODY)
				owner = "NOBODY";
			log("\tOwner:    "+owner);
			log("\tDistance: "+getDistanceBetween(s, t));
		}
	}

	private static void printEval(PlanetWars pw, Planet s, Planet t, int maxGrowth, int maxForce, float maxDistance) {
		if (DEBUG) {
			float sourceGrowth = getSourceGrowthValue(s, maxGrowth);
			float sourceForce = getSourceForceValue(pw, s, maxForce);
			float targetGrowth = getTargetGrowthValue(t, maxGrowth);
			float targetForce = getTargetForceValue(pw, t, maxForce);
			float targetOwnership = getTargetOwnershipValue(t);
			float distance = getDistanceValue(s, t, maxDistance);
	
			float total = (sourceGrowth * SOURCE_GROWTH_WEIGHT) +
	                      (sourceForce  * SOURCE_FORCE_WEIGHT) +
	                      (targetGrowth * TARGET_GROWTH_WEIGHT) +
	                      (targetForce  * TARGET_FORCE_WEIGHT) +
	                      (targetOwnership * TARGET_OWNERSHIP_WEIGHT) +
	                      (distance * DISTANCE_WEIGHT);
			
			String owner = "ME";
			if (t.Owner() == ENEMY)
				owner = "ENEMY";
			else if (t.Owner() == NOBODY)
				owner = "NOBODY";

			float distanceVal = getDistanceBetween(s, t);
			
			log(" " + t.PlanetID() + "\t" + targetGrowth + " ("+t.GrowthRate()+")\t\t" + targetForce + " ("+t.NumShips()+")\t" + targetOwnership + " ("+owner+")\t" + distance + " ("+distanceVal+")\t" + total);
		}
	}
	
	private static void chooseBestMove(PlanetWars pw) {
		Planet source = null;
		Planet target = null;
		float maxValue = Float.MIN_VALUE;

		int maxGrowth = MyBot.getMaxValue(pw, GROWTH);
		int maxForce = MyBot.getMaxValue(pw, FORCE);
		float maxDistance = MyBot.getMaxDistance(pw);

		// determine which pair makes the most sense
		for (Planet s: pw.MyPlanets()) {
			
			printMaximums(maxGrowth, maxForce, maxDistance);
			printSource(pw, s, maxGrowth, maxForce);

			if (DEBUG) {
				log("");
				log("ID\tTGrowth\t\tTForce\t\tTOwner\t\tDistance\t\tTotal");
			}

			for (Planet t: pw.Planets()) {
				if (t != s) {
					printEval(pw, s, t, maxGrowth, maxForce, maxDistance);
					
					float pairValue = getPairEvaluation(pw, s, t, maxGrowth, maxForce, maxDistance);
					
					if (pairValue > maxValue) {
						maxValue = pairValue;
						source = s;
						target = t;
					}
				}
			}
		}
		
		// determine the force size
		int attackForceSize = (int)((float)source.NumShips() * maxValue);
		
		// ensure that the attack force is sufficient to be successful
		if (target.Owner() != ME) {
			// TODO: Calculate ships en route as well
			int fleetDistance = pw.Distance(source.PlanetID(), target.PlanetID());
			int numShips = target.NumShips();
			numShips += fleetDistance * target.GrowthRate();
			
			if ((attackForceSize < numShips) && (source.NumShips() > numShips)) {
				attackForceSize = numShips + ((source.NumShips() - numShips) / 2);
			}
		}
		

		// make the call
		pw.IssueOrder(source, target, attackForceSize);
	}
	
	private static float getPairEvaluation(PlanetWars pw, Planet source, Planet target, int maxGrowth, int maxForce, float maxDistance) {
		float sourceGrowth = getSourceGrowthValue(source, maxGrowth)   * SOURCE_GROWTH_WEIGHT;
		float sourceForce = getSourceForceValue(pw, source, maxForce)  * SOURCE_FORCE_WEIGHT;
		float targetGrowth = getTargetGrowthValue(target, maxGrowth)   * TARGET_GROWTH_WEIGHT;
		float targetForce = getTargetForceValue(pw, target, maxForce)  * TARGET_FORCE_WEIGHT;
		float targetOwnership = getTargetOwnershipValue(target)        * TARGET_OWNERSHIP_WEIGHT;
		float distance = getDistanceValue(source, target, maxDistance) * DISTANCE_WEIGHT;

		// aggregate the values together
		return sourceGrowth + sourceForce + targetGrowth + targetForce + targetOwnership + distance;
	}
	
	// should return a value between 0 and 1 (1 being the best possible value)
	private static float getSourceGrowthValue(Planet p, int maxGrowth) {
		return (float)(maxGrowth - p.GrowthRate()) / (float)maxGrowth;
	}

	private static float getSourceForceValue(PlanetWars pw, Planet p, int maxForce) {
		return (float)Math.abs(getNumShips(ME, pw, p)) / (float)maxForce;
	}

	private static float getTargetGrowthValue(Planet p, int maxGrowth) {
		return (float)p.GrowthRate() / (float)maxGrowth;
	}

	private static float getTargetForceValue(PlanetWars pw, Planet p, int maxForce) {
		return (float)(maxForce - Math.abs(getNumShips(p.Owner(), pw, p))) / (float)maxForce;
	}

	public static final float OWNED_BASE = 0.05f;
	public static final float ENEMY_BASE = 1.00f;
	public static final float UNOWNED_BASE = 0.95f;
	
	private static float getTargetOwnershipValue(Planet p) {
		if (p.Owner() == ME) {
			return OWNED_BASE;
		} else if (p.Owner() == ENEMY) {
			return ENEMY_BASE;
		} else {
			return UNOWNED_BASE;
		}
	}

	private static float getDistanceValue(Planet source, Planet target, float maxDistance) {
		return (maxDistance - getDistanceBetween(source, target)) / maxDistance;
	}
	
	// low-level calculation methods follow
	
	public static final int GROWTH = 1;
	public static final int FORCE = 2;
	private static int getMaxValue(PlanetWars pw, int type) {
		int maxValue = Integer.MIN_VALUE;
		for (Planet p: pw.Planets()) {
			int value = 0;
			if (type == GROWTH)
				value = p.GrowthRate();
			else if (type == FORCE)
				value = Math.abs(getNumShips(ME, pw, p));
			if (value > maxValue)
				maxValue = value;
		}
		
		return maxValue;
	}

	private static float getMaxDistance(PlanetWars pw) {
		float maxDistance = Float.MIN_VALUE;
		for (Planet p1: pw.MyPlanets()) {
			for (Planet p2: pw.Planets()) {
				float distance = getDistanceBetween(p1, p2);
				if (distance > maxDistance)
					maxDistance = distance;
			}
		}
		return maxDistance;
	}
	
	private static float getDistanceBetween(Planet source, Planet dest) {
		double x1 = source.X();
		double y1 = source.Y();
		double x2 = dest.X();
		double y2 = dest.Y();
		
		return (float)Math.sqrt(((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2)));
	}
	
	public static final float EN_ROUTE_FACTOR = 0.6f;
	
	private static int getNumShips(int player, PlanetWars pw, Planet p) {
		int numShips = 0;
		int numShipsEnRoute = 0;
		
		if (p.Owner() != ME)
			numShips = -1 * p.NumShips();
		else
			numShips = p.NumShips();
		
		for (Fleet f: pw.MyFleets()) {
			if (f.DestinationPlanet() == p.PlanetID()) {
				if (p.Owner() == ME) {
					numShipsEnRoute += f.NumShips() * EN_ROUTE_FACTOR;
				} else {
					numShipsEnRoute -= f.NumShips() * EN_ROUTE_FACTOR;
				}
			}
		}
		
		for (Fleet f: pw.EnemyFleets()) {
			if (f.DestinationPlanet() == p.PlanetID()) {
				if (p.Owner() != ME) {
					numShipsEnRoute += f.NumShips() * EN_ROUTE_FACTOR;
				} else {
					numShipsEnRoute -= f.NumShips() * EN_ROUTE_FACTOR;
				}
			}
		}

		return numShips + numShipsEnRoute;
	}
}
