#ifndef STDBOOL_H_
#define STDBOOL_H_

#ifndef __cplusplus

typedef int __Bool;

/**
 * Define the Boolean macros only if they are not already defined.
 */
#ifndef bool
#define bool __Bool
#define false 0 
#define true 1
#endif

#endif

#endif /*STDBOOL_H_*/