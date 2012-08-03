
#ifndef TRUE
#define TRUE	(1)
#endif

#ifndef FALSE
#define FALSE	(0)
#endif

#define	CHECK_FLAG(x,y) ( (x&y)==y ? (TRUE) : (FALSE) )
