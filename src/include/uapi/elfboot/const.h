#ifndef __CONST_H__
#define __CONST_H__

/*
 * Constant macros
 */

#define __AC(x, y)                                (x ## y)
#define _AC(x, y)                                 __AC(x, y)

#define _UL(x)                                    (_AC(x, UL))
#define _ULL(x)                                   (_AC(x, ULL))

#define _BITUL(x)                                 (_UL(1) << (x))
#define _BITULL(x)                                (_ULL(1) << (x))

#endif /* __CONST_H__ */