// The cegcc sysroot has no ceddk.h. StdAfx.h includes it for MmMapIoSpace,
// which is only used by Display.cpp (SH4 / QY7 configuration). That file is
// not part of the ARMV4I / QY8XXX build, so an empty header is enough.
#ifndef LEAFSDTOOLS_COMPAT_CEDDK_H
#define LEAFSDTOOLS_COMPAT_CEDDK_H
#endif
