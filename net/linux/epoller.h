#include "../poller.h"
#include <sys/epoll.h>

class Epoller : public Poller {
public:
    Epoller();
    ~Epoller();
    virtual void update(Channel *channel) override;
    virtual void remove(Channel *channel) override;
    virtual int getTimerFd() override;
    virtual std::vector<Channel*> poll() override;
    virtual void wakeup() override;
private:
    int epollfd_;
    typedef std::vector<struct epoll_event> EventList;
    EventList events_;
};

