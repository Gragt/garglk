//
//  robinofsherwood.h
//  part of ScottFree, an interpreter for adventures in Scott Adams format
//
//  Created by Petter Sjölund on 2022-01-10.
//

#ifndef robinofsherwood_h
#define robinofsherwood_h

#include "scottdefines.h"
#include <stdio.h>

void UpdateRobinOfSherwoodAnimations(void);
void RobinOfSherwoodLook(void);
GameIDType LoadExtraSherwoodData(void);
GameIDType LoadExtraSherwoodData64(void);
int IsForestLocation(void);
void SherwoodAction(int p);

#endif /* robinofsherwood_h */
