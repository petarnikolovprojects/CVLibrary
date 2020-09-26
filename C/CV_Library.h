/************************************************************************************************
*																								*
*  This is the main header file for the CV Library												*
*  																								*
*   Author: Petar Nikolov																		*
*																								*
*																								*
*   External links (Books, source code, Algos													*
*	- http://en.wikipedia.org/wiki/Correlated_color_temperature#Correlated_color_temperature    *
*	-																							*
*	-																							*
*	-																							*
*	-																							*
*	-																							*
*																								*
*************************************************************************************************/
//#ifndef LIBRARY
//#define LIBRARY

#define _CRT_SECURE_NO_WARNINGS

#include "CV_Structures.h"
#include "CV_enums.h"
#include "CV_FuncPrototypes.h"

//#endif

/* DEBUG information - define if you  want to save debug information */
//#define DEBUG_LOG

#ifdef DEBUG_LOG
#define DEBUG_FILE "Debug_log.txt"
#endif // DEBUG_LOG

#define QT_LIBRARIES
#ifndef QT_LIBRARIES
#define VS_LIBRARIES
#endif

#define NOEDGE 255
#define POSSIBLE_EDGE 128
#define EDGE 0

#ifndef MAX
#define MAX(a,b) (a>b?a:b)
#endif

#ifndef MIN
#define MIN(a,b) (a<b?a:b)
#endif

