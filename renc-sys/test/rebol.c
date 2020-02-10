#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "rebol.h"

int main ()
{
    RL_rebStartup();
    REBVAL *one = RL_rebInteger(1);
#ifdef GOOD
    assert(1 == rebUnboxInteger(one));
#else
    assert(1 == RL_rebUnboxInteger0(one));
#endif
    rebRelease(one);
    RL_rebShutdown(1);
    return 0;
}
