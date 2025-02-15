#include "includes.h"
#include "threading/threading.h"
#include "threading/shared_mutex.h"

int __stdcall DllMain( HMODULE self, ulong_t reason, void *reserved ) {
    if( reason == DLL_PROCESS_ATTACH ) {
        CreateThread( nullptr, 0, Client::Init, nullptr, 0, nullptr );

        return 1;
    }

    return 0;
}