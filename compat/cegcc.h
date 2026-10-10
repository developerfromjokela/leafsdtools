// Force-included (-include) for every translation unit when building with
// cegcc / mingw32ce (GCC 4.4). Supplies what eMbedded Visual C++ 4 provides
// and the cegcc sysroot does not. Not used by the eVC4 project.
#ifndef LEAFSDTOOLS_COMPAT_CEGCC_H
#define LEAFSDTOOLS_COMPAT_CEGCC_H

#include <windows.h>

// Integer types that the eVC4 / Windows CE SDK headers define.
typedef unsigned char byte;
typedef unsigned int uint;
#ifndef UINT16
typedef unsigned short UINT16;
#endif
#ifndef INT16
typedef signed short INT16;
#endif
#ifndef UINT8
typedef unsigned char UINT8;
#endif
#ifndef INT8
typedef signed char INT8;
#endif

// GCC has no Microsoft structured exception handling. The guarded block runs
// unprotected and the handler block is compiled out, so a fault that the eVC4
// build would catch terminates the process here.
#define __try if (1)
#define __except(x) else if (0)
#ifndef GetExceptionCode
#define GetExceptionCode() (0UL)
#endif

#endif
