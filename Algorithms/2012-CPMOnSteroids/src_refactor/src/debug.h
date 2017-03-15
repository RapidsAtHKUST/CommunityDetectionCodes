#ifndef DEBUG_H
#define	DEBUG_H

/* debug flags */
#define DEBUG_NORMAL	1
#define DEBUG_FAT	2
#define DEBUG_OBESE	3
#define DEBUG_EXPLODING	4

#define	strornull(x)	((x) ? (x) : "<null>")

#ifdef DEBUG
void	debug__(int, const char *, ...)
			__attribute__((__format__(__printf__, 2, 3)));
#define debug(x)	debug__ x
#else
#define	debug(x)	
#endif /* DEBUG */

#endif	/* DEBUG_H */

