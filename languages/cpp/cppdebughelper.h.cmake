#cmakedefine DEBUG_UI_LOCKUP 
#cmakedefine DEBUG 
#cmakedefine DEBUG_INCLUDE_PATHS 

#ifdef DEBUG
#define ifDebug(x) x
#else
#define ifDebug(x)
#endif

