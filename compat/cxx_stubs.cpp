// operator new/delete on top of malloc/free. Without these the linker pulls
// operator new[] from libstdc++-6.dll, which does not exist on the head unit,
// and the tool fails to load.
#include <stdlib.h>

void *operator new(unsigned int n) { return malloc(n ? n : 1); }
void *operator new[](unsigned int n) { return malloc(n ? n : 1); }
void operator delete(void *p) { free(p); }
void operator delete[](void *p) { free(p); }
