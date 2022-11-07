// errors.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)


#ifndef ISA_ERRORS_H
#define ISA_ERRORS_H

#include <stdexcept>
#include <system_error>
#include <stdexcept>
#include <unistd.h>

/**
 * Throws an std::system_error encapsulating the current value of errno.
 */
#define THROW_ERRNO() throw std::system_error(errno, std::generic_category())

/**
 * Throws an std::system_error encapsulating the current value of errno.
 * Fills the exception's what field.
 */
#define THROW_ERRNO_W(what) throw std::system_error(errno, std::generic_category(), (what))

/**
 * Saves the current value of errno, calls close() on the specified file descriptor
 * and then throws an std::system_error encapsulating the saved value of errno.
 */
#define CLOSE_THROW(fd) auto __errno_prev = errno; close((fd)); \
    throw std::system_error(__errno_prev, std::generic_category())

/**
 * Saves the current value of errno, calls close() on the specified file descriptor
 * and then throws an std::system_error encapsulating the saved value of errno.
 * Fills the exception's what field.
 */
#define CLOSE_THROW_W(fd, what) auto __errno_prev = errno; close((fd)); \
    throw std::system_error(__errno_prev, std::generic_category(), (what))

#endif //ISA_ERRORS_H
