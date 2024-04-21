This program iteratively creates random Power Weaver rotations and finds the best one.
Basic flow is:
- generate random attunements order with random (but reasonable) swap times
- generate list of random skills, from skills that are available (ie. follow cooldown/attunements)
- calculate all damage instances from each skill the same way as in-game
- compare final DPS number with the number from previous rotation and overwrite all rotation/attunement vectors if it's better

Notes:
- Weave Self is always the first skill used
- Player will always obtain Perfect Weave buff when close to ending of Weave Self
- Condition damage instances are actually calculated with higher granularity than in-game (aka damage from conditions happens more frequently, not every 1 second)
- player can maintain AA chain in different attunement, if it was started in previous attunement, as in-game (eg. AA chain starts in Fire, but you can continue it in Water)
- BttH is assumed to be 10%, perma uptime
- Burning Precision proc is truely random
- there are two outut files, outputRotation.csv and DamageInstances.csv

Things to change/adjust:
- numIterations(50,000 by default)     // how many iterative cycles will be done. Probably around 5000 iterations can be done per minute on average machine.
- maxRotationTime(62 by default)       // how long the rotation should be
- double randomSwapFireWS = 2.7;       // random max swap intervals during Weave Self/outside of Weave Self, e.g randomSwapFireWS = 2.7 means while in Fire attunement, player will stay in it for 1.6 - 2.7 sec (random)
  double randomSwapWaterWS = 1.75;
  double randomSwapAirWS = 2.2;
	double randomSwapEarthWS = 1.75;
  double randomSwapFire = 5.2;
  double randomSwapWater = 3.2;
	double randomSwapAir = 4.0;
  double randomSwapEarth = 3.2
- at line 300, there is optional overwriting of attunement order
- at line 376 there are adjustable skillWeights
- you can right after skillWeights declaration write some conditions that'd change them, eg.
  if (primaryAttunement[i-1] == "Air" && secondaryAttunement[i-1] == "Fire") {
	    skillWeights["PyroVortex"] = 1.0;
  }
- at line 813 there is manual declaration of Primordial Stance timing - can be adjusted
- at line 918 there is manual declaration of Arcane Blast timing - can be adjusted
