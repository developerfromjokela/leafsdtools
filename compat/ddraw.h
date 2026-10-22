// The cegcc sysroot has no ddraw.h. DisplayDirectDraw.cpp needs only
// DirectDrawCreate(), Release() on the two interfaces, DDSD_CAPS and
// LPDDBLTFX as an opaque pointer; every other DirectDraw method is called
// through the vtable by offset. Full COM declarations are not required.
#ifndef LEAFSDTOOLS_COMPAT_DDRAW_H
#define LEAFSDTOOLS_COMPAT_DDRAW_H

#include <windows.h>
#include <unknwn.h>

#ifndef DDSD_CAPS
#define DDSD_CAPS 0x00000001
#endif

typedef struct _DDBLTFX *LPDDBLTFX;

#ifdef __cplusplus
struct IDirectDraw : public IUnknown {};
struct IDirectDrawSurface : public IUnknown {};
#else
typedef struct IDirectDraw IDirectDraw;
typedef struct IDirectDrawSurface IDirectDrawSurface;
#endif

typedef IDirectDraw *LPDIRECTDRAW;
typedef IDirectDrawSurface *LPDIRECTDRAWSURFACE;

#ifdef __cplusplus
extern "C" {
#endif
HRESULT WINAPI DirectDrawCreate(GUID *lpGUID, LPDIRECTDRAW *lplpDD,
                                IUnknown *pUnkOuter);
#ifdef __cplusplus
}
#endif

#endif
