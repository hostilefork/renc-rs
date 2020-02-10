/**********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2018 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**  Licensed under the Apache License, Version 2.0
**
************************************************************************
**
**  Title: "Rebol External Library Interface"
**  Build: A0
**  File: %rebol.h
**  Author: %make-reb-lib.r
**  License: {
**      Licensed under the Apache License, Version 2.0.
**      See: http://www.apache.org/licenses/LICENSE-2.0
**  }
**
***********************************************************************/

/*
 * The goal is to make it possible that the only include file one needs
 * to make a simple Rebol library client is `#include "rebol.h"`.  Yet
 * pre-C99 or pre-C++11 compilers will need `#define REBOL_EXPLICIT_END`
 * since variadic macros don't work.  They will also need shims for
 * stdint.h and stdbool.h included.
 */
#include <stdlib.h>  /* for size_t */
#include <stdarg.h>  /* for va_list, va_start() in inline functions */
#if !defined(_PSTDINT_H_INCLUDED) && !defined(LIBREBOL_NO_STDINT)
    #include <stdint.h>  /* for uintptr_t, int64_t, etc. */
#endif
#if !defined(_PSTDBOOL_H_INCLUDED) && !defined(LIBREBOL_NO_STDBOOL)
    #if !defined(__cplusplus)
        #include <stdbool.h>  /* for bool, true, false (if C99) */
    #endif
#endif

/*
 * !!! Needed by following two macros.
 */
#ifndef __has_builtin
    #define __has_builtin(x) 0
#endif
#if !defined(GCC_VERSION_AT_LEAST) /* !!! duplicated in %reb-config.h */
    #ifdef __GNUC__
        #define GCC_VERSION_AT_LEAST(m, n) \
            (__GNUC__ > (m) || (__GNUC__ == (m) && __GNUC_MINOR__ >= (n)))
    #else
        #define GCC_VERSION_AT_LEAST(m, n) 0
    #endif
#endif

/*
 * !!! _Noreturn was introduced in C11, but prior to that (including C99)
 * there was no standard way of doing it.  If we didn't mark APIs which
 * don't return with this, there'd be warnings in the calling code.
 */
#if !defined(ATTRIBUTE_NO_RETURN) /* !!! duplicated in %reb-config.h */
    #if defined(__clang__) || GCC_VERSION_AT_LEAST(2, 5)
        #define ATTRIBUTE_NO_RETURN __attribute__ ((noreturn))
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
        #define ATTRIBUTE_NO_RETURN _Noreturn
    #elif defined(_MSC_VER)
        #define ATTRIBUTE_NO_RETURN __declspec(noreturn)
    #else
        #define ATTRIBUTE_NO_RETURN
    #endif
#endif

/*
 * !!! Same story for DEAD_END as for ATTRIBUTE_NO_RETURN.  Necessary to
 * suppress spurious warnings.
 */
#if !defined(DEAD_END) /* !!! duplicated in %reb-config.h */
    #if __has_builtin(__builtin_unreachable) || GCC_VERSION_AT_LEAST(4, 5)
        #define DEAD_END __builtin_unreachable()
    #elif defined(_MSC_VER)
        __declspec(noreturn) static inline void msvc_unreachable(void) {
            while (1) { }
        }
        #define DEAD_END msvc_unreachable()
    #else
        #define DEAD_END
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * !!! These constants are part of an old R3-Alpha versioning system
 * that hasn't been paid much attention to.  Keeping as a placeholder.
 */
#define RL_VER 2
#define RL_REV 102
#define RL_UPD 0

/*
 * The API can be used by the core on value cell pointers that are in
 * stable locations guarded by GC (e.g. frame argument or output cells).
 * Since the core uses REBVAL*, it must be accurate (not just a void*)
 */
struct Reb_Value;
#define REBVAL struct Reb_Value

/*
 * `wchar_t` is a pre-Unicode abstraction, whose size varies per-platform
 * and should be avoided where possible.  But Win32 standardizes it to
 * 2 bytes in size for UTF-16, and uses it pervasively.  So libRebol
 * currently offers APIs (e.g. rebTextWide() instead of rebText()) which
 * support this 2-byte notion of wide characters.
 *
 * In order for C++ to be type-compatible with Windows's WCHAR definition,
 * a #define on Windows to wchar_t is needed.  But on non-Windows, it
 * must use `uint16_t` since there's no size guarantee for wchar_t.  This
 * is useful for compatibility with unixodbc's SQLWCHAR.
 *
 * !!! REBWCHAR is just for the API definitions--don't mention it in
 * client code.  If the client code is on Windows, use WCHAR.  If it's in
 * a unixodbc client use SQLWCHAR.  But use UTF-8 if you possibly can.
 */
#ifdef TO_WINDOWS
    #define REBWCHAR wchar_t
#else
    #define REBWCHAR uint16_t
#endif

/*
 * "Dangerous Function" which is called by rebRescue().  Argument can be a
 * REBVAL* but does not have to be.  Result must be a REBVAL* or NULL.
 *
 * !!! If the dangerous function returns an ERROR!, it will currently be
 * converted to null, which parallels TRAP without a handler.  nulls will
 * be converted to voids.
 */
typedef REBVAL* (REBDNG)(void *opaque);

/*
 * "Rescue Function" called as the handler in rebRescueWith().  Receives
 * the REBVAL* of the error that occurred, and the opaque pointer.
 *
 * !!! If either the dangerous function or the rescuing function return an
 * ERROR! value, that is not interfered with the way rebRescue() does.
 */
typedef REBVAL* (REBRSC)(REBVAL *error, void *opaque);

/*
 * For some HANDLE!s GC callback
 */
typedef void (CLEANUP_CFUNC)(const REBVAL*);

/*
 * The API maps Rebol's `null` to C's 0 pointer, **but don't use NULL**.
 * Some C compilers define NULL as simply the constant 0, which breaks
 * use with variadic APIs...since they will interpret it as an integer
 * and not a pointer.
 *
 * **It's best to use C++'s `nullptr`**, or a suitable C shim for it,
 * e.g. `#define nullptr ((void*)0)`.  That helps avoid obscuring the
 * fact that the Rebol API's null really is C's null, and is conditionally
 * false.  Seeing `rebNull` in source doesn't as clearly suggest this.
 *
 * However, **using NULL is broken, so don't use it**.  This macro is
 * provided in case defining `nullptr` is not an option--for some reason.
 */
#define rebNull \
    ((REBVAL*)0)

/*
 * Since a C nullptr (pointer cast of 0) is used to represent the Rebol
 * `null` in the API, something different must be used to indicate the
 * end of variadic input.  So a pointer to data is used where the first
 * byte is illegal for starting UTF-8 (a continuation byte, first bit 1,
 * second bit 0) and the second byte is 0.
 *
 * To Rebol, the first bit being 1 means it's a Rebol node, the second
 * that it is not in the "free" state.  The lowest bit in the first byte
 * clear indicates it doesn't point to a "cell".  The SECOND_BYTE() is
 * where the VAL_TYPE() of a cell is usually stored, and this being 0
 * indicates an END marker.
 */
#define rebEND \
    ((const void*)"\x80")

/*
 * Some trick macros are not done with function calls to the API.  So
 * they risk having bad typing.  This helper can be low cost (or no
 * cost, if you don't want it).
 */
#if defined(__cplusplus)
    #define rebCELL(v) \
        static_cast<const REBVAL*>(v)
#elif !defined(NDEBUG)
    inline static const void *rebCELL(const REBVAL *v)
        { return v; }
#else
    #define rebCELL(v) (v)
#endif

/*
 * SHORTHAND MACROS
 *
 * These shorthand macros make the API somewhat more readable, but as
 * they are macros you can redefine them to other definitions if you want.
 *
 * THESE DON'T WORK IN JAVASCRIPT, so when updating them be sure to update
 * the JavaScript versions, which have to make ordinary stub functions.
 * (The C portion of the Emscripten build can use these internally, as
 * the implementation is C.  But when calling the lib from JS, it is
 * obviously not reading this generated header file!)
 */

#define rebR rebRELEASING
#define rebQ rebQUOTING
#define rebU rebUNQUOTING

#define rebQ1(v)  /* C89 requires the rebEND, nice to omit it */ \
    rebQ((v), rebEND)  /* has optimization in rebQ() for this case */

#define rebU1(v) /* C89 requires the rebEND, nice to omit it */ \
    rebU((v), rebEND)  /* has optimization in rebU() for this case */

#define rebT(utf8) \
    rebR(rebText(utf8))  /* might rebTEXT() delayed-load? */

#define rebI(int64) \
    rebR(rebInteger(int64))

#define rebL(flag) \
    rebR(rebLogic(flag))

/*
 * Function entry points for reb-lib.  Formulating this way allows the
 * interface structure to be passed from an EXE to a DLL, then the DLL
 * can call into the EXE (which is not generically possible via linking).
 *
 * For convenience, calls to RL->xxx are wrapped in inline functions:
 */
typedef struct rebol_ext_api {
    void (*rebEnterApi_internal)(void);
    void * (*rebMalloc)(size_t size);
    void * (*rebRealloc)(void * ptr, size_t new_size);
    void (*rebFree)(void * ptr);
    REBVAL * (*rebRepossess)(void * ptr, size_t size);
    void (*rebStartup)(void);
    void (*rebShutdown)(bool clean);
    uintptr_t (*rebTick)(void);
    REBVAL * (*rebVoid)(void);
    REBVAL * (*rebBlank)(void);
    REBVAL * (*rebLogic)(bool logic);
    REBVAL * (*rebChar)(uint32_t codepoint);
    REBVAL * (*rebInteger)(int64_t i);
    REBVAL * (*rebDecimal)(double dec);
    REBVAL * (*rebSizedBinary)(const void * bytes, size_t size);
    REBVAL * (*rebUninitializedBinary_internal)(size_t size);
    unsigned char * (*rebBinaryHead_internal)(const REBVAL * binary);
    unsigned char * (*rebBinaryAt_internal)(const REBVAL * binary);
    unsigned int (*rebBinarySizeAt_internal)(const REBVAL * binary);
    REBVAL * (*rebSizedText)(const char * utf8, size_t size);
    REBVAL * (*rebText)(const char * utf8);
    REBVAL * (*rebLengthedTextWide)(const REBWCHAR * wstr, unsigned int num_chars);
    REBVAL * (*rebTextWide)(const REBWCHAR * wstr);
    REBVAL * (*rebHandle)(void * data, size_t length, CLEANUP_CFUNC * cleaner);
    const void * (*rebArgR)(unsigned char quotes, const void *p, va_list *vaptr);
    REBVAL * (*rebArg)(unsigned char quotes, const void *p, va_list *vaptr);
    REBVAL * (*rebValue)(unsigned char quotes, const void *p, va_list *vaptr);
    REBVAL * (*rebQuote)(unsigned char quotes, const void *p, va_list *vaptr);
    void (*rebElide)(unsigned char quotes, const void *p, va_list *vaptr);
    void (*rebJumps)(unsigned char quotes, const void *p, va_list *vaptr);
    bool (*rebDid)(unsigned char quotes, const void *p, va_list *vaptr);
    bool (*rebNot)(unsigned char quotes, const void *p, va_list *vaptr);
    intptr_t (*rebUnbox)(unsigned char quotes, const void *p, va_list *vaptr);
    intptr_t (*rebUnbox0)(const void * p);
    intptr_t (*rebUnboxInteger)(unsigned char quotes, const void *p, va_list *vaptr);
    intptr_t (*rebUnboxInteger0)(const void * p);
    double (*rebUnboxDecimal)(unsigned char quotes, const void *p, va_list *vaptr);
    uint32_t (*rebUnboxChar)(unsigned char quotes, const void *p, va_list *vaptr);
    size_t (*rebSpellInto)(unsigned char quotes, char * buf, size_t buf_size, const void *p, va_list *vaptr);
    char * (*rebSpell)(unsigned char quotes, const void *p, va_list *vaptr);
    unsigned int (*rebSpellIntoWide)(unsigned char quotes, REBWCHAR * buf, unsigned int buf_chars, const void *p, va_list *vaptr);
    REBWCHAR * (*rebSpellWide)(unsigned char quotes, const void *p, va_list *vaptr);
    size_t (*rebBytesInto)(unsigned char quotes, unsigned char * buf, size_t buf_size, const void *p, va_list *vaptr);
    unsigned char * (*rebBytes)(unsigned char quotes, size_t * size_out, const void *p, va_list *vaptr);
    REBVAL * (*rebRescue)(REBDNG * dangerous, void * opaque);
    REBVAL * (*rebRescueWith)(REBDNG * dangerous, REBRSC * rescuer, void * opaque);
    void (*rebHalt)(void);
    const void * (*rebQUOTING)(unsigned char quotes, const void *p, va_list *vaptr);
    const void * (*rebUNQUOTING)(unsigned char quotes, const void *p, va_list *vaptr);
    const void * (*rebRELEASING)(REBVAL * v);
    REBVAL * (*rebManage)(REBVAL * v);
    void (*rebUnmanage)(void * p);
    void (*rebRelease)(const REBVAL * v);
    void * (*rebDeflateAlloc)(size_t * out_len, const void * input, size_t in_len);
    void * (*rebZdeflateAlloc)(size_t * out_len, const void * input, size_t in_len);
    void * (*rebGzipAlloc)(size_t * out_len, const void * input, size_t in_len);
    void * (*rebInflateAlloc)(size_t * len_out, const void * input, size_t len_in, int max);
    void * (*rebZinflateAlloc)(size_t * len_out, const void * input, size_t len_in, int max);
    void * (*rebGunzipAlloc)(size_t * len_out, const void * input, size_t len_in, int max);
    void * (*rebDeflateDetectAlloc)(size_t * len_out, const void * input, size_t len_in, int max);
    void (*rebFail_OS)(int errnum);
} RL_LIB;

#ifdef REB_EXT /* can't direct call into EXE, must go through interface */
    /*
     * The inline functions below will require this base pointer:
     */
    extern RL_LIB *RL; /* is passed to the RX_Init() function */

    /*
     * Inlines to access reb-lib functions (from non-linked extensions):
     */

    inline static void * rebMalloc_inline(size_t size) {
        RL->rebEnterApi_internal();
         return RL->rebMalloc(size);
     }
    
    inline static void * rebRealloc_inline(void * ptr, size_t new_size) {
        RL->rebEnterApi_internal();
         return RL->rebRealloc(ptr, new_size);
     }
    
    inline static void rebFree_inline(void * ptr) {
        RL->rebEnterApi_internal();
         RL->rebFree(ptr);
     }
    
    inline static REBVAL * rebRepossess_inline(void * ptr, size_t size) {
        RL->rebEnterApi_internal();
         return RL->rebRepossess(ptr, size);
     }
    
    inline static void rebStartup_inline(void) {
         RL->rebStartup();
     }
    
    inline static void rebShutdown_inline(bool clean) {
        RL->rebEnterApi_internal();
         RL->rebShutdown(clean);
     }
    
    inline static uintptr_t rebTick_inline(void) {
        RL->rebEnterApi_internal();
         return RL->rebTick();
     }
    
    inline static REBVAL * rebVoid_inline(void) {
        RL->rebEnterApi_internal();
         return RL->rebVoid();
     }
    
    inline static REBVAL * rebBlank_inline(void) {
        RL->rebEnterApi_internal();
         return RL->rebBlank();
     }
    
    inline static REBVAL * rebLogic_inline(bool logic) {
        RL->rebEnterApi_internal();
         return RL->rebLogic(logic);
     }
    
    inline static REBVAL * rebChar_inline(uint32_t codepoint) {
        RL->rebEnterApi_internal();
         return RL->rebChar(codepoint);
     }
    
    inline static REBVAL * rebInteger_inline(int64_t i) {
        RL->rebEnterApi_internal();
         return RL->rebInteger(i);
     }
    
    inline static REBVAL * rebDecimal_inline(double dec) {
        RL->rebEnterApi_internal();
         return RL->rebDecimal(dec);
     }
    
    inline static REBVAL * rebSizedBinary_inline(const void * bytes, size_t size) {
        RL->rebEnterApi_internal();
         return RL->rebSizedBinary(bytes, size);
     }
    
    inline static REBVAL * rebUninitializedBinary_internal_inline(size_t size) {
        RL->rebEnterApi_internal();
         return RL->rebUninitializedBinary_internal(size);
     }
    
    inline static unsigned char * rebBinaryHead_internal_inline(const REBVAL * binary) {
        RL->rebEnterApi_internal();
         return RL->rebBinaryHead_internal(binary);
     }
    
    inline static unsigned char * rebBinaryAt_internal_inline(const REBVAL * binary) {
        RL->rebEnterApi_internal();
         return RL->rebBinaryAt_internal(binary);
     }
    
    inline static unsigned int rebBinarySizeAt_internal_inline(const REBVAL * binary) {
        RL->rebEnterApi_internal();
         return RL->rebBinarySizeAt_internal(binary);
     }
    
    inline static REBVAL * rebSizedText_inline(const char * utf8, size_t size) {
        RL->rebEnterApi_internal();
         return RL->rebSizedText(utf8, size);
     }
    
    inline static REBVAL * rebText_inline(const char * utf8) {
        RL->rebEnterApi_internal();
         return RL->rebText(utf8);
     }
    
    inline static REBVAL * rebLengthedTextWide_inline(const REBWCHAR * wstr, unsigned int num_chars) {
        RL->rebEnterApi_internal();
         return RL->rebLengthedTextWide(wstr, num_chars);
     }
    
    inline static REBVAL * rebTextWide_inline(const REBWCHAR * wstr) {
        RL->rebEnterApi_internal();
         return RL->rebTextWide(wstr);
     }
    
    inline static REBVAL * rebHandle_inline(void * data, size_t length, CLEANUP_CFUNC * cleaner) {
        RL->rebEnterApi_internal();
         return RL->rebHandle(data, length, cleaner);
     }
    
    inline static const void * rebArgR_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebArgR(0, p, &va);
     }
    
    inline static const void * rebArgRQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebArgR(1, p, &va);
     }
    
    inline static REBVAL * rebArg_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebArg(0, p, &va);
     }
    
    inline static REBVAL * rebArgQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebArg(1, p, &va);
     }
    
    inline static REBVAL * rebValue_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebValue(0, p, &va);
     }
    
    inline static REBVAL * rebValueQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebValue(1, p, &va);
     }
    
    inline static REBVAL * rebQuote_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebQuote(0, p, &va);
     }
    
    inline static REBVAL * rebQuoteQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebQuote(1, p, &va);
     }
    
    inline static void rebElide_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        RL->rebElide(0, p, &va);
     }
    
    inline static void rebElideQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        RL->rebElide(1, p, &va);
     }
    
    ATTRIBUTE_NO_RETURN
    inline static void rebJumps_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        RL->rebJumps(0, p, &va);
        DEAD_END;
    }
    
    ATTRIBUTE_NO_RETURN
    inline static void rebJumpsQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        RL->rebJumps(1, p, &va);
        DEAD_END;
    }
    
    inline static bool rebDid_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebDid(0, p, &va);
     }
    
    inline static bool rebDidQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebDid(1, p, &va);
     }
    
    inline static bool rebNot_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebNot(0, p, &va);
     }
    
    inline static bool rebNotQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebNot(1, p, &va);
     }
    
    inline static intptr_t rebUnbox_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebUnbox(0, p, &va);
     }
    
    inline static intptr_t rebUnboxQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebUnbox(1, p, &va);
     }
    
    inline static intptr_t rebUnbox0_inline(const void * p) {
        RL->rebEnterApi_internal();
         return RL->rebUnbox0(p);
     }
    
    inline static intptr_t rebUnboxInteger_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebUnboxInteger(0, p, &va);
     }
    
    inline static intptr_t rebUnboxIntegerQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebUnboxInteger(1, p, &va);
     }
    
    inline static intptr_t rebUnboxInteger0_inline(const void * p) {
        RL->rebEnterApi_internal();
         return RL->rebUnboxInteger0(p);
     }
    
    inline static double rebUnboxDecimal_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebUnboxDecimal(0, p, &va);
     }
    
    inline static double rebUnboxDecimalQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebUnboxDecimal(1, p, &va);
     }
    
    inline static uint32_t rebUnboxChar_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebUnboxChar(0, p, &va);
     }
    
    inline static uint32_t rebUnboxCharQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebUnboxChar(1, p, &va);
     }
    
    inline static size_t rebSpellInto_inline(char * buf, size_t buf_size, const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebSpellInto(0, buf, buf_size, p, &va);
     }
    
    inline static size_t rebSpellIntoQ_inline(char * buf, size_t buf_size, const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebSpellInto(1, buf, buf_size, p, &va);
     }
    
    inline static char * rebSpell_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebSpell(0, p, &va);
     }
    
    inline static char * rebSpellQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebSpell(1, p, &va);
     }
    
    inline static unsigned int rebSpellIntoWide_inline(REBWCHAR * buf, unsigned int buf_chars, const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebSpellIntoWide(0, buf, buf_chars, p, &va);
     }
    
    inline static unsigned int rebSpellIntoWideQ_inline(REBWCHAR * buf, unsigned int buf_chars, const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebSpellIntoWide(1, buf, buf_chars, p, &va);
     }
    
    inline static REBWCHAR * rebSpellWide_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebSpellWide(0, p, &va);
     }
    
    inline static REBWCHAR * rebSpellWideQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebSpellWide(1, p, &va);
     }
    
    inline static size_t rebBytesInto_inline(unsigned char * buf, size_t buf_size, const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebBytesInto(0, buf, buf_size, p, &va);
     }
    
    inline static size_t rebBytesIntoQ_inline(unsigned char * buf, size_t buf_size, const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebBytesInto(1, buf, buf_size, p, &va);
     }
    
    inline static unsigned char * rebBytes_inline(size_t * size_out, const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebBytes(0, size_out, p, &va);
     }
    
    inline static unsigned char * rebBytesQ_inline(size_t * size_out, const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebBytes(1, size_out, p, &va);
     }
    
    inline static REBVAL * rebRescue_inline(REBDNG * dangerous, void * opaque) {
        RL->rebEnterApi_internal();
         return RL->rebRescue(dangerous, opaque);
     }
    
    inline static REBVAL * rebRescueWith_inline(REBDNG * dangerous, REBRSC * rescuer, void * opaque) {
        RL->rebEnterApi_internal();
         return RL->rebRescueWith(dangerous, rescuer, opaque);
     }
    
    inline static void rebHalt_inline(void) {
        RL->rebEnterApi_internal();
         RL->rebHalt();
     }
    
    inline static const void * rebQUOTING_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebQUOTING(0, p, &va);
     }
    
    inline static const void * rebQUOTINGQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebQUOTING(1, p, &va);
     }
    
    inline static const void * rebUNQUOTING_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebUNQUOTING(0, p, &va);
     }
    
    inline static const void * rebUNQUOTINGQ_inline(const void *p, ...) {
        RL->rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL->rebUNQUOTING(1, p, &va);
     }
    
    inline static const void * rebRELEASING_inline(REBVAL * v) {
        RL->rebEnterApi_internal();
         return RL->rebRELEASING(v);
     }
    
    inline static REBVAL * rebManage_inline(REBVAL * v) {
        RL->rebEnterApi_internal();
         return RL->rebManage(v);
     }
    
    inline static void rebUnmanage_inline(void * p) {
        RL->rebEnterApi_internal();
         RL->rebUnmanage(p);
     }
    
    inline static void rebRelease_inline(const REBVAL * v) {
        RL->rebEnterApi_internal();
         RL->rebRelease(v);
     }
    
    inline static void * rebDeflateAlloc_inline(size_t * out_len, const void * input, size_t in_len) {
        RL->rebEnterApi_internal();
         return RL->rebDeflateAlloc(out_len, input, in_len);
     }
    
    inline static void * rebZdeflateAlloc_inline(size_t * out_len, const void * input, size_t in_len) {
        RL->rebEnterApi_internal();
         return RL->rebZdeflateAlloc(out_len, input, in_len);
     }
    
    inline static void * rebGzipAlloc_inline(size_t * out_len, const void * input, size_t in_len) {
        RL->rebEnterApi_internal();
         return RL->rebGzipAlloc(out_len, input, in_len);
     }
    
    inline static void * rebInflateAlloc_inline(size_t * len_out, const void * input, size_t len_in, int max) {
        RL->rebEnterApi_internal();
         return RL->rebInflateAlloc(len_out, input, len_in, max);
     }
    
    inline static void * rebZinflateAlloc_inline(size_t * len_out, const void * input, size_t len_in, int max) {
        RL->rebEnterApi_internal();
         return RL->rebZinflateAlloc(len_out, input, len_in, max);
     }
    
    inline static void * rebGunzipAlloc_inline(size_t * len_out, const void * input, size_t len_in, int max) {
        RL->rebEnterApi_internal();
         return RL->rebGunzipAlloc(len_out, input, len_in, max);
     }
    
    inline static void * rebDeflateDetectAlloc_inline(size_t * len_out, const void * input, size_t len_in, int max) {
        RL->rebEnterApi_internal();
         return RL->rebDeflateDetectAlloc(len_out, input, len_in, max);
     }
    
    ATTRIBUTE_NO_RETURN
    inline static void rebFail_OS_inline(int errnum) {
        RL->rebEnterApi_internal();
         RL->rebFail_OS(errnum);
        DEAD_END;
    }
    

#else  /* ...calling Rebol as DLL, or code built into the EXE itself */

    /*
     * !!! The RL_API macro has to be defined for the external prototypes
     * to compile.  Assume for now that if not defined via %reb-config.h,
     * then it can be empty--though this will almost certainly need to
     * be revisited (as it needs __dllimport and other such things on
     * Windows, so those details will come up at some point)
     */
  #if !defined(RL_API)
    #define RL_API
  #endif

    /*
     * Extern prototypes for RL_XXX, don't call these functions directly.
     * They use vaptr instead of `...`, and may not do all the proper
     * exception/longjmp handling needed.
     */

    RL_API void RL_rebEnterApi_internal(void);
    RL_API void *RL_rebMalloc(size_t size);
    RL_API void *RL_rebRealloc(void *ptr, size_t new_size);
    RL_API void RL_rebFree(void *ptr);
    RL_API REBVAL *RL_rebRepossess(void *ptr, size_t size);
    RL_API void RL_rebStartup(void);
    RL_API void RL_rebShutdown(bool clean);
    RL_API uintptr_t RL_rebTick(void);
    RL_API REBVAL *RL_rebVoid(void);
    RL_API REBVAL *RL_rebBlank(void);
    RL_API REBVAL *RL_rebLogic(bool logic);
    RL_API REBVAL *RL_rebChar(uint32_t codepoint);
    RL_API REBVAL *RL_rebInteger(int64_t i);
    RL_API REBVAL *RL_rebDecimal(double dec);
    RL_API REBVAL *RL_rebSizedBinary(const void *bytes, size_t size);
    RL_API REBVAL *RL_rebUninitializedBinary_internal(size_t size);
    RL_API unsigned char *RL_rebBinaryHead_internal(const REBVAL *binary);
    RL_API unsigned char *RL_rebBinaryAt_internal(const REBVAL *binary);
    RL_API unsigned int RL_rebBinarySizeAt_internal(const REBVAL *binary);
    RL_API REBVAL *RL_rebSizedText(const char *utf8, size_t size);
    RL_API REBVAL *RL_rebText(const char *utf8);
    RL_API REBVAL *RL_rebLengthedTextWide(const REBWCHAR *wstr, unsigned int num_chars);
    RL_API REBVAL *RL_rebTextWide(const REBWCHAR *wstr);
    RL_API REBVAL *RL_rebHandle(void *data, size_t length, CLEANUP_CFUNC *cleaner);
    RL_API const void *RL_rebArgR(unsigned char quotes, const void *p, va_list *vaptr);
    RL_API REBVAL *RL_rebArg(unsigned char quotes, const void *p, va_list *vaptr);
    RL_API REBVAL *RL_rebValue(unsigned char quotes, const void *p, va_list *vaptr);
    RL_API REBVAL *RL_rebQuote(unsigned char quotes, const void *p, va_list *vaptr);
    RL_API void RL_rebElide(unsigned char quotes, const void *p, va_list *vaptr);
    RL_API void RL_rebJumps(unsigned char quotes, const void *p, va_list *vaptr);
    RL_API bool RL_rebDid(unsigned char quotes, const void *p, va_list *vaptr);
    RL_API bool RL_rebNot(unsigned char quotes, const void *p, va_list *vaptr);
    RL_API intptr_t RL_rebUnbox(unsigned char quotes, const void *p, va_list *vaptr);
    RL_API intptr_t RL_rebUnbox0(const void *p);
    RL_API intptr_t RL_rebUnboxInteger( unsigned char quotes, const void *p, va_list *vaptr );
    RL_API intptr_t RL_rebUnboxInteger0(const void *p);
    RL_API double RL_rebUnboxDecimal( unsigned char quotes, const void *p, va_list *vaptr );
    RL_API uint32_t RL_rebUnboxChar( unsigned char quotes, const void *p, va_list *vaptr );
    RL_API size_t RL_rebSpellInto( unsigned char quotes, char *buf, size_t buf_size, const void *p, va_list *vaptr );
    RL_API char *RL_rebSpell( unsigned char quotes, const void *p, va_list *vaptr );
    RL_API unsigned int RL_rebSpellIntoWide( unsigned char quotes, REBWCHAR *buf, unsigned int buf_chars, const void *p, va_list *vaptr );
    RL_API REBWCHAR *RL_rebSpellWide( unsigned char quotes, const void *p, va_list *vaptr );
    RL_API size_t RL_rebBytesInto( unsigned char quotes, unsigned char *buf, size_t buf_size, const void *p, va_list *vaptr );
    RL_API unsigned char *RL_rebBytes( unsigned char quotes, size_t *size_out, const void *p, va_list *vaptr );
    RL_API REBVAL *RL_rebRescue( REBDNG *dangerous, void *opaque );
    RL_API REBVAL *RL_rebRescueWith( REBDNG *dangerous, REBRSC *rescuer, void *opaque );
    RL_API void RL_rebHalt(void);
    RL_API const void *RL_rebQUOTING( unsigned char quotes, const void *p, va_list *vaptr );
    RL_API const void *RL_rebUNQUOTING( unsigned char quotes, const void *p, va_list *vaptr );
    RL_API const void *RL_rebRELEASING(REBVAL *v);
    RL_API REBVAL *RL_rebManage(REBVAL *v);
    RL_API void RL_rebUnmanage(void *p);
    RL_API void RL_rebRelease(const REBVAL *v);
    RL_API void *RL_rebDeflateAlloc( size_t *out_len, const void *input, size_t in_len );
    RL_API void *RL_rebZdeflateAlloc( size_t *out_len, const void *input, size_t in_len );
    RL_API void *RL_rebGzipAlloc( size_t *out_len, const void *input, size_t in_len );
    RL_API void *RL_rebInflateAlloc( size_t *len_out, const void *input, size_t len_in, int max );
    RL_API void *RL_rebZinflateAlloc( size_t *len_out, const void *input, size_t len_in, int max );
    RL_API void *RL_rebGunzipAlloc( size_t *len_out, const void *input, size_t len_in, int max );
    RL_API void *RL_rebDeflateDetectAlloc( size_t *len_out, const void *input, size_t len_in, int max );
    RL_API void RL_rebFail_OS(int errnum);

    /*
     * rebXXX_inline functions which do the work of
     */

    inline static void * rebMalloc_inline(size_t size) {
        RL_rebEnterApi_internal();
         return RL_rebMalloc(size);
     }
    
    inline static void * rebRealloc_inline(void * ptr, size_t new_size) {
        RL_rebEnterApi_internal();
         return RL_rebRealloc(ptr, new_size);
     }
    
    inline static void rebFree_inline(void * ptr) {
        RL_rebEnterApi_internal();
         RL_rebFree(ptr);
     }
    
    inline static REBVAL * rebRepossess_inline(void * ptr, size_t size) {
        RL_rebEnterApi_internal();
         return RL_rebRepossess(ptr, size);
     }
    
    inline static void rebStartup_inline(void) {
         RL_rebStartup();
     }
    
    inline static void rebShutdown_inline(bool clean) {
        RL_rebEnterApi_internal();
         RL_rebShutdown(clean);
     }
    
    inline static uintptr_t rebTick_inline(void) {
        RL_rebEnterApi_internal();
         return RL_rebTick();
     }
    
    inline static REBVAL * rebVoid_inline(void) {
        RL_rebEnterApi_internal();
         return RL_rebVoid();
     }
    
    inline static REBVAL * rebBlank_inline(void) {
        RL_rebEnterApi_internal();
         return RL_rebBlank();
     }
    
    inline static REBVAL * rebLogic_inline(bool logic) {
        RL_rebEnterApi_internal();
         return RL_rebLogic(logic);
     }
    
    inline static REBVAL * rebChar_inline(uint32_t codepoint) {
        RL_rebEnterApi_internal();
         return RL_rebChar(codepoint);
     }
    
    inline static REBVAL * rebInteger_inline(int64_t i) {
        RL_rebEnterApi_internal();
         return RL_rebInteger(i);
     }
    
    inline static REBVAL * rebDecimal_inline(double dec) {
        RL_rebEnterApi_internal();
         return RL_rebDecimal(dec);
     }
    
    inline static REBVAL * rebSizedBinary_inline(const void * bytes, size_t size) {
        RL_rebEnterApi_internal();
         return RL_rebSizedBinary(bytes, size);
     }
    
    inline static REBVAL * rebUninitializedBinary_internal_inline(size_t size) {
        RL_rebEnterApi_internal();
         return RL_rebUninitializedBinary_internal(size);
     }
    
    inline static unsigned char * rebBinaryHead_internal_inline(const REBVAL * binary) {
        RL_rebEnterApi_internal();
         return RL_rebBinaryHead_internal(binary);
     }
    
    inline static unsigned char * rebBinaryAt_internal_inline(const REBVAL * binary) {
        RL_rebEnterApi_internal();
         return RL_rebBinaryAt_internal(binary);
     }
    
    inline static unsigned int rebBinarySizeAt_internal_inline(const REBVAL * binary) {
        RL_rebEnterApi_internal();
         return RL_rebBinarySizeAt_internal(binary);
     }
    
    inline static REBVAL * rebSizedText_inline(const char * utf8, size_t size) {
        RL_rebEnterApi_internal();
         return RL_rebSizedText(utf8, size);
     }
    
    inline static REBVAL * rebText_inline(const char * utf8) {
        RL_rebEnterApi_internal();
         return RL_rebText(utf8);
     }
    
    inline static REBVAL * rebLengthedTextWide_inline(const REBWCHAR * wstr, unsigned int num_chars) {
        RL_rebEnterApi_internal();
         return RL_rebLengthedTextWide(wstr, num_chars);
     }
    
    inline static REBVAL * rebTextWide_inline(const REBWCHAR * wstr) {
        RL_rebEnterApi_internal();
         return RL_rebTextWide(wstr);
     }
    
    inline static REBVAL * rebHandle_inline(void * data, size_t length, CLEANUP_CFUNC * cleaner) {
        RL_rebEnterApi_internal();
         return RL_rebHandle(data, length, cleaner);
     }
    
    inline static const void * rebArgR_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebArgR(0, p, &va);
     }
    
    inline static const void * rebArgRQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebArgR(1, p, &va);
     }
    
    inline static REBVAL * rebArg_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebArg(0, p, &va);
     }
    
    inline static REBVAL * rebArgQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebArg(1, p, &va);
     }
    
    inline static REBVAL * rebValue_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebValue(0, p, &va);
     }
    
    inline static REBVAL * rebValueQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebValue(1, p, &va);
     }
    
    inline static REBVAL * rebQuote_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebQuote(0, p, &va);
     }
    
    inline static REBVAL * rebQuoteQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebQuote(1, p, &va);
     }
    
    inline static void rebElide_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        RL_rebElide(0, p, &va);
     }
    
    inline static void rebElideQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        RL_rebElide(1, p, &va);
     }
    
    ATTRIBUTE_NO_RETURN
    inline static void rebJumps_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        RL_rebJumps(0, p, &va);
        DEAD_END;
    }
    
    ATTRIBUTE_NO_RETURN
    inline static void rebJumpsQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        RL_rebJumps(1, p, &va);
        DEAD_END;
    }
    
    inline static bool rebDid_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebDid(0, p, &va);
     }
    
    inline static bool rebDidQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebDid(1, p, &va);
     }
    
    inline static bool rebNot_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebNot(0, p, &va);
     }
    
    inline static bool rebNotQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebNot(1, p, &va);
     }
    
    inline static intptr_t rebUnbox_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebUnbox(0, p, &va);
     }
    
    inline static intptr_t rebUnboxQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebUnbox(1, p, &va);
     }
    
    inline static intptr_t rebUnbox0_inline(const void * p) {
        RL_rebEnterApi_internal();
         return RL_rebUnbox0(p);
     }
    
    inline static intptr_t rebUnboxInteger_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebUnboxInteger(0, p, &va);
     }
    
    inline static intptr_t rebUnboxIntegerQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebUnboxInteger(1, p, &va);
     }
    
    inline static intptr_t rebUnboxInteger0_inline(const void * p) {
        RL_rebEnterApi_internal();
         return RL_rebUnboxInteger0(p);
     }
    
    inline static double rebUnboxDecimal_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebUnboxDecimal(0, p, &va);
     }
    
    inline static double rebUnboxDecimalQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebUnboxDecimal(1, p, &va);
     }
    
    inline static uint32_t rebUnboxChar_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebUnboxChar(0, p, &va);
     }
    
    inline static uint32_t rebUnboxCharQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebUnboxChar(1, p, &va);
     }
    
    inline static size_t rebSpellInto_inline(char * buf, size_t buf_size, const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebSpellInto(0, buf, buf_size, p, &va);
     }
    
    inline static size_t rebSpellIntoQ_inline(char * buf, size_t buf_size, const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebSpellInto(1, buf, buf_size, p, &va);
     }
    
    inline static char * rebSpell_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebSpell(0, p, &va);
     }
    
    inline static char * rebSpellQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebSpell(1, p, &va);
     }
    
    inline static unsigned int rebSpellIntoWide_inline(REBWCHAR * buf, unsigned int buf_chars, const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebSpellIntoWide(0, buf, buf_chars, p, &va);
     }
    
    inline static unsigned int rebSpellIntoWideQ_inline(REBWCHAR * buf, unsigned int buf_chars, const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebSpellIntoWide(1, buf, buf_chars, p, &va);
     }
    
    inline static REBWCHAR * rebSpellWide_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebSpellWide(0, p, &va);
     }
    
    inline static REBWCHAR * rebSpellWideQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebSpellWide(1, p, &va);
     }
    
    inline static size_t rebBytesInto_inline(unsigned char * buf, size_t buf_size, const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebBytesInto(0, buf, buf_size, p, &va);
     }
    
    inline static size_t rebBytesIntoQ_inline(unsigned char * buf, size_t buf_size, const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebBytesInto(1, buf, buf_size, p, &va);
     }
    
    inline static unsigned char * rebBytes_inline(size_t * size_out, const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebBytes(0, size_out, p, &va);
     }
    
    inline static unsigned char * rebBytesQ_inline(size_t * size_out, const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebBytes(1, size_out, p, &va);
     }
    
    inline static REBVAL * rebRescue_inline(REBDNG * dangerous, void * opaque) {
        RL_rebEnterApi_internal();
         return RL_rebRescue(dangerous, opaque);
     }
    
    inline static REBVAL * rebRescueWith_inline(REBDNG * dangerous, REBRSC * rescuer, void * opaque) {
        RL_rebEnterApi_internal();
         return RL_rebRescueWith(dangerous, rescuer, opaque);
     }
    
    inline static void rebHalt_inline(void) {
        RL_rebEnterApi_internal();
         RL_rebHalt();
     }
    
    inline static const void * rebQUOTING_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebQUOTING(0, p, &va);
     }
    
    inline static const void * rebQUOTINGQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebQUOTING(1, p, &va);
     }
    
    inline static const void * rebUNQUOTING_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebUNQUOTING(0, p, &va);
     }
    
    inline static const void * rebUNQUOTINGQ_inline(const void *p, ...) {
        RL_rebEnterApi_internal();
        va_list va; va_start(va, p);
        return RL_rebUNQUOTING(1, p, &va);
     }
    
    inline static const void * rebRELEASING_inline(REBVAL * v) {
        RL_rebEnterApi_internal();
         return RL_rebRELEASING(v);
     }
    
    inline static REBVAL * rebManage_inline(REBVAL * v) {
        RL_rebEnterApi_internal();
         return RL_rebManage(v);
     }
    
    inline static void rebUnmanage_inline(void * p) {
        RL_rebEnterApi_internal();
         RL_rebUnmanage(p);
     }
    
    inline static void rebRelease_inline(const REBVAL * v) {
        RL_rebEnterApi_internal();
         RL_rebRelease(v);
     }
    
    inline static void * rebDeflateAlloc_inline(size_t * out_len, const void * input, size_t in_len) {
        RL_rebEnterApi_internal();
         return RL_rebDeflateAlloc(out_len, input, in_len);
     }
    
    inline static void * rebZdeflateAlloc_inline(size_t * out_len, const void * input, size_t in_len) {
        RL_rebEnterApi_internal();
         return RL_rebZdeflateAlloc(out_len, input, in_len);
     }
    
    inline static void * rebGzipAlloc_inline(size_t * out_len, const void * input, size_t in_len) {
        RL_rebEnterApi_internal();
         return RL_rebGzipAlloc(out_len, input, in_len);
     }
    
    inline static void * rebInflateAlloc_inline(size_t * len_out, const void * input, size_t len_in, int max) {
        RL_rebEnterApi_internal();
         return RL_rebInflateAlloc(len_out, input, len_in, max);
     }
    
    inline static void * rebZinflateAlloc_inline(size_t * len_out, const void * input, size_t len_in, int max) {
        RL_rebEnterApi_internal();
         return RL_rebZinflateAlloc(len_out, input, len_in, max);
     }
    
    inline static void * rebGunzipAlloc_inline(size_t * len_out, const void * input, size_t len_in, int max) {
        RL_rebEnterApi_internal();
         return RL_rebGunzipAlloc(len_out, input, len_in, max);
     }
    
    inline static void * rebDeflateDetectAlloc_inline(size_t * len_out, const void * input, size_t len_in, int max) {
        RL_rebEnterApi_internal();
         return RL_rebDeflateDetectAlloc(len_out, input, len_in, max);
     }
    
    ATTRIBUTE_NO_RETURN
    inline static void rebFail_OS_inline(int errnum) {
        RL_rebEnterApi_internal();
         RL_rebFail_OS(errnum);
        DEAD_END;
    }
    

#endif  /* !REB_EXT */

/*
 * C's variadic interface is very low-level, as a thin wrapper over the
 * stack memory of a function call.  So va_start() and va_end() aren't
 * really function calls...in fact, va_end() is usually a no-op.
 *
 * The simplicity is an advantage for optimization, but unsafe!  Type
 * checking is non-existent, and there is no protocol for knowing how
 * many items are in a va_list.  The libRebol API uses rebEND to signal
 * termination, but it is awkward and easy to forget.
 *
 * C89 offers no real help, but C99 (and C++11 onward) standardize an
 * interface for variadic macros:
 *
 * https://stackoverflow.com/questions/4786649/
 *
 * These macros can transform variadic input in such a way that a rebEND
 * may be automatically placed on the tail of a call.  If rebEND is used
 * explicitly, this gives a harmless but slightly inefficient repetition.
 */
#if !defined(REBOL_EXPLICIT_END)

  #if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    /* C99 or above */
  #elif defined(__cplusplus) && __cplusplus >= 201103L
    /* C++11 or above, if following the standard (VS2017 does not) */
  #elif defined (CPLUSPLUS_11)
    /* Custom C++11 or above flag, e.g. to override Visual Studio's lie */
  #else
    #error "REBOL_EXPLICIT_END must be used prior to C99 or C+++11"
  #endif

  #if !defined(REBOL_DISABLE_ACCESSOR_MACROS)
    #define rebEnterApi_internal rebEnterApi_internal_inline
    #define rebMalloc rebMalloc_inline
    #define rebRealloc rebRealloc_inline
    #define rebFree rebFree_inline
    #define rebRepossess rebRepossess_inline
    #define rebStartup rebStartup_inline
    #define rebShutdown rebShutdown_inline
    #define rebTick rebTick_inline
    #define rebVoid rebVoid_inline
    #define rebBlank rebBlank_inline
    #define rebLogic rebLogic_inline
    #define rebChar rebChar_inline
    #define rebInteger rebInteger_inline
    #define rebDecimal rebDecimal_inline
    #define rebSizedBinary rebSizedBinary_inline
    #define rebUninitializedBinary_internal rebUninitializedBinary_internal_inline
    #define rebBinaryHead_internal rebBinaryHead_internal_inline
    #define rebBinaryAt_internal rebBinaryAt_internal_inline
    #define rebBinarySizeAt_internal rebBinarySizeAt_internal_inline
    #define rebSizedText rebSizedText_inline
    #define rebText rebText_inline
    #define rebLengthedTextWide rebLengthedTextWide_inline
    #define rebTextWide rebTextWide_inline
    #define rebHandle rebHandle_inline
    #define rebArgR(...) rebArgR_inline(__VA_ARGS__, rebEND)
    #define rebArgRQ(...) rebArgRQ_inline(__VA_ARGS__, rebEND)
    #define rebArg(...) rebArg_inline(__VA_ARGS__, rebEND)
    #define rebArgQ(...) rebArgQ_inline(__VA_ARGS__, rebEND)
    #define rebValue(...) rebValue_inline(__VA_ARGS__, rebEND)
    #define rebValueQ(...) rebValueQ_inline(__VA_ARGS__, rebEND)
    #define rebQuote(...) rebQuote_inline(__VA_ARGS__, rebEND)
    #define rebQuoteQ(...) rebQuoteQ_inline(__VA_ARGS__, rebEND)
    #define rebElide(...) rebElide_inline(__VA_ARGS__, rebEND)
    #define rebElideQ(...) rebElideQ_inline(__VA_ARGS__, rebEND)
    #define rebJumps(...) rebJumps_inline(__VA_ARGS__, rebEND)
    #define rebJumpsQ(...) rebJumpsQ_inline(__VA_ARGS__, rebEND)
    #define rebDid(...) rebDid_inline(__VA_ARGS__, rebEND)
    #define rebDidQ(...) rebDidQ_inline(__VA_ARGS__, rebEND)
    #define rebNot(...) rebNot_inline(__VA_ARGS__, rebEND)
    #define rebNotQ(...) rebNotQ_inline(__VA_ARGS__, rebEND)
    #define rebUnbox(...) rebUnbox_inline(__VA_ARGS__, rebEND)
    #define rebUnboxQ(...) rebUnboxQ_inline(__VA_ARGS__, rebEND)
    #define rebUnbox0 rebUnbox0_inline
    #define rebUnboxInteger(...) rebUnboxInteger_inline(__VA_ARGS__, rebEND)
    #define rebUnboxIntegerQ(...) rebUnboxIntegerQ_inline(__VA_ARGS__, rebEND)
    #define rebUnboxInteger0 rebUnboxInteger0_inline
    #define rebUnboxDecimal(...) rebUnboxDecimal_inline(__VA_ARGS__, rebEND)
    #define rebUnboxDecimalQ(...) rebUnboxDecimalQ_inline(__VA_ARGS__, rebEND)
    #define rebUnboxChar(...) rebUnboxChar_inline(__VA_ARGS__, rebEND)
    #define rebUnboxCharQ(...) rebUnboxCharQ_inline(__VA_ARGS__, rebEND)
    #define rebSpellInto(...) rebSpellInto_inline(__VA_ARGS__, rebEND)
    #define rebSpellIntoQ(...) rebSpellIntoQ_inline(__VA_ARGS__, rebEND)
    #define rebSpell(...) rebSpell_inline(__VA_ARGS__, rebEND)
    #define rebSpellQ(...) rebSpellQ_inline(__VA_ARGS__, rebEND)
    #define rebSpellIntoWide(...) rebSpellIntoWide_inline(__VA_ARGS__, rebEND)
    #define rebSpellIntoWideQ(...) rebSpellIntoWideQ_inline(__VA_ARGS__, rebEND)
    #define rebSpellWide(...) rebSpellWide_inline(__VA_ARGS__, rebEND)
    #define rebSpellWideQ(...) rebSpellWideQ_inline(__VA_ARGS__, rebEND)
    #define rebBytesInto(...) rebBytesInto_inline(__VA_ARGS__, rebEND)
    #define rebBytesIntoQ(...) rebBytesIntoQ_inline(__VA_ARGS__, rebEND)
    #define rebBytes(...) rebBytes_inline(__VA_ARGS__, rebEND)
    #define rebBytesQ(...) rebBytesQ_inline(__VA_ARGS__, rebEND)
    #define rebRescue rebRescue_inline
    #define rebRescueWith rebRescueWith_inline
    #define rebHalt rebHalt_inline
    #define rebQUOTING(...) rebQUOTING_inline(__VA_ARGS__, rebEND)
    #define rebQUOTINGQ(...) rebQUOTINGQ_inline(__VA_ARGS__, rebEND)
    #define rebUNQUOTING(...) rebUNQUOTING_inline(__VA_ARGS__, rebEND)
    #define rebUNQUOTINGQ(...) rebUNQUOTINGQ_inline(__VA_ARGS__, rebEND)
    #define rebRELEASING rebRELEASING_inline
    #define rebManage rebManage_inline
    #define rebUnmanage rebUnmanage_inline
    #define rebRelease rebRelease_inline
    #define rebDeflateAlloc rebDeflateAlloc_inline
    #define rebZdeflateAlloc rebZdeflateAlloc_inline
    #define rebGzipAlloc rebGzipAlloc_inline
    #define rebInflateAlloc rebInflateAlloc_inline
    #define rebZinflateAlloc rebZinflateAlloc_inline
    #define rebGunzipAlloc rebGunzipAlloc_inline
    #define rebDeflateDetectAlloc rebDeflateDetectAlloc_inline
    #define rebFail_OS rebFail_OS_inline
  #endif

#else  /* REBOL_EXPLICIT_END */

    /*
     * !!! Some kind of C++ variadic trick using template recursion could
     * check to make sure you used a rebEND under this interface, when
     * building the C89-targeting code under C++11 and beyond.  TBD.
     */

  #if !defined(REBOL_DISABLE_ACCESSOR_MACROS)
    #define rebEnterApi_internal rebEnterApi_internal_inline
    #define rebMalloc rebMalloc_inline
    #define rebRealloc rebRealloc_inline
    #define rebFree rebFree_inline
    #define rebRepossess rebRepossess_inline
    #define rebStartup rebStartup_inline
    #define rebShutdown rebShutdown_inline
    #define rebTick rebTick_inline
    #define rebVoid rebVoid_inline
    #define rebBlank rebBlank_inline
    #define rebLogic rebLogic_inline
    #define rebChar rebChar_inline
    #define rebInteger rebInteger_inline
    #define rebDecimal rebDecimal_inline
    #define rebSizedBinary rebSizedBinary_inline
    #define rebUninitializedBinary_internal rebUninitializedBinary_internal_inline
    #define rebBinaryHead_internal rebBinaryHead_internal_inline
    #define rebBinaryAt_internal rebBinaryAt_internal_inline
    #define rebBinarySizeAt_internal rebBinarySizeAt_internal_inline
    #define rebSizedText rebSizedText_inline
    #define rebText rebText_inline
    #define rebLengthedTextWide rebLengthedTextWide_inline
    #define rebTextWide rebTextWide_inline
    #define rebHandle rebHandle_inline
    #define rebArgR rebArgR_inline
    #define rebArgRQ rebArgRQ_inline
    #define rebArg rebArg_inline
    #define rebArgQ rebArgQ_inline
    #define rebValue rebValue_inline
    #define rebValueQ rebValueQ_inline
    #define rebQuote rebQuote_inline
    #define rebQuoteQ rebQuoteQ_inline
    #define rebElide rebElide_inline
    #define rebElideQ rebElideQ_inline
    #define rebJumps rebJumps_inline
    #define rebJumpsQ rebJumpsQ_inline
    #define rebDid rebDid_inline
    #define rebDidQ rebDidQ_inline
    #define rebNot rebNot_inline
    #define rebNotQ rebNotQ_inline
    #define rebUnbox rebUnbox_inline
    #define rebUnboxQ rebUnboxQ_inline
    #define rebUnbox0 rebUnbox0_inline
    #define rebUnboxInteger rebUnboxInteger_inline
    #define rebUnboxIntegerQ rebUnboxIntegerQ_inline
    #define rebUnboxInteger0 rebUnboxInteger0_inline
    #define rebUnboxDecimal rebUnboxDecimal_inline
    #define rebUnboxDecimalQ rebUnboxDecimalQ_inline
    #define rebUnboxChar rebUnboxChar_inline
    #define rebUnboxCharQ rebUnboxCharQ_inline
    #define rebSpellInto rebSpellInto_inline
    #define rebSpellIntoQ rebSpellIntoQ_inline
    #define rebSpell rebSpell_inline
    #define rebSpellQ rebSpellQ_inline
    #define rebSpellIntoWide rebSpellIntoWide_inline
    #define rebSpellIntoWideQ rebSpellIntoWideQ_inline
    #define rebSpellWide rebSpellWide_inline
    #define rebSpellWideQ rebSpellWideQ_inline
    #define rebBytesInto rebBytesInto_inline
    #define rebBytesIntoQ rebBytesIntoQ_inline
    #define rebBytes rebBytes_inline
    #define rebBytesQ rebBytesQ_inline
    #define rebRescue rebRescue_inline
    #define rebRescueWith rebRescueWith_inline
    #define rebHalt rebHalt_inline
    #define rebQUOTING rebQUOTING_inline
    #define rebQUOTINGQ rebQUOTINGQ_inline
    #define rebUNQUOTING rebUNQUOTING_inline
    #define rebUNQUOTINGQ rebUNQUOTINGQ_inline
    #define rebRELEASING rebRELEASING_inline
    #define rebManage rebManage_inline
    #define rebUnmanage rebUnmanage_inline
    #define rebRelease rebRelease_inline
    #define rebDeflateAlloc rebDeflateAlloc_inline
    #define rebZdeflateAlloc rebZdeflateAlloc_inline
    #define rebGzipAlloc rebGzipAlloc_inline
    #define rebInflateAlloc rebInflateAlloc_inline
    #define rebZinflateAlloc rebZinflateAlloc_inline
    #define rebGunzipAlloc rebGunzipAlloc_inline
    #define rebDeflateDetectAlloc rebDeflateDetectAlloc_inline
    #define rebFail_OS rebFail_OS_inline
  #endif

#endif  /* REBOL_EXPLICIT_END */


/*
 * TYPE-SAFE rebMalloc() MACRO VARIANTS
 *
 * rebMalloc() offers some advantages over hosts just using malloc():
 *
 *  1. Memory can be retaken to act as a BINARY! series without another
 *     allocation, via rebRepossess().
 *
 *  2. Memory is freed automatically in the case of a failure in the
 *     frame where the rebMalloc() occured.  This is especially useful
 *     when mixing C code involving allocations with rebValue(), etc.
 *
 *  3. Memory gets counted in Rebol's knowledge of how much memory the
 *     system is using, for the purposes of triggering GC.
 *
 *  4. Out-of-memory errors on allocation automatically trigger
 *     failure vs. needing special handling by returning NULL (which may
 *     or may not be desirable, depending on what you're doing)
 *
 * Additionally, the rebAlloc(type) and rebAllocN(type, num) macros
 * automatically cast to the correct type for C++ compatibility.
 *
 * Note: There currently is no rebUnmanage() equivalent for rebMalloc()
 * data, so it must either be rebRepossess()'d or rebFree()'d before its
 * frame ends.  This limitation will be addressed in the future.
 */

#define rebAlloc(t) \
    cast(t *, rebMalloc(sizeof(t)))
#define rebAllocN(t,n) \
    cast(t *, rebMalloc(sizeof(t) * (n)))

#ifdef __cplusplus
}
#endif
