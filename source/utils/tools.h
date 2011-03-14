#ifndef TOOLS_H_
#define TOOLS_H_

#define LIMIT(x, min, max) ( ((x) < (min)) ? (min) : ((x) > (max)) ? (max) : (x) )
#define ALIGN(x) (((x) + 3) & ~3)
#define ALIGN32(x) (((x) + 31) & ~31)

#endif
