#ifndef TOOLS_H_
#define TOOLS_H_

#define ABS(x) ( (x) >= (0) ? (x) : (-(x)) )
#define LIMIT(x, min, max)																	\
	({																						\
		typeof( x ) _x = x;																	\
		typeof( min ) _min = min;															\
		typeof( max ) _max = max;															\
		( ( ( _x ) < ( _min ) ) ? ( _min ) : ( ( _x ) > ( _max ) ) ? ( _max) : ( _x ) );	\
	})
#define ALIGN(x) (((x) + 3) & ~3)
#define ALIGN32(x) (((x) + 31) & ~31)

#endif
