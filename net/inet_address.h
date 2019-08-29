#pragma once
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>


class InetAddress {
public:
    explicit InetAddress(const struct sockaddr_in& addr)
    : addr_(addr)
    { }

    explicit InetAddress(const struct sockaddr_in6& addr)
    : addr6_(addr)
    { }

    sa_family_t family() const { return addr_.sin_family; }

    const struct sockaddr* getSockAddr() const { 
        return (struct sockaddr *)&addr_;
    }

    size_t sizeOfAddr() const {
        return sizeof(addr_);
    }

    void setAddr(const struct sockaddr_in& addr) {
        addr_ = addr;
    }

    std::string toString() const {
        std::string str(inet_ntoa(addr_.sin_addr));
        str += ":";
        str += std::to_string(ntohs(addr_.sin_port));
        return str;
    }


private:
    union
    {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};