// interface_find.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)


#ifndef ISA_INTERFACE_FIND_H
#define ISA_INTERFACE_FIND_H

#include <map>
#include <ifaddrs.h>
#include <string>
#include <netdb.h>


class InterfaceFinder {
public:
    InterfaceFinder() : addresses() {};

    /**
     * Walks through an addrinfo linked list of destination IP addresses and attempts to find an interface that
     * reaches them. The first pair of reachable destination IP address and source interface is returned.
     *
     * @remark The memory pointed to by the returned @a src_address and @a dst_address is deallocated in InterfaceFinder
     * destructor.
     * @param [in] target An addrinfo linked list of destination IP addresses.
     * @param [out] if_name The name of the interface that reaches one of the destination addresses.
     * @param [out] src_address A pointer to sockaddr representing the source (interface) IP address.
     * @param [out] dst_address A pointer to sockaddr representing the destination IP address that can be reached
     * using the interface returned in if_name and its address returned in src_address.
     * @return True if a reachable destination has been found, false otherwise.
     */
    bool find_source_interface(addrinfo *target, std::string &if_name, sockaddr *&src_address, sockaddr *&dst_address);

    /**
     * Deletes the structures allocated and returned to the caller by find_source_interface().
     */
    ~InterfaceFinder();

private:
    /** Holds a pointer to the last ifaddrs instance in memory. This is used to call freeifaddrs() in the destructor.*/
    struct ifaddrs *addresses;

    // Dictionaries of all network interface addresses and the corresponding interface names
    std::map<sockaddr *, std::string> ipv4Ifaces;
    std::map<sockaddr *, std::string> ipv6Ifaces;

    /**
     * Loads the addresses of all interfaces available in the system.
     */
    void load_interface_list();

    /**
     * Attempts to ping a destination using an interface and its IP address (either IPv4 or IPv6).
     *
     * @param [in] dst The destination address.
     * @param [in] src The source (interface) address.
     * @param [in] if_name The name of the source interface.
     * @return True if the target is reachable, false otherwise.
     */
    static bool send_ping(sockaddr *dst, sockaddr *src, const std::string &if_name);
};


#endif //ISA_INTERFACE_FIND_H
