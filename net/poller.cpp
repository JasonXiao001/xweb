#include "poller.h"
#ifdef __APPLE__
#include "./mac/kqueue_poller.h"
#else
#include "./linux/epoller.h"
#endif

Poller *createPoller() {
#ifdef __APPLE__
    return new KqueuePoller();
#else
    return new Epoller();
#endif
}