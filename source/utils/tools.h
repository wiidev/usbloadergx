#ifndef TOOLS_H_
#define TOOLS_H_

	/* Constants */
#define KB_SIZE	 1024.0f
#define MB_SIZE	 1048576.0f
#define GB_SIZE	 1073741824.0f

#define round_up(x,n)   (-(-(x) & -(n)))

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
