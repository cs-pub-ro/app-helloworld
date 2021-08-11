#include <stdio.h>

__thread volatile char test_var_tdata[10] = {1,2,3,4,5,6,7,8,9,10};
__thread volatile char test_var_tbss[20];
__thread volatile char push_data_tbss[1136];
// __thread volatile char push_data_tdata[1144] = {1,2,3,4,5,6,7,8,9,10};

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

