// This file has been automatically genrated by "generate_keys.sh" 
#ifndef KCKEY_H 
#define KCKEY_H 

typedef struct { 
	const char *name; 
	int code; 
} KKeys; 

#define MAX_KEY_LENGTH           15   // should be calculated (gawk ?) 
#define MAX_KEY_MODIFIER_LENGTH   21  // "SHIFT + CRTL + ALT + " :  
#define MAX_FCTN_LENGTH           50  // arbitrary limit 
#define NB_KEYS                 233 
extern const KKeys KKEYS[NB_KEYS]; 

#endif
