#include <stdio.h>

__thread volatile char test_var_bss[2984];
__thread volatile char test_var_data[2408] = {34,4,4,4,3,32,2,45,4,3,4,3,43};

extern char _stdata[], _etdata[], _stbss[], _etbss[], _tls_end[], _stbootdata[],
   _etbootdata[], _stbootbss[], _etbootbss[];

int main(void)
{
    printf(
        "_stdata: %p\n"
        "_etdata: %p\n"
        "_stbss: %p\n"
        "_etbss: %p\n"
        "_tls_end: %p\n"
        "_stbootdata: %p\n"
        "_etbootdata: %p\n"
        "_stbootbss: %p\n"
        "_etbootbss: %p\n",
        _stdata, _etdata, _stbss, _etbss, _tls_end,
        _stbootdata, _etbootdata, _stbootbss, _etbootbss
    );

    return 0;
}
