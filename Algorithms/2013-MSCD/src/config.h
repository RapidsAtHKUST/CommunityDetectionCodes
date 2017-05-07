//
//  Global setup configuration file
//  config.h
//
//  Created by Erwan Le Martelot on 03/04/2012.
//  Copyright (c) 2012 Erwan Le Martelot. All rights reserved.
//

#ifndef MSCD_CONFIG
#define MSCD_CONFIG

// Real number precision
#define preal float

// Enable multi-threading (requires C++11)
#define USE_MULTITHREADING

// Enable Armadillo for fast matrix operations in SOM (stability optimisation using matrices)
//#define USE_ARMADILLO

// Enable debug mode
//#define __DEBUG__


// Turn off the "assert" tests when not in debug mode
#ifndef __DEBUG__
#define NDEBUG
#endif

#endif
