/*
 * pac_states.h - Valid states and substates for the pacifier module.
 */

/* States for the pacifier state machine:  */
#define FORWARD	0
#define REVERSE	1

/* These substates are valid for both states:  */
#define N_S	0
#define NE_SW	1
#define E_W	2
#define NW_SE	3

/* Number of substates to pass through before reversing.  */
#define THRESHOLD 20
