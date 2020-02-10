#ifdef WIN32
#define RL_API __dllspec(dllimport)
#endif
#define REBOL_DISABLE_ACCESSOR_MACROS
#include "../include/rebol.h"

#ifdef WIN32
#define RL_API __dllspec(dllexport)
#else
#define RL_API
#endif

RL_API void * rebMalloc(size_t size) {
    RL_rebEnterApi_internal();
     return RL_rebMalloc(size);
 }

RL_API void * rebRealloc(void * ptr, size_t new_size) {
    RL_rebEnterApi_internal();
     return RL_rebRealloc(ptr, new_size);
 }

RL_API void rebFree(void * ptr) {
    RL_rebEnterApi_internal();
     RL_rebFree(ptr);
 }

RL_API REBVAL * rebRepossess(void * ptr, size_t size) {
    RL_rebEnterApi_internal();
     return RL_rebRepossess(ptr, size);
 }

RL_API void rebStartup(void) {
     RL_rebStartup();
 }

RL_API void rebShutdown(bool clean) {
    RL_rebEnterApi_internal();
     RL_rebShutdown(clean);
 }

RL_API uintptr_t rebTick(void) {
    RL_rebEnterApi_internal();
     return RL_rebTick();
 }

RL_API REBVAL * rebVoid(void) {
    RL_rebEnterApi_internal();
     return RL_rebVoid();
 }

RL_API REBVAL * rebBlank(void) {
    RL_rebEnterApi_internal();
     return RL_rebBlank();
 }

RL_API REBVAL * rebLogic(bool logic) {
    RL_rebEnterApi_internal();
     return RL_rebLogic(logic);
 }

RL_API REBVAL * rebChar(uint32_t codepoint) {
    RL_rebEnterApi_internal();
     return RL_rebChar(codepoint);
 }

RL_API REBVAL * rebInteger(int64_t i) {
    RL_rebEnterApi_internal();
     return RL_rebInteger(i);
 }

RL_API REBVAL * rebDecimal(double dec) {
    RL_rebEnterApi_internal();
     return RL_rebDecimal(dec);
 }

RL_API REBVAL * rebSizedBinary(const void * bytes, size_t size) {
    RL_rebEnterApi_internal();
     return RL_rebSizedBinary(bytes, size);
 }

RL_API REBVAL * rebUninitializedBinary_internal(size_t size) {
    RL_rebEnterApi_internal();
     return RL_rebUninitializedBinary_internal(size);
 }

RL_API unsigned char * rebBinaryHead_internal(const REBVAL * binary) {
    RL_rebEnterApi_internal();
     return RL_rebBinaryHead_internal(binary);
 }

RL_API unsigned char * rebBinaryAt_internal(const REBVAL * binary) {
    RL_rebEnterApi_internal();
     return RL_rebBinaryAt_internal(binary);
 }

RL_API unsigned int rebBinarySizeAt_internal(const REBVAL * binary) {
    RL_rebEnterApi_internal();
     return RL_rebBinarySizeAt_internal(binary);
 }

RL_API REBVAL * rebSizedText(const char * utf8, size_t size) {
    RL_rebEnterApi_internal();
     return RL_rebSizedText(utf8, size);
 }

RL_API REBVAL * rebText(const char * utf8) {
    RL_rebEnterApi_internal();
     return RL_rebText(utf8);
 }

RL_API REBVAL * rebLengthedTextWide(const REBWCHAR * wstr, unsigned int num_chars) {
    RL_rebEnterApi_internal();
     return RL_rebLengthedTextWide(wstr, num_chars);
 }

RL_API REBVAL * rebTextWide(const REBWCHAR * wstr) {
    RL_rebEnterApi_internal();
     return RL_rebTextWide(wstr);
 }

RL_API REBVAL * rebHandle(void * data, size_t length, CLEANUP_CFUNC * cleaner) {
    RL_rebEnterApi_internal();
     return RL_rebHandle(data, length, cleaner);
 }

RL_API const void * rebArgR(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebArgR(0, p, &va);
 }

RL_API const void * rebArgRQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebArgR(1, p, &va);
 }

RL_API REBVAL * rebArg(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebArg(0, p, &va);
 }

RL_API REBVAL * rebArgQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebArg(1, p, &va);
 }

RL_API REBVAL * rebValue(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebValue(0, p, &va);
 }

RL_API REBVAL * rebValueQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebValue(1, p, &va);
 }

RL_API REBVAL * rebQuote(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebQuote(0, p, &va);
 }

RL_API REBVAL * rebQuoteQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebQuote(1, p, &va);
 }

RL_API void rebElide(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    RL_rebElide(0, p, &va);
 }

RL_API void rebElideQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    RL_rebElide(1, p, &va);
 }

ATTRIBUTE_NO_RETURN
RL_API void rebJumps(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    RL_rebJumps(0, p, &va);
    DEAD_END;
}

ATTRIBUTE_NO_RETURN
RL_API void rebJumpsQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    RL_rebJumps(1, p, &va);
    DEAD_END;
}

RL_API bool rebDid(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebDid(0, p, &va);
 }

RL_API bool rebDidQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebDid(1, p, &va);
 }

RL_API bool rebNot(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebNot(0, p, &va);
 }

RL_API bool rebNotQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebNot(1, p, &va);
 }

RL_API intptr_t rebUnbox(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebUnbox(0, p, &va);
 }

RL_API intptr_t rebUnboxQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebUnbox(1, p, &va);
 }

RL_API intptr_t rebUnbox0(const void * p) {
    RL_rebEnterApi_internal();
     return RL_rebUnbox0(p);
 }

RL_API intptr_t rebUnboxInteger(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebUnboxInteger(0, p, &va);
 }

RL_API intptr_t rebUnboxIntegerQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebUnboxInteger(1, p, &va);
 }

RL_API intptr_t rebUnboxInteger0(const void * p) {
    RL_rebEnterApi_internal();
     return RL_rebUnboxInteger0(p);
 }

RL_API double rebUnboxDecimal(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebUnboxDecimal(0, p, &va);
 }

RL_API double rebUnboxDecimalQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebUnboxDecimal(1, p, &va);
 }

RL_API uint32_t rebUnboxChar(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebUnboxChar(0, p, &va);
 }

RL_API uint32_t rebUnboxCharQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebUnboxChar(1, p, &va);
 }

RL_API size_t rebSpellInto(char * buf, size_t buf_size, const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebSpellInto(0, buf, buf_size, p, &va);
 }

RL_API size_t rebSpellIntoQ(char * buf, size_t buf_size, const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebSpellInto(1, buf, buf_size, p, &va);
 }

RL_API char * rebSpell(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebSpell(0, p, &va);
 }

RL_API char * rebSpellQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebSpell(1, p, &va);
 }

RL_API unsigned int rebSpellIntoWide(REBWCHAR * buf, unsigned int buf_chars, const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebSpellIntoWide(0, buf, buf_chars, p, &va);
 }

RL_API unsigned int rebSpellIntoWideQ(REBWCHAR * buf, unsigned int buf_chars, const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebSpellIntoWide(1, buf, buf_chars, p, &va);
 }

RL_API REBWCHAR * rebSpellWide(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebSpellWide(0, p, &va);
 }

RL_API REBWCHAR * rebSpellWideQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebSpellWide(1, p, &va);
 }

RL_API size_t rebBytesInto(unsigned char * buf, size_t buf_size, const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebBytesInto(0, buf, buf_size, p, &va);
 }

RL_API size_t rebBytesIntoQ(unsigned char * buf, size_t buf_size, const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebBytesInto(1, buf, buf_size, p, &va);
 }

RL_API unsigned char * rebBytes(size_t * size_out, const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebBytes(0, size_out, p, &va);
 }

RL_API unsigned char * rebBytesQ(size_t * size_out, const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebBytes(1, size_out, p, &va);
 }

RL_API REBVAL * rebRescue(REBDNG * dangerous, void * opaque) {
    RL_rebEnterApi_internal();
     return RL_rebRescue(dangerous, opaque);
 }

RL_API REBVAL * rebRescueWith(REBDNG * dangerous, REBRSC * rescuer, void * opaque) {
    RL_rebEnterApi_internal();
     return RL_rebRescueWith(dangerous, rescuer, opaque);
 }

RL_API void rebHalt(void) {
    RL_rebEnterApi_internal();
     RL_rebHalt();
 }

RL_API const void * rebQUOTING(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebQUOTING(0, p, &va);
 }

RL_API const void * rebQUOTINGQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebQUOTING(1, p, &va);
 }

RL_API const void * rebUNQUOTING(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebUNQUOTING(0, p, &va);
 }

RL_API const void * rebUNQUOTINGQ(const void *p, ...) {
    RL_rebEnterApi_internal();
    va_list va; va_start(va, p);
    return RL_rebUNQUOTING(1, p, &va);
 }

RL_API const void * rebRELEASING(REBVAL * v) {
    RL_rebEnterApi_internal();
     return RL_rebRELEASING(v);
 }

RL_API REBVAL * rebManage(REBVAL * v) {
    RL_rebEnterApi_internal();
     return RL_rebManage(v);
 }

RL_API void rebUnmanage(void * p) {
    RL_rebEnterApi_internal();
     RL_rebUnmanage(p);
 }

RL_API void rebRelease(const REBVAL * v) {
    RL_rebEnterApi_internal();
     RL_rebRelease(v);
 }

RL_API void * rebDeflateAlloc(size_t * out_len, const void * input, size_t in_len) {
    RL_rebEnterApi_internal();
     return RL_rebDeflateAlloc(out_len, input, in_len);
 }

RL_API void * rebZdeflateAlloc(size_t * out_len, const void * input, size_t in_len) {
    RL_rebEnterApi_internal();
     return RL_rebZdeflateAlloc(out_len, input, in_len);
 }

RL_API void * rebGzipAlloc(size_t * out_len, const void * input, size_t in_len) {
    RL_rebEnterApi_internal();
     return RL_rebGzipAlloc(out_len, input, in_len);
 }

RL_API void * rebInflateAlloc(size_t * len_out, const void * input, size_t len_in, int max) {
    RL_rebEnterApi_internal();
     return RL_rebInflateAlloc(len_out, input, len_in, max);
 }

RL_API void * rebZinflateAlloc(size_t * len_out, const void * input, size_t len_in, int max) {
    RL_rebEnterApi_internal();
     return RL_rebZinflateAlloc(len_out, input, len_in, max);
 }

RL_API void * rebGunzipAlloc(size_t * len_out, const void * input, size_t len_in, int max) {
    RL_rebEnterApi_internal();
     return RL_rebGunzipAlloc(len_out, input, len_in, max);
 }

RL_API void * rebDeflateDetectAlloc(size_t * len_out, const void * input, size_t len_in, int max) {
    RL_rebEnterApi_internal();
     return RL_rebDeflateDetectAlloc(len_out, input, len_in, max);
 }

ATTRIBUTE_NO_RETURN
RL_API void rebFail_OS(int errnum) {
    RL_rebEnterApi_internal();
     RL_rebFail_OS(errnum);
    DEAD_END;
}

