#include "poller.h"
#include "kqueue_poller.h"

Poller *createPoller() {
    return new KqueuePoller();
}