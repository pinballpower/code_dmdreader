#include "endian.h"

void fix_uint32_endian(uint32_t *d) {
#ifdef LITTLE_ENDIAN
	uint32_t res = (*d >> 24) + (((*d >> 16) & 0xff) << 8) + (((*d >> 8) & 0xff) << 16) + ((*d & 0xff) << 24);
	*d = res;
#endif
}
