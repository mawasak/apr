/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2001 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#ifndef APR_NETWORK_IO_H
#define APR_NETWORK_IO_H
/**
 * @file apr_network_io.h
 * @brief APR Network library
 */
/**
 * @defgroup APR_Net Network Routines
 * @ingroup APR
 * @{
 */

#include "apr.h"
#include "apr_pools.h"
#include "apr_file_io.h"
#include "apr_errno.h"
#include "apr_inherit.h" 

#if APR_HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef MAX_SECS_TO_LINGER
#define MAX_SECS_TO_LINGER 30
#endif

#ifndef APRMAXHOSTLEN
#define APRMAXHOSTLEN 256
#endif

#ifndef APR_ANYADDR
#define APR_ANYADDR "0.0.0.0"
#endif

/**
 * @defgroup Sock_opt Socket option definitions
 * @{
 */
#define APR_SO_LINGER        1
#define APR_SO_KEEPALIVE     2
#define APR_SO_DEBUG         4
#define APR_SO_NONBLOCK      8
#define APR_SO_REUSEADDR     16
#define APR_SO_TIMEOUT       32
#define APR_SO_SNDBUF        64
#define APR_SO_RCVBUF        128
#define APR_SO_DISCONNECTED  256
#define APR_TCP_NODELAY      512
#define APR_TCP_NOPUSH       1024
#define APR_RESET_NODELAY    2048 /**< This flag is ONLY set internally
                                   * when we set APR_TCP_NOPUSH with
                                   * APR_TCP_NODELAY set to tell us that
                                   * APR_TCP_NODELAY should be turned on
                                   * again when NOPUSH is turned off
                                   */
#define APR_INCOMPLETE_READ 4096  /**< Set on non-blocking sockets
				   * (APR_SO_TIMEOUT != 0) on which the
				   * previous read() did not fill a buffer
				   * completely.  the next apr_recv() will
				   * first call select()/poll() rather than
				   * going straight into read().  (Can also
				   * be set by an application to force a
				   * select()/poll() call before the next
				   * read, in cases where the app expects
				   * that an immediate read would fail.)
				   */

#define APR_POLLIN    0x001 
#define APR_POLLPRI   0x002
#define APR_POLLOUT   0x004
#define APR_POLLERR   0x010
#define APR_POLLHUP   0x020
#define APR_POLLNVAL  0x040
/** @} */

typedef enum {APR_SHUTDOWN_READ, APR_SHUTDOWN_WRITE, 
	      APR_SHUTDOWN_READWRITE} apr_shutdown_how_e;

#if (!APR_HAVE_IN_ADDR)
/**
 * We need to make sure we always have an in_addr type, so APR will just
 * define it ourselves, if the platform doesn't provide it.
 */
struct in_addr {
    apr_uint32_t  s_addr; /**< storage to hold the IP# */
}
#endif

/**
 * @def APR_INET
 * Not all platforms have these defined, so we'll define them here
 * The default values come from FreeBSD 4.1.1
 */
#define APR_INET     AF_INET
/** @def APR_UNSPEC
 * Let the system decide which address family to use
 */
#ifdef AF_UNSPEC
#define APR_UNSPEC   AF_UNSPEC
#else
#define APR_UNSPEC   0
#endif
#if APR_HAVE_IPV6
#define APR_INET6    AF_INET6
#endif

/**
 * Enum to tell us if we're interested in remote or local socket
 */
typedef enum {
    APR_LOCAL,
    APR_REMOTE
} apr_interface_e;

/* I guess not everybody uses inet_addr.  This defines apr_inet_addr
 * appropriately.
 */

#if APR_HAVE_INET_ADDR
#define apr_inet_addr    inet_addr
#elif APR_HAVE_INET_NETWORK        /* only DGUX, as far as I know */
/**
 * @warning
 * not generally safe... inet_network() and inet_addr() perform
 * different functions */
#define apr_inet_addr    inet_network
#endif

typedef struct apr_socket_t     apr_socket_t;
typedef struct apr_pollfd_t     apr_pollfd_t;
/**
 * A structure to encapsulate headers and trailers for apr_sendfile
 */
typedef struct apr_hdtr_t       apr_hdtr_t;
typedef struct in_addr          apr_in_addr_t;

/** @remark use apr_uint16_t just in case some system has a short that isn't 16 bits... */
typedef apr_uint16_t            apr_port_t;

/* It's defined here as I think it should all be platform safe...
 */
/**
 * APRs socket address type, used to ensure protocol independence
 */
typedef struct apr_sockaddr_t apr_sockaddr_t;

struct apr_sockaddr_t {
    /** The pool to use... */
    apr_pool_t *pool;
    /** The hostname */
    char *hostname;
    /** Either a string of the port number or the service name for the port */
    char *servname;
    /** The numeric port */
    apr_port_t port;
    /** The family */
    apr_int32_t family;
    union {
        /** IPv4 sockaddr structure */
        struct sockaddr_in sin;
#if APR_HAVE_IPV6
        /** IPv6 sockaddr structure */
        struct sockaddr_in6 sin6;
#endif
    } sa;
    /** How big is the sockaddr we're using? */
    apr_socklen_t salen;
    /** How big is the ip address structure we're using? */
    int ipaddr_len;
    /** How big should the address buffer be?  16 for v4 or 46 for v6
     *  used in inet_ntop... */
    int addr_str_len;
    /** This points to the IP address structure within the appropriate
     *  sockaddr structure.  */
    void *ipaddr_ptr;
    /** If multiple addresses were found by apr_sockaddr_info_get(), this 
     *  points to a representation of the next address. */
    apr_sockaddr_t *next;
};

#if APR_HAS_SENDFILE
/* Define flags passed in on apr_sendfile() */
#define APR_SENDFILE_DISCONNECT_SOCKET      1
#endif

/** A structure to encapsulate headers and trailers for apr_sendfile */
struct apr_hdtr_t {
    /** An iovec to store the headers sent before the file. 
     *  @defvar iovec *headers */
    struct iovec* headers;
    /** number of headers in the iovec */
    int numheaders;
    /** An iovec to store the trailers sent before the file. 
     *  @defvar iovec *trailers */
    struct iovec* trailers;
    /** number of trailers in the iovec */
    int numtrailers;
};

/** A structure to represent an IP subnet */
typedef struct apr_ipsubnet_t apr_ipsubnet_t;
struct apr_ipsubnet_t {
    int family;
#if APR_HAVE_IPV6
    apr_uint32_t sub[4]; /* big enough for IPv4 and IPv6 addresses */
    apr_uint32_t mask[4];
#else
    apr_uint32_t sub[1];
    apr_uint32_t mask[1];
#endif
};

/* function definitions */

/**
 * Create a socket.
 * @param new_sock The new socket that has been set up.
 * @param family The address family of the socket (e.g., APR_INET).
 * @param type The type of the socket (e.g., SOCK_STREAM).
 * @param cont The pool to use
 */
APR_DECLARE(apr_status_t) apr_socket_create(apr_socket_t **new_sock, 
                                            int family, int type,
                                            apr_pool_t *cont);

/**
 * Shutdown either reading, writing, or both sides of a tcp socket.
 * @param thesocket The socket to close 
 * @param how How to shutdown the socket.  One of:
 * <PRE>
 *            APR_SHUTDOWN_READ         no longer allow read requests
 *            APR_SHUTDOWN_WRITE        no longer allow write requests
 *            APR_SHUTDOWN_READWRITE    no longer allow read or write requests 
 * </PRE>
 * @remark This does not actually close the socket descriptor, it just
 *      controls which calls are still valid on the socket.
 */
APR_DECLARE(apr_status_t) apr_shutdown(apr_socket_t *thesocket,
                                       apr_shutdown_how_e how);

/**
 * Close a tcp socket.
 * @param thesocket The socket to close 
 */
APR_DECLARE(apr_status_t) apr_socket_close(apr_socket_t *thesocket);

/**
 * Bind the socket to its associated port
 * @param sock The socket to bind 
 * @param sa The socket address to bind to
 * @remark This may be where we will find out if there is any other process
 *      using the selected port.
 */
APR_DECLARE(apr_status_t) apr_bind(apr_socket_t *sock, apr_sockaddr_t *sa);

/**
 * Listen to a bound socket for connections.
 * @param sock The socket to listen on 
 * @param backlog The number of outstanding connections allowed in the sockets
 *                listen queue.  If this value is less than zero, the listen
 *                queue size is set to zero.  
 */
APR_DECLARE(apr_status_t) apr_listen(apr_socket_t *sock, apr_int32_t backlog);

/**
 * Accept a new connection request
 * @param new_sock A copy of the socket that is connected to the socket that
 *                 made the connection request.  This is the socket which should
 *                 be used for all future communication.
 * @param sock The socket we are listening on.
 * @param connection_pool The pool for the new socket.
 */
APR_DECLARE(apr_status_t) apr_accept(apr_socket_t **new_sock, 
                                     apr_socket_t *sock,
                                     apr_pool_t *connection_pool);

/**
 * Issue a connection request to a socket either on the same machine 
 * or a different one.
 * @param sock The socket we wish to use for our side of the connection 
 * @param sa The address of the machine we wish to connect to.  If NULL,
 *           APR assumes that the sockaddr_in in the apr_socket is 
 *           completely filled out.
 */
APR_DECLARE(apr_status_t) apr_connect(apr_socket_t *sock, apr_sockaddr_t *sa);

/**
 * Create apr_sockaddr_t from hostname, address family, and port.
 * @param sa The new apr_sockaddr_t.
 * @param hostname The hostname or numeric address string to resolve/parse.
 * @param family The address family to use, or APR_UNSPEC if the system should 
 *               decide.
 * @param port The port number.
 * @param flags Special processing flags.
 * @param p The pool for the apr_sockaddr_t and associated storage.
 */
APR_DECLARE(apr_status_t) apr_sockaddr_info_get(apr_sockaddr_t **sa,
                                          const char *hostname,
                                          apr_int32_t family,
                                          apr_port_t port,
                                          apr_int32_t flags,
                                          apr_pool_t *p);

/**
 * Look up the host name from an apr_sockaddr_t.
 * @param hostname The hostname.
 * @param sa The apr_sockaddr_t.
 * @param flags Special processing flags.
 */
APR_DECLARE(apr_status_t) apr_getnameinfo(char **hostname,
                                          apr_sockaddr_t *sa,
                                          apr_int32_t flags);
                             
/**
 * Parse hostname/IP address with scope id and port.
 *
 * Any of the following strings are accepted:
 *   8080                  (just the port number)
 *   www.apache.org        (just the hostname)
 *   www.apache.org:8080   (hostname and port number)
 *   [fe80::1]:80          (IPv6 numeric address string only)
 *   [fe80::1%eth0]        (IPv6 numeric address string and scope id)
 *
 * Invalid strings:
 *                         (empty string)
 *   [abc]                 (not valid IPv6 numeric address string)
 *   abc:65536             (invalid port number)
 *
 * @param addr The new buffer containing just the hostname.  On output, *addr 
 *             will be NULL if no hostname/IP address was specfied.
 * @param scope_id The new buffer containing just the scope id.  On output, 
 *                 *scope_id will be NULL if no scope id was specified.
 * @param port The port number.  On output, *port will be 0 if no port was 
 *             specified.
 * @param str The input string to be parsed.
 * @param p The pool from which *addr and *scope_id are allocated.
 * @remark If scope id shouldn't be allowed, check for scope_id != NULL in 
 *         addition to checking the return code.  If addr/hostname should be 
 *         required, check for addr == NULL in addition to checking the 
 *         return code.
 */
APR_DECLARE(apr_status_t) apr_parse_addr_port(char **addr,
                                              char **scope_id,
                                              apr_port_t *port,
                                              const char *str,
                                              apr_pool_t *p);

/**
 * Get name of the current machine
 * @param buf A buffer to store the hostname in.
 * @param len The maximum length of the hostname that can be stored in the
 *            buffer provided. 
 * @param cont The pool to use.
 */
APR_DECLARE(apr_status_t) apr_gethostname(char *buf, int len, apr_pool_t *cont);

/**
 * Return the data associated with the current socket
 * @param data The user data associated with the socket.
 * @param key The key to associate with the user data.
 * @param sock The currently open socket.
 */
APR_DECLARE(apr_status_t) apr_socket_data_get(void **data, const char *key,
                                             apr_socket_t *sock);

/**
 * Set the data associated with the current socket.
 * @param sock The currently open socket.
 * @param data The user data to associate with the socket.
 * @param key The key to associate with the data.
 * @param cleanup The cleanup to call when the socket is destroyed.
 */
APR_DECLARE(apr_status_t) apr_socket_data_set(apr_socket_t *sock, void *data,
                                             const char *key,
                                             apr_status_t (*cleanup)(void*));

/**
 * Send data over a network.
 * @param sock The socket to send the data over.
 * @param buf The buffer which contains the data to be sent. 
 * @param len On entry, the number of bytes to send; on exit, the number
 *            of bytes sent.
 * @remark
 * <PRE>
 * This functions acts like a blocking write by default.  To change 
 * this behavior, use apr_setsocketopt with the APR_SO_TIMEOUT option.
 *
 * It is possible for both bytes to be sent and an error to be returned.
 *
 * APR_EINTR is never returned.
 * </PRE>
 */
APR_DECLARE(apr_status_t) apr_send(apr_socket_t *sock, const char *buf, 
                                   apr_size_t *len);

/**
 * Send multiple packets of data over a network.
 * @param sock The socket to send the data over.
 * @param vec The array of iovec structs containing the data to send 
 * @param nvec The number of iovec structs in the array
 * @param len Receives the number of bytes actually written
 * @remark
 * <PRE>
 * This functions acts like a blocking write by default.  To change 
 * this behavior, use apr_setsocketopt with the APR_SO_TIMEOUT option.
 * The number of bytes actually sent is stored in argument 3.
 *
 * It is possible for both bytes to be sent and an error to be returned.
 *
 * APR_EINTR is never returned.
 * </PRE>
 */
APR_DECLARE(apr_status_t) apr_sendv(apr_socket_t *sock, 
                                    const struct iovec *vec,
                                    apr_int32_t nvec, apr_size_t *len);

/**
 * @param sock The socket to send from
 * @param where The apr_sockaddr_t describing where to send the data
 * @param data The data to send
 * @param len  The length of the data to send
 */
APR_DECLARE(apr_status_t) apr_sendto(apr_socket_t *sock, apr_sockaddr_t *where,
                                     apr_int32_t flags, const char *buf, 
                                     apr_size_t *len);

/**
 * @param from The apr_sockaddr_t to fill in the recipient info
 * @param sock The socket to use
 * @param buf  The buffer to use
 * @param len  The length of the available buffer
 */

APR_DECLARE(apr_status_t) apr_recvfrom(apr_sockaddr_t *from, apr_socket_t *sock,
                                       apr_int32_t flags, char *buf, 
                                       apr_size_t *len);
 
#if APR_HAS_SENDFILE

/**
 * Send a file from an open file descriptor to a socket, along with 
 * optional headers and trailers
 * @param sock The socket to which we're writing
 * @param file The open file from which to read
 * @param hdtr A structure containing the headers and trailers to send
 * @param offset Offset into the file where we should begin writing
 * @param len (input)  - Number of bytes to send from the file 
 *            (output) - Number of bytes actually sent, 
 *                       including headers, file, and trailers
 * @param flags APR flags that are mapped to OS specific flags
 * @remark This functions acts like a blocking write by default.  To change 
 *         this behavior, use apr_setsocketopt with the APR_SO_TIMEOUT option.
 *         The number of bytes actually sent is stored in argument 5.
 */
APR_DECLARE(apr_status_t) apr_sendfile(apr_socket_t *sock, apr_file_t *file,
                                       apr_hdtr_t *hdtr, apr_off_t *offset,
                                       apr_size_t *len, apr_int32_t flags);

#endif /* APR_HAS_SENDFILE */

/**
 * Read data from a network.
 * @param sock The socket to read the data from.
 * @param buf The buffer to store the data in. 
 * @param len On entry, the number of bytes to receive; on exit, the number
 *            of bytes received.
 * @remark
 * <PRE>
 * This functions acts like a blocking read by default.  To change 
 * this behavior, use apr_setsocketopt with the APR_SO_TIMEOUT option.
 * The number of bytes actually sent is stored in argument 3.
 *
 * It is possible for both bytes to be received and an APR_EOF or
 * other error to be returned.
 *
 * APR_EINTR is never returned.
 * </PRE>
 */
APR_DECLARE(apr_status_t) apr_recv(apr_socket_t *sock, 
                                   char *buf, apr_size_t *len);

/**
 * Setup socket options for the specified socket
 * @param sock The socket to set up.
 * @param opt The option we would like to configure.  One of:
 * <PRE>
 *            APR_SO_DEBUG      --  turn on debugging information 
 *            APR_SO_KEEPALIVE  --  keep connections active
 *            APR_SO_LINGER     --  lingers on close if data is present
 *            APR_SO_NONBLOCK   --  Turns blocking on/off for socket
 *            APR_SO_REUSEADDR  --  The rules used in validating addresses
 *                                  supplied to bind should allow reuse
 *                                  of local addresses.
 *            APR_SO_TIMEOUT    --  Set the timeout value in microseconds.
 *                                  values < 0 mean wait forever.  0 means
 *                                  don't wait at all.
 *            APR_SO_SNDBUF     --  Set the SendBufferSize
 *            APR_SO_RCVBUF     --  Set the ReceiveBufferSize
 * </PRE>
 * @param on Value for the option.
 */
APR_DECLARE(apr_status_t) apr_setsocketopt(apr_socket_t *sock,
                                           apr_int32_t opt, apr_int32_t on);

/**
 * Query socket options for the specified socket
 * @param sock The socket to query
 * @param opt The option we would like to query.  One of:
 * <PRE>
 *            APR_SO_DEBUG      --  turn on debugging information 
 *            APR_SO_KEEPALIVE  --  keep connections active
 *            APR_SO_LINGER     --  lingers on close if data is present
 *            APR_SO_NONBLOCK   --  Turns blocking on/off for socket
 *            APR_SO_REUSEADDR  --  The rules used in validating addresses
 *                                  supplied to bind should allow reuse
 *                                  of local addresses.
 *            APR_SO_TIMEOUT    --  Set the timeout value in microseconds.
 *                                  values < 0 mean wait forever.  0 means
 *                                  don't wait at all.
 *            APR_SO_SNDBUF     --  Set the SendBufferSize
 *            APR_SO_RCVBUF     --  Set the ReceiveBufferSize
 *            APR_SO_DISCONNECTED -- Query the disconnected state of the socket.
 *                                  (Currently only used on Windows)
 * </PRE>
 * @param on Socket option returned on the call.
 */
APR_DECLARE(apr_status_t) apr_getsocketopt(apr_socket_t *sock, 
                                           apr_int32_t opt, apr_int32_t *on);

/**
 * Return an apr_sockaddr_t from an apr_socket_t
 * @param sa The returned apr_sockaddr_t.
 * @param which Which interface do we want the apr_sockaddr_t for?
 * @param sock The socket to use
 */
APR_DECLARE(apr_status_t) apr_socket_addr_get(apr_sockaddr_t **sa,
                                           apr_interface_e which,
                                           apr_socket_t *sock);
 
/**
 * Set the port in an APR socket address.
 * @param sockaddr The socket address to set.
 * @param port The port to be stored in the socket address.
 */
APR_DECLARE(apr_status_t) apr_sockaddr_port_set(apr_sockaddr_t *sockaddr,
                                       apr_port_t port);

/**
 * Return the port in an APR socket address.
 * @param port The port from the socket address.
 * @param sockaddr The socket address to reference.
 */
APR_DECLARE(apr_status_t) apr_sockaddr_port_get(apr_port_t *port,
                                       apr_sockaddr_t *sockaddr);

/**
 * Set the IP address in an APR socket address.
 * @param sockaddr The socket address to use 
 * @param addr The IP address to attach to the socket.
 *             Use APR_ANYADDR to use any IP addr on the machine.
 */
APR_DECLARE(apr_status_t) apr_sockaddr_ip_set(apr_sockaddr_t *sockaddr,
                                         const char *addr);

/**
 * Return the IP address (in numeric address string format) in
 * an APR socket address.
 * @param addr The IP address.
 * @param sockaddr The socket address to reference.
 */
APR_DECLARE(apr_status_t) apr_sockaddr_ip_get(char **addr, 
                                         apr_sockaddr_t *sockaddr);

/**
 * Setup the memory required for poll to operate properly
 * @param new_poll The poll structure to be used. 
 * @param num The number of socket descriptors to be polled.
 * @param cont The pool to operate on.
 */
APR_DECLARE(apr_status_t) apr_poll_setup(apr_pollfd_t **new_poll, 
                                         apr_int32_t num,
                                         apr_pool_t *cont);

/**
 * Poll the sockets in the poll structure
 * @param aprset The poll structure we will be using. 
 * @param nsds The number of sockets we are polling. 
 * @param timeout The amount of time in microseconds to wait.  This is 
 *                a maximum, not a minimum.  If a socket is signalled, we 
 *                will wake up before this time.  A negative number means 
 *                wait until a socket is signalled.
 * @remark
 * <PRE>
 * The number of sockets signalled is returned in the second argument. 
 *
 *        This is a blocking call, and it will not return until either a 
 *        socket has been signalled, or the timeout has expired. 
 * </PRE>
 */
APR_DECLARE(apr_status_t) apr_poll(apr_pollfd_t *aprset, apr_int32_t *nsds, 
                                   apr_interval_time_t timeout);

/**
 * Add a socket to the poll structure.
 * @param aprset The poll structure we will be using. 
 * @param socket The socket to add to the current poll structure. 
 * @param event The events to look for when we do the poll.  One of:
 * <PRE>
 *            APR_POLLIN       signal if read will not block
 *            APR_POLLPRI      signal if prioirty data is availble to be read
 *            APR_POLLOUT      signal if write will not block
 * </PRE>
 */
APR_DECLARE(apr_status_t) apr_poll_socket_add(apr_pollfd_t *aprset, 
                                              apr_socket_t *sock,
                                              apr_int16_t event);

/**
 * Modify a socket in the poll structure with mask.
 * @param aprset The poll structure we will be using. 
 * @param sock The socket to modify in poll structure. 
 * @param events The events to stop looking for during the poll.  One of:
 * <PRE>
 *            APR_POLLIN       signal if read will not block
 *            APR_POLLPRI      signal if prioirty data is availble to be read
 *            APR_POLLOUT      signal if write will not block
 * </PRE>
 */
APR_DECLARE(apr_status_t) apr_poll_socket_mask(apr_pollfd_t *aprset,
                                               apr_socket_t *sock,
                                               apr_int16_t events);
/**
 * Remove a socket from the poll structure.
 * @param aprset The poll structure we will be using. 
 * @param sock The socket to remove from the current poll structure. 
 */
APR_DECLARE(apr_status_t) apr_poll_socket_remove(apr_pollfd_t *aprset, 
                                                 apr_socket_t *sock);

/**
 * Remove all sockets from the poll structure.
 * @param aprset The poll structure we will be using. 
 * @param events The events to clear from all sockets.  One of:
 * <PRE>
 *            APR_POLLIN       signal if read will not block
 *            APR_POLLPRI      signal if prioirty data is availble to be read
 *            APR_POLLOUT      signal if write will not block
 * </PRE>
 */
APR_DECLARE(apr_status_t) apr_poll_socket_clear(apr_pollfd_t *aprset, 
                                                 apr_int16_t events);

/**
 * Get the return events for the specified socket.
 * @param event The returned events for the socket.  One of:
 * <PRE>
 *            APR_POLLIN       Data is available to be read 
 *            APR_POLLPRI      Prioirty data is availble to be read
 *            APR_POLLOUT      Write will succeed
 *            APR_POLLERR      An error occurred on the socket
 *            APR_POLLHUP      The connection has been terminated
 *            APR_POLLNVAL     This is an invalid socket to poll on.
 *                             Socket not open.
 * </PRE>
 * @param sock The socket we wish to get information about. 
 * @param aprset The poll structure we will be using. 
 */
APR_DECLARE(apr_status_t) apr_poll_revents_get(apr_int16_t *event, 
                                          apr_socket_t *sock,
                                          apr_pollfd_t *aprset);

/**
 * Return the data associated with the current poll.
 * @param pollfd The currently open pollfd.
 * @param key The key to use for retreiving data associated with a poll struct.
 * @param data The user data associated with the pollfd.
 */
APR_DECLARE(apr_status_t) apr_poll_data_get(apr_pollfd_t *pollfd, 
                                           const char *key, void *data);

/**
 * Set the data associated with the current poll.
 * @param pollfd The currently open pollfd.
 * @param data The key to associate with the data.
 * @param key The user data to associate with the pollfd.
 * @param cleanup The cleanup function
 */
APR_DECLARE(apr_status_t) apr_poll_data_set(apr_pollfd_t *pollfd, void *data,
                                           const char *key,
                                           apr_status_t (*cleanup)(void *));

#if APR_FILES_AS_SOCKETS

/**
 * Convert a File type to a socket so that it can be used in a poll operation.
 * @param newsock the newly created socket which represents a file.
 * @param file the file to mask as a socket.
 * @warning This is not available on all platforms.  Platforms that have the
 *      ability to poll files for data to be read/written/exceptions will
 *      have the APR_FILES_AS_SOCKETS macro defined as true.
 */
APR_DECLARE(apr_status_t) apr_socket_from_file(apr_socket_t **newsock,
                                               apr_file_t *file);

#endif /* APR_FILES_AS_SOCKETS */

/**
 * Given an apr_sockaddr_t and a service name, set the port for the service
 * @param sockaddr The apr_sockaddr_t that will have it's port set
 * @param servname The name of the service you wish to use
 */
APR_DECLARE(apr_status_t) apr_getservbyname(apr_sockaddr_t *sockaddr, 
                                            const char *servname);

/**
 * Build an ip-subnet representation from an IP address and optional netmask or
 * number-of-bits.
 * @param ipsub The new ip-subnet representation
 * @param ipstr The input IP address string
 * @param mask_or_numbits The input netmask or number-of-bits string, or NULL
 * @param p The pool to allocate from
 */
APR_DECLARE(apr_status_t) apr_ipsubnet_create(apr_ipsubnet_t **ipsub, const char *ipstr, 
                                              const char *mask_or_numbits, apr_pool_t *p);

/**
 * Test the IP address in an apr_sockaddr_t against a pre-built ip-subnet
 * representation.
 * @param ipsub The ip-subnet representation
 * @param sa The socket address to test
 * @return non-zero if the socket address is within the subnet, 0 otherwise
 */
APR_DECLARE(int) apr_ipsubnet_test(apr_ipsubnet_t *ipsub, apr_sockaddr_t *sa);

#if APR_HAS_SO_ACCEPTFILTER
/**
 * Set an OS level accept filter.
 * @param sock The socket to put the accept filter on.
 * @param name The accept filter
 * @param args Any extra args to the accept filter.  Passing NULL here removes
 *             the accept filter. 
 */
apr_status_t apr_socket_accept_filter(apr_socket_t *sock, char *name,
                                      char *args);
#endif

/**
 * Set a socket to be inherited by child processes.
 * @param socket The socket to enable inheritance.
 */
APR_DECLARE(void) apr_socket_set_inherit(apr_socket_t *skt);

/**
 * Unset a socket from being inherited by child processes.
 * @param socket The socket to disable inheritance.
 */
APR_DECLARE(void) apr_socket_unset_inherit(apr_socket_t *skt);

#ifdef __cplusplus
}
#endif
/** @} */
#endif  /* ! APR_NETWORK_IO_H */

