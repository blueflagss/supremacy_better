#pragma once

/*
	Include thread util header files
*/
#include "atomic_lock.h"
#include "mutex.h"
//#include "named_semaphores.h"
//#include "packed_heap.h"
//#include "semaphores.h"
#include "shared_mutex.h"
#include "threading.h"

/*
	Stuff used in the cheat that has to do with threading
*/
namespace ThreadingCheatUtils {
	inline Mutex luaLock; // used to not call lua multiple times (this way, it doesn't crash)
}
