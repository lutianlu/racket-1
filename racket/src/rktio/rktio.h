#ifndef __RKTIO_H__
#define __RKTIO_H__

#include "rktio_config.h"

#ifndef RKTIO_EXTERN
# define RKTIO_EXTERN extern
#endif

typedef struct rktio_t rktio_t;
/* A rktio_t value represents an instance of the Racket I/O system.
   Almost every rktio_...() function takes it as the first argument. */

RKTIO_EXTERN rktio_t *rktio_init(void);
/* Call rktio_init() before anything else. The first call to
   rktio_init() must return before any additional calls (in other
   threads), but there's no ordering requirement after that. */

RKTIO_EXTERN void rktio_destroy(rktio_t *);
/* Call rktio_destroy() as the last thing. Everything else must be
   explicitly deallocated/closed/forgotten before calling
   rktio_destroy(). */

RKTIO_EXTERN void rktio_free(void *p);
/* Normally equivalent to free(), but ensures the same malloc()/free()
   that rktio function use: */

typedef int rktio_ok_t;
/* A result of this type is 0 for failure (in which case an error is
   available from `rktio_get_last_error`) and 1 for success. */

typedef int rktio_tri_t;
/* A result of this type is a boolean, but a `...ERROR` value means
   that an error value is available from `rktio_get_last_error`. */

typedef int rktio_bool_t;
/* 0 or 1. */

/*************************************************/
/* Reading and writing files                     */

typedef struct rktio_fd_t rktio_fd_t;

/* Mode flags shared in part by `rktio_open` and `rktio_system_fd`. */

/* Accepted by both, but `RKTIO_OPEN_READ` and `RKTIO_OPEN_WRITE` are
   merely advisory for `rktio_system_fd` */
#define RKTIO_OPEN_READ        (1<<0)
#define RKTIO_OPEN_WRITE       (1<<1)
#define RKTIO_OPEN_TEXT        (1<<2)

/* Used for `rktio_open` with `RKTIO_OPEN_WRITE`: */
#define RKTIO_OPEN_TRUNCATE    (1<<3)
#define RKTIO_OPEN_APPEND      (1<<4)
#define RKTIO_OPEN_REPLACE     (1<<5)
#define RKTIO_OPEN_MUST_EXIST  (1<<6)
#define RKTIO_OPEN_CAN_EXIST   (1<<7)

/* Used for `rktio_system_fd`: */
#define RKTIO_OPEN_SOCKET      (1<<8)
#define RKTIO_OPEN_UDP         (1<<9)
#define RKTIO_OPEN_REGFILE     (1<<10)
#define RKTIO_OPEN_NOT_REGFILE (1<<11)
/* If neither RKTIO_OPEN_REGILE nor RKTIO_OPEN_NOT_REGILE
   are specified, then the value is inferred by `rtkio_system_fd`. */
#define RKTIO_OPEN_INIT        (1 << 12)
/* Make `rtkio_system_fd` set a socket as nonblocking, etc. */
#define RKTIO_OPEN_OWN         (1 << 13)
/* Make `rtkio_system_fd` record a socket for reliable clean up on pre-NT Windows. */

RKTIO_EXTERN rktio_fd_t *rktio_system_fd(rktio_t *rktio, intptr_t system_fd, int modes);
/* A socket (as opposed to other file descriptors) registered this way
   should include include `RKTIO_OPEN_SOCKET` and be non-blocking or
   use `RKTIO_OPEN_INIT`. */

RKTIO_EXTERN intptr_t rktio_fd_system_fd(rktio_t *rktio, rktio_fd_t *rfd);
/* Extracts a native file descriptor or socket. */

RKTIO_EXTERN rktio_bool_t rktio_fd_is_regular_file(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN rktio_bool_t rktio_fd_is_socket(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN rktio_bool_t rktio_fd_is_udp(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN rktio_bool_t rktio_fd_is_terminal(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN rktio_bool_t rktio_fd_is_text_converted(rktio_t *rktio, rktio_fd_t *rfd);
/* The functions mostly report values of recorded mode flags. */

RKTIO_EXTERN int rktio_fd_modes(rktio_t *rktio, rktio_fd_t *rfd);
/* Returns all of the recorded mode flags. */

RKTIO_EXTERN rktio_fd_t *rktio_open(rktio_t *rktio, const char *src, int modes);
/* Can report `RKTIO_ERROR_DOES_NOT_EXIST` in place of system error,
   and can report `RKTIO_ERROR_UNSUPPORTED_TEXT_MODE` on Windows:=. */

RKTIO_EXTERN rktio_ok_t rktio_close(rktio_t *rktio, rktio_fd_t *fd);
/* Can report `RKTIO_ERROR_EXISTS` in place of system error,
   and can report `RKTIO_ERROR_UNSUPPORTED_TEXT_MODE` on Windows.
   See also `rktio_write` and `rktio_poll_write_flushed. */

RKTIO_EXTERN rktio_fd_t *rktio_dup(rktio_t *rktio, rktio_fd_t *rfd);
/* Copies a file descriptor, where each must be closed or forgotten
   independenty. */

RKTIO_EXTERN void rktio_forget(rktio_t *rktio, rktio_fd_t *fd);
/* Deallocates a `rktio_fd_t` without closing the file descriptor,
   but the descriptor is no longer recorded if it was opened with
   `RKTIO_OPEN_OWN`. */

RKTIO_EXTERN rktio_fd_t *rktio_std_fd(rktio_t *rktio, int which);
/* Gets stdin/stdout/stderr.
   `which` values: */
#define RKTIO_STDIN  0
#define RKTIO_STDOUT 1
#define RKTIO_STDERR 2

RKTIO_EXTERN intptr_t rktio_read(rktio_t *rktio, rktio_fd_t *fd, char *buffer, intptr_t len);
/* Returns the number of bytes read, possibly 0, in non-blocking mode.
   Alternatively, the result can be `RKTIO_READ_EOF` for end-of-file
   or `RKTIO_READ_ERROR` for an error. Although rktio_read is intended
   to have no buffering, text-mode conversion (on Windows) and certain
   uncooperative OS corners can buffer 1 byte. */

#define RKTIO_READ_EOF   (-1)
#define RKTIO_READ_ERROR (-2)

RKTIO_EXTERN intptr_t rktio_write(rktio_t *rktio, rktio_fd_t *fd, const char *buffer, intptr_t len);
/* Returns the number of bytes written, possibly 0, in non-blocking
   mode. Alternatively, the result can be `RKTIO_WRITE_ERROR` for an
   error. Although rktio_write() is intended to write only bytes that
   can be fully delivered to the OS, there may be OS limitations that
   require buffering (e.g., on Windows). Use
   rktio_poll_write_flushed() to make sure it's completely flushed
   before closing. */

#define RKTIO_WRITE_ERROR (-2)

RKTIO_EXTERN intptr_t rktio_read_converted(rktio_t *rktio, rktio_fd_t *fd, char *buffer, intptr_t len,
                                           char *is_converted);
/* Like rktio_read(), but also reports whether each character was
   original two characters that were converted to a single newline for
   text mode. */

RKTIO_EXTERN intptr_t rktio_buffered_byte_count(rktio_t *rktio, rktio_fd_t *fd);
/* Reports the number of bytes that are buffered kfrom the file descriptor.
   The result is normally zero, but text-mode conversion and the rare
   uncooperative corner of an OS can make the result 1 byte. */

RKTIO_EXTERN rktio_tri_t rktio_poll_read_ready(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN rktio_tri_t rktio_poll_write_ready(rktio_t *rktio, rktio_fd_t *rfd);
/* Each polling function returns one of the following: */
#define RKTIO_POLL_NOT_READY 0
#define RKTIO_POLL_READY 1
#define RKTIO_POLL_ERROR (-2)

RKTIO_EXTERN rktio_tri_t rktio_poll_write_flushed(rktio_t *rktio, rktio_fd_t *rfd);
/* See `rktio_write` above. */

RKTIO_EXTERN rktio_tri_t rktio_file_lock_try(rktio_t *rktio, rktio_fd_t *rfd, int excl);
RKTIO_EXTERN rktio_ok_t rktio_file_unlock(rktio_t *rktio, rktio_fd_t *rfd);
/* Advisory file locks, where `excl` attempts to claim an exclusive
   lock. Whether these work in various situations depend on many OS
   details, where the differences involve promoting from non-exlcusive
   to exclusive, taking a lock that is already held, getting an
   exclusive lock for a file descriptor in read mode, getting a
   non-exclusive lock in write mode, and whether a lock prevents
   opening or using another file descriptor. */

#define RKTIO_LOCK_ERROR        (-2)
#define RKTIO_LOCK_ACQUIRED     1
#define RKTIO_LOCK_NOT_ACQUIRED 0

typedef rktio_int64_t rktio_filesize_t;

rktio_ok_t rktio_set_file_position(rktio_t *rktio, rktio_fd_t *rfd, rktio_filesize_t pos, int whence);
/* Can report `RKTIO_ERROR_CANNOT_SET_FILE_POSITION` on Windows. */
/* For `whence`: */
enum {
  RKTIO_POSITION_FROM_START,
  RKTIO_POSITION_FROM_END
};

rktio_filesize_t *rktio_get_file_position(rktio_t *rktio, rktio_fd_t *rfd);
/* Returns the file position, not taking into account rare input
   buffering (see `rktio_read`). On Windows, can report
   `RKTIO_ERROR_CANNOT_SET_FILE_POSITION`, which doesn't have a
   corresponding Windows error code. */

rktio_ok_t rktio_set_file_size(rktio_t *rktio, rktio_fd_t *rfd, rktio_filesize_t sz);
/* Can report `RKTIO_ERROR_CANNOT_SET_FILE_POSITION` on Windows. */

/*************************************************/
/* Network                                       */

typedef struct rktio_addrinfo_lookup_t rktio_addrinfo_lookup_t;
typedef struct rktio_addrinfo_t rktio_addrinfo_t;

RKTIO_EXTERN rktio_addrinfo_lookup_t *rktio_start_addrinfo_lookup(rktio_t *rktio,
                                                                  const char *hostname, int portno,
                                                                  int family, rktio_bool_t passive, rktio_bool_t tcp);
/* The `family` argument should be one of the following: */
#define RKTIO_FAMILY_ANY (-1)
RKTIO_EXTERN int rktio_get_ipv4_family(rktio_t *rktio);

RKTIO_EXTERN rktio_tri_t rktio_poll_addrinfo_lookup_ready(rktio_t *rktio, rktio_addrinfo_lookup_t *lookup);
/* Check whether an address is available for a lookup request. */

RKTIO_EXTERN rktio_addrinfo_t *rktio_addrinfo_lookup_get(rktio_t *rktio, rktio_addrinfo_lookup_t *lookup);
/* Deallocates `lookup`. */

RKTIO_EXTERN void rktio_addrinfo_lookup_stop(rktio_t *rktio, rktio_addrinfo_lookup_t *lookup);
/* Abandons a lookup whose result (or error) is not yet received. */

RKTIO_EXTERN void rktio_addrinfo_free(rktio_t *rktio, struct rktio_addrinfo_t *a);
/* Frees the result of a lookup. */

typedef struct rktio_listener_t rktio_listener_t;
typedef struct rktio_connect_t rktio_connect_t;

RKTIO_EXTERN rktio_listener_t *rktio_listen(rktio_t *rktio, rktio_addrinfo_t *local, int backlog, rktio_bool_t reuse);
/* Can fail with `RKTIO_ERROR_TRY_AGAIN_WITH_IPV4`, which suggests
   trying an address using the family reported by
   `rktio_get_ipv4_family` instead of `RKTIO_FAMILY_ANY`. */

RKTIO_EXTERN void rktio_listen_stop(rktio_t *rktio, rktio_listener_t *l);
/* Stops a listener. */

RKTIO_EXTERN rktio_tri_t rktio_poll_accept_ready(rktio_t *rktio, rktio_listener_t *listener);
/* Returns one of `RKTIO_POLL_READY`, etc. */

RKTIO_EXTERN rktio_fd_t *rktio_accept(rktio_t *rktio, rktio_listener_t *listener);
/* Accepts one connection on a listener. */

RKTIO_EXTERN rktio_connect_t *rktio_start_connect(rktio_t *rktio, rktio_addrinfo_t *remote, rktio_addrinfo_t *local);
/* Starts a connection request. Addreses must not be freed until the
   connection is complete, errored, or stopped. */

RKTIO_EXTERN rktio_fd_t *rktio_connect_finish(rktio_t *rktio, rktio_connect_t *conn);
/* A `RKTIO_ERROR_CONNECT_TRYING_NEXT` error effectively means "try
   again", and the connection object is still valid. On any other
   error, or if the connection completes successfully, `conn` is
   deallocated */

RKTIO_EXTERN void rktio_connect_stop(rktio_t *rktio, rktio_connect_t *conn);
/* Stops a connection whose result or error has not been received. */

RKTIO_EXTERN rktio_tri_t rktio_poll_connect_ready(rktio_t *rktio, rktio_connect_t *conn);
/* Returns one of `RKTIO_POLL_READY`, etc. */

RKTIO_EXTERN rktio_fd_t *rktio_connect_trying(rktio_t *rktio, rktio_connect_t *conn);
/* Returns a file descriptor that `conn` is currently trying, or
   returns NULL without setting any error. The result file descriptor
   should not be closed, and may be closed by a `rktio_connect_finish`
   or `rktio_connect_stop` call (so if you register it in an long-term
   poll set, unregister it before trying to finish or stop the
   connection). */

RKTIO_EXTERN int rktio_socket_shutdown(rktio_t *rktio, rktio_fd_t *rfd, int mode);
/* Useful for TCP to report an EOF to the other end. Does not close the socket,
   but may make it ineligible for forther use.
   `mode` values: */
#define RKTIO_SHUTDOWN_READ   0
#define RKTIO_SHUTDOWN_WRITE  1

RKTIO_EXTERN rktio_fd_t *rktio_udp_open(rktio_t *rktio, rktio_addrinfo_t *addr, int family);
/* The `addr` argument can be NULL to create a socket without
   specifying an interface, and `family` is used only if `addr` is not
   specified. */

RKTIO_EXTERN rktio_ok_t rktio_udp_disconnect(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN rktio_ok_t rktio_udp_bind(rktio_t *rktio, rktio_fd_t *rfd, rktio_addrinfo_t *addr,
                                       rktio_bool_t reuse);
RKTIO_EXTERN rktio_ok_t rktio_udp_connect(rktio_t *rktio, rktio_fd_t *rfd, rktio_addrinfo_t *addr);

RKTIO_EXTERN intptr_t rktio_udp_sendto(rktio_t *rktio, rktio_fd_t *rfd, rktio_addrinfo_t *addr,
                                       const char *buffer, intptr_t len);
/* Extends `rktio_write` to accept a destination `addr`, and binds `rfd` if it 
   is not bound aready. The `addr` can be NULL if the socket is connected. */

typedef struct rktio_length_and_addrinfo_t {
  intptr_t len;
  char **address; /* like the result of `rktio_socket_address` */
} rktio_length_and_addrinfo_t;

RKTIO_EXTERN rktio_length_and_addrinfo_t *rktio_udp_recvfrom(rktio_t *rktio, rktio_fd_t *rfd,
                                                             char *buffer, intptr_t len);
/* Extend `rktio_read` to report the sender. If the reported error can
   be `RKTIO_ERROR_TRY_AGAIN` or `RKTIO_ERROR_INFO_TRY_AGAIN`, where
   the latter can happen if the sock claims to be ready to read. */

/* The following accessors return `RKTIO_PROP_ERROR` on failure */
RKTIO_EXTERN rktio_tri_t rktio_udp_get_multicast_loopback(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN rktio_ok_t rktio_udp_set_multicast_loopback(rktio_t *rktio, rktio_fd_t *rfd, rktio_bool_t on);
RKTIO_EXTERN rktio_tri_t rktio_udp_get_multicast_ttl(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN rktio_ok_t rktio_udp_set_multicast_ttl(rktio_t *rktio, rktio_fd_t *rfd, int ttl_val);

#define RKTIO_PROP_ERROR (-2)

RKTIO_EXTERN char **rktio_socket_address(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN char **rktio_socket_peer_address(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN char **rktio_listener_address(rktio_t *rktio, rktio_listener_t *lnr);
/* These return two strings in an array (where the array itself should
   be deallocated): address and service. */

RKTIO_EXTERN char *rktio_udp_multicast_interface(rktio_t *rktio, rktio_fd_t *rfd);
RKTIO_EXTERN rktio_ok_t rktio_udp_set_multicast_interface(rktio_t *rktio, rktio_fd_t *rfd, rktio_addrinfo_t *addr);
/* The `addr` argument can be NULL to auto-select the interface. */

RKTIO_EXTERN rktio_ok_t rktio_udp_change_multicast_group(rktio_t *rktio, rktio_fd_t *rfd,
                                                         rktio_addrinfo_t *group_addr,
                                                         rktio_addrinfo_t *intf_addr,
                                                         int action);
/* `action` values: */
enum {
  RKTIO_ADD_MEMBERSHIP,
  RKTIO_DROP_MEMBERSHIP
};

/*************************************************/
/* Environment variables                         */

RKTIO_EXTERN rktio_bool_t rktio_is_ok_envvar_name(rktio_t *rktio, const char *name);
/* Checks whether a string is valid as a new (e.g., no "="). */

RKTIO_EXTERN rktio_bool_t rktio_are_envvar_names_case_insensitive(rktio_t *rktio);
/* Checks whether environment variables are case-folded by the OS.
   That doesn't mean that clients need to case-fold names, but clients
   may want to immitate the OS. */

RKTIO_EXTERN char *rktio_getenv(rktio_t *rktio, const char *name);
/* Gets an environment variable value, or reports
   `RKTIO_ERROR_NO_SUCH_ENVVAR` when returning NULL; the result must
   be freed. */

RKTIO_EXTERN rktio_ok_t rktio_setenv(rktio_t *rktio, const char *name, const char *val);
/* Set an environment variable's value, where a NULL value for `val`
   unsets it. */

typedef struct rktio_envvars_t rktio_envvars_t;

RKTIO_EXTERN rktio_envvars_t *rktio_envvars(rktio_t *rktio);
/* Extracts all environment variables into a record */

RKTIO_EXTERN rktio_envvars_t *rktio_empty_envvars(rktio_t *rktio);
/* Create an empty environment-variables record. */

RKTIO_EXTERN rktio_envvars_t *rktio_envvars_copy(rktio_t *rktio, rktio_envvars_t *envvars);
/* Clones an environment-variable record. */

RKTIO_EXTERN void rktio_envvars_free(rktio_t *rktio, rktio_envvars_t *envvars);
/* Deallocates an environment-variables record: */

RKTIO_EXTERN char *rktio_envvars_get(rktio_t *rktio, rktio_envvars_t *envvars, const char *name);
RKTIO_EXTERN void rktio_envvars_set(rktio_t *rktio, rktio_envvars_t *envvars, const char *name, const char *value);
/* Access/update environment-variables record by name. */

RKTIO_EXTERN intptr_t rktio_envvars_count(rktio_t *rktio, rktio_envvars_t *envvars);
RKTIO_EXTERN char *rktio_envvars_name_ref(rktio_t *rktio, rktio_envvars_t *envvars, intptr_t i);
RKTIO_EXTERN char *rktio_envvars_value_ref(rktio_t *rktio, rktio_envvars_t *envvars, intptr_t i);
/* Access/update environment-variables record by index. */

/*************************************************/
/* Processes                                     */

typedef struct rktio_process_t rktio_process_t;

typedef struct rktio_process_result_t {
  rktio_process_t *process;
  rktio_fd_t *stdin_fd, *stdout_fd, *stderr_fd;
} rktio_process_result_t;

RKTIO_EXTERN rktio_process_result_t *rktio_process(rktio_t *rktio,
                                                   const char *command, int argc, char **argv,
                                                   rktio_fd_t *stdout_fd, rktio_fd_t *stdin_fd, rktio_fd_t *stderr_fd,
                                                   const char *current_directory, rktio_envvars_t *envvars,
                                                   int flags,
                                                   void (*unix_child_process_callback)());
/* `flags` flags: */
#define RKTIO_PROCESS_NEW_GROUP                 (1<<0)
#define RKTIO_PROCESS_STDOUT_AS_STDERR          (1<<1)
#define RKTIO_PROCESS_WINDOWS_EXACT_CMDLINE     (1<<2)
#define RKTIO_PROCESS_WINDOWS_CHAIN_TERMINATION (1<<3)

RKTIO_EXTERN int rktio_process_allowed_flags(rktio_t *rktio);
/* Reports the flags that are accepted by `rktio_process` on the
   current OS. */

RKTIO_EXTERN int rktio_process_pid(rktio_t *rktio, rktio_process_t *sp);
/* Always succeeds, whether or not the process is still running. */

RKTIO_EXTERN rktio_ok_t rktio_process_kill(rktio_t *rktio, rktio_process_t *sp);
RKTIO_EXTERN rktio_ok_t rktio_process_interrupt(rktio_t *rktio, rktio_process_t *sp);
/* Interrupts or kills a process; does not deallocate the process record. */

RKTIO_EXTERN void rktio_process_forget(rktio_t *rktio, rktio_process_t *sp);
/* Deallocates a process record, whether or not the process has
   stopped. */

RKTIO_EXTERN rktio_ok_t rktio_poll_process_done(rktio_t *rktio, rktio_process_t *sp);
/* Check whether a process has completed: */
#define RKTIO_PROCESS_ERROR    (-2)
#define RKTIO_PROCESS_DONE     1
#define RKTIO_PROCESS_RUNNING  0

typedef struct rktio_status_t {
  rktio_bool_t running;
  int result;
} rktio_status_t;

RKTIO_EXTERN rktio_status_t *rktio_process_status(rktio_t *rktio, rktio_process_t *sp);
/* The `result` value is only value if `running` is 0. */

RKTIO_EXTERN void rktio_block_child_signals(rktio_t*rktio, int block);


/*************************************************/
/* Filesystem-change events                      */

RKTIO_EXTERN int rktio_fs_change_properties(rktio_t *rktio);
/* Reports properties of the filesystem-change event implementation: */
#define RKTIO_FS_CHANGE_SUPPORTED   (1 << 0)
#define RKTIO_FS_CHANGE_SCALABLE    (1 << 1)
#define RKTIO_FS_CHANGE_LOW_LATENCY (1 << 2)
#define RKTIO_FS_CHANGE_FILE_LEVEL  (1 << 3)

typedef struct rktio_fs_change_t rktio_fs_change_t;

RKTIO_EXTERN rktio_fs_change_t *rktio_fs_change(rktio_t *rktio, const char *path);

RKTIO_EXTERN void rktio_fs_change_forget(rktio_t *rktio, rktio_fs_change_t *fc);

RKTIO_EXTERN rktio_tri_t rktio_poll_fs_change_ready(rktio_t *rktio, rktio_fs_change_t *fc);
/* Returns one of `RKTIO_POLL_READY`, etc. */

/*************************************************/
/* File-descriptor sets for polling              */

/* A poll set works for a single use via rktio_sleep(), as opposed to
   "long-term" poll sets that can be used multiple times. The
   `rktio_sleep` function accepts one of each and combines them. */

typedef struct rktio_poll_set_t rktio_poll_set_t;

RKTIO_EXTERN rktio_poll_set_t *rktio_make_poll_set(rktio_t *rktio);
RKTIO_EXTERN void rktio_poll_set_forget(rktio_t *rktio, rktio_poll_set_t *fds);
/* Don't reuse a poll set after calling `rktio_sleep`, but do
   explicitly forget it afterward. */

RKTIO_EXTERN void rktio_poll_add(rktio_t *rktio, rktio_fd_t *rfd, rktio_poll_set_t *fds, int modes);
/* Registers a wait on a file descriptor in read and/or write mode or
   flush mode. The flush mode corresponds to
   `rktio_poll_write_flushed`.
   `modes` values: */
#define RKTIO_POLL_READ   RKTIO_OPEN_READ
#define RKTIO_POLL_WRITE  RKTIO_OPEN_WRITE
#define RKTIO_POLL_FLUSH  (RKTIO_OPEN_WRITE << 2)

RKTIO_EXTERN void rktio_poll_add_accept(rktio_t *rktio, rktio_listener_t *listener, rktio_poll_set_t *fds);
RKTIO_EXTERN void rktio_poll_add_connect(rktio_t *rktio, rktio_connect_t *conn, rktio_poll_set_t *fds);
RKTIO_EXTERN void rktio_poll_add_addrinfo_lookup(rktio_t *rktio, rktio_addrinfo_lookup_t *lookup, rktio_poll_set_t *fds);
RKTIO_EXTERN void rktio_poll_add_process(rktio_t *rktio, rktio_process_t *sp, rktio_poll_set_t *fds);
RKTIO_EXTERN void rktio_poll_add_fs_change(rktio_t *rktio, rktio_fs_change_t *fc, rktio_poll_set_t *fds);
/* Registers various other waits. */

RKTIO_EXTERN void rktio_poll_set_add_nosleep(rktio_t *rktio, rktio_poll_set_t *fds);
/* Causes a sleep given `fds` to return immediately. */

#ifdef RKTIO_SYSTEM_WINDOWS
RKTIO_EXTERN void rktio_poll_set_add_handle(rktio_t *rktio, intptr_t h, rktio_poll_set_t *fds, int repost);
RKTIO_EXTERN void rktio_poll_set_add_eventmask(rktio_t *rktio, rktio_poll_set_t *fds, int mask);
/* When sleeping on Windows, extra handles or eventmasks can be added
   to trigger a wake up */
#endif

/*************************************************/
/* Long-term poll sets                           */

/* "Long-term" means that the poll set will be used frequently with
   incremental updates, which means that it's worthwhile to use an OS
   facililty (epoll, kqueue, etc.) to speed up polling. */

typedef struct rktio_ltps_t rktio_ltps_t;
typedef struct rktio_ltps_handle_t rktio_ltps_handle_t;

RKTIO_EXTERN rktio_ltps_t *rktio_ltps_open(rktio_t *rktio);

RKTIO_EXTERN int rktio_ltps_close(rktio_t *rktio, rktio_ltps_t *lt);
/* Closing will signal all remianing handles and free all signaled
   handles, but use `rktio_ltps_remove_all` and
   `rktio_ltps_get_signaled_handle` is you need to clean up any
   per-handle data: */

RKTIO_EXTERN rktio_ltps_handle_t *rktio_ltps_add(rktio_t *rktio, rktio_ltps_t *lt, rktio_fd_t *rfd, int mode);
/* Don't free the returned handle; use it with `rktio_ltps_handle_set_data`
   and `rktio_ltps_handle_get_data`, and free it only when the same handle
   is returned by `rktio_ltps_get_signaled_handle`. Using the `RKTIO_LTPS_REMOVE`
   mode causes a previous created handle to be signaled. A successful remove
   reports `RKTIO_ERROR_LTPS_REMOVED` while returning NULL. A `...CHECK...`
   or `...REMOVE...` mode that doesn't find the handle reports
   `RKTIO_ERROR_LTPS_NOT_FOUND`.
   `mode` values: */
enum {
  RKTIO_LTPS_CREATE_READ = 1,
  RKTIO_LTPS_CREATE_WRITE,
  RKTIO_LTPS_CHECK_READ,
  RKTIO_LTPS_CHECK_WRITE,
  RKTIO_LTPS_REMOVE,
  /* Internal, for filesystem-change events with kqueue: */
  RKTIO_LTPS_CREATE_VNODE,
  RKTIO_LTPS_CHECK_VNODE,
  RKTIO_LTPS_REMOVE_VNODE
};

RKTIO_EXTERN void rktio_ltps_handle_set_data(rktio_t *rktio, rktio_ltps_handle_t *s, void *data);
RKTIO_EXTERN void *rktio_ltps_handle_get_data(rktio_t *rktio, rktio_ltps_handle_t *s);

void rktio_ltps_remove_all(rktio_t *rktio, rktio_ltps_t *lt);
/* Removes all additions, signaling all handles. */

RKTIO_EXTERN rktio_ok_t rktio_ltps_poll(rktio_t *rktio, rktio_ltps_t *lt);
/* Enqueues signaled handles for retreival via `rktio_ltps_get_signaled_handle`.  */

RKTIO_EXTERN rktio_ltps_handle_t *rktio_ltps_get_signaled_handle(rktio_t *rktio, rktio_ltps_t *lt);
/* Free the returned handle when you're done with it. */

RKTIO_EXTERN void rktio_sleep(rktio_t *rktio, float nsecs, rktio_poll_set_t *fds, rktio_ltps_t *lt);
/* Waits up to `nsecs` seconds (or forever if `nsecs` is 0) or until
   something registered with `fds` or `lt` is ready. */

/*************************************************/
/* Files, directories, and links                 */

RKTIO_EXTERN int rktio_file_exists(rktio_t *rktio, const char *filename);
RKTIO_EXTERN int rktio_directory_exists(rktio_t *rktio, const char *dirname);
RKTIO_EXTERN int rktio_link_exists(rktio_t *rktio, const char *filename);
RKTIO_EXTERN int rktio_is_regular_file(rktio_t *rktio, const char *filename);

RKTIO_EXTERN rktio_ok_t rktio_delete_file(rktio_t *rktio, const char *fn, int enable_write_on_fail);

RKTIO_EXTERN rktio_ok_t rktio_rename_file(rktio_t *rktio, const char *dest, const char *src, int exists_ok);
/* Can report `RKTIO_ERROR_EXISTS`. */

RKTIO_EXTERN char *rktio_get_current_directory(rktio_t *rktio);
RKTIO_EXTERN rktio_ok_t rktio_set_current_directory(rktio_t *rktio, const char *path);

RKTIO_EXTERN rktio_ok_t rktio_make_directory(rktio_t *rktio, const char *filename);
/* Can report `RKTIO_ERROR_EXISTS`. */

RKTIO_EXTERN rktio_ok_t rktio_delete_directory(rktio_t *rktio, const char *filename, const char *current_directory,
                                               int enable_write_on_fail);
/* The `current_directory` argument is used on Windows to avoid being
   in `filename` (instead) as a directory while trying to delete it.
   The `enable_write_on_fail` argument also applied to Windows. */

RKTIO_EXTERN char *rktio_readlink(rktio_t *rktio, const char *fullfilename);
/* Argument should not have a trailing separator. Can report
   `RKTIO_ERROR_NOT_A_LINK`. */

RKTIO_EXTERN rktio_ok_t rktio_make_link(rktio_t *rktio, const char *src, const char *dest,
                                        int dest_is_directory);
/* The `dest_is_directory` argument is used only
   on Windows. Can report `RKTIO_ERROR_EXISTS`. */

/*************************************************/
/* File attributes                               */

typedef intptr_t rktio_timestamp_t;

RKTIO_EXTERN rktio_filesize_t *rktio_file_size(rktio_t *rktio, const char *filename);

RKTIO_EXTERN rktio_timestamp_t *rktio_get_file_modify_seconds(rktio_t *rktio, const char *file);
RKTIO_EXTERN rktio_ok_t rktio_set_file_modify_seconds(rktio_t *rktio, const char *file, rktio_timestamp_t secs);

typedef struct {
  uintptr_t a, b, c;
  int a_bits, b_bits, c_bits; /* size of each in bits */
} rktio_identity_t;

RKTIO_EXTERN rktio_identity_t *rktio_fd_identity(rktio_t *rktio, rktio_fd_t *fd);
RKTIO_EXTERN rktio_identity_t *rktio_path_identity(rktio_t *rktio, const char *path, int follow_links);

/*************************************************/
/* Permissions                                   */

/* Should match OS bits: */
#define RKTIO_PERMISSION_READ  0x4
#define RKTIO_PERMISSION_WRITE 0x2
#define RKTIO_PERMISSION_EXEC  0x1

#define RKTIO_PERMISSION_ERROR (-1)

RKTIO_EXTERN int rktio_get_file_or_directory_permissions(rktio_t *rktio, const char *filename, int all_bits);
/* Result is `RKTIO_PERMISSION_ERROR` for error, otherwise a combination of
   bits. If not `all_bits`, then use constants above. */

RKTIO_EXTERN rktio_ok_t rktio_set_file_or_directory_permissions(rktio_t *rktio, const char *filename, int new_bits);
/* The `new_bits` format corresponds to `all_bits` for getting permissions.
   Can report `RKTIO_ERROR_BAD_PERMISSION` for bits that make no sense. */

/*************************************************/
/* Directory listing                             */

typedef struct rktio_directory_list_t rktio_directory_list_t;

RKTIO_EXTERN rktio_directory_list_t *rktio_directory_list_start(rktio_t *rktio, const char *dirname);
/* On Windows, the given `dirname` must be normalized and not have
   `.` or `..`: */

RKTIO_EXTERN char *rktio_directory_list_step(rktio_t *rktio, rktio_directory_list_t *dl);
/* Returns an unallocated "" and deallocates `dl` when the iteration
   is complete. A NULL result would mean an error without deallocating
   `dl`, but that doesn't currently happen. */

RKTIO_EXTERN void rktio_directory_list_stop(rktio_t *rktio, rktio_directory_list_t *dl);
/* Interrupt a directory list in progress, not needed after
   rktio_directory_list_step() returns "": */

RKTIO_EXTERN char **rktio_filesystem_root_list(rktio_t *rktio);
/* Returns a NULL-terminated array. Free each string. Currently never
   errors. */

/*************************************************/
/* File copying                                  */

typedef struct rktio_file_copy_t rktio_file_copy_t;

RKTIO_EXTERN rktio_file_copy_t *rktio_copy_file_start(rktio_t *rktio, const char *dest, const char *src,
                                                      rktio_bool_t exists_ok);
/* Can report `RKTIO_ERROR_EXISTS`. */

RKTIO_EXTERN int rktio_copy_file_is_done(rktio_t *rktio, rktio_file_copy_t *fc);
RKTIO_EXTERN rktio_ok_t rktio_copy_file_step(rktio_t *rktio, rktio_file_copy_t *fc);
RKTIO_EXTERN void rktio_copy_file_stop(rktio_t *rktio, rktio_file_copy_t *fc);

/*************************************************/
/* System paths                                  */

RKTIO_EXTERN char *rktio_system_path(rktio_t *rktio, int which);
/* `which` values: */
enum {
  RKTIO_PATH_SYS_DIR,
  RKTIO_PATH_TEMP_DIR,
  RKTIO_PATH_PREF_DIR,
  RKTIO_PATH_PREF_FILE,
  RKTIO_PATH_ADDON_DIR,
  RKTIO_PATH_HOME_DIR,
  RKTIO_PATH_DESK_DIR,
  RKTIO_PATH_DOC_DIR,
  RKTIO_PATH_INIT_DIR,
  RKTIO_PATH_INIT_FILE
};

RKTIO_EXTERN char *rktio_expand_user_tilde(rktio_t *rktio, const char *filename);
/* Path must start with tilde, otherwise `RKTIO_ERROR_NO_TILDE`.
   Other possible errors are `RKTIO_ERROR_ILL_FORMED_USER` and
   `RKTIO_ERROR_UNKNOWN_USER`. */

/*************************************************/
/* Sleep and signals                             */

typedef struct rktio_signal_handle_t rktio_signal_handle_t;

RKTIO_EXTERN rktio_signal_handle_t *rktio_get_signal_handle(rktio_t *rktio);
RKTIO_EXTERN void rktio_signal_received_at(rktio_signal_handle_t *h);
RKTIO_EXTERN void rktio_signal_received(rktio_t *rktio);

RKTIO_EXTERN void rktio_wait_until_signal_received(rktio_t *rktio);
RKTIO_EXTERN void rktio_flush_signals_received(rktio_t *rktio);

#ifdef RKTIO_SYSTEM_UNIX
RKTIO_EXTERN int rktio_signal_handle_to_fd();
#endif

/*************************************************/
/* Time and date                                 */

typedef struct rktio_date_t {
  int nanosecond, second, minute, hour, day, month;
  intptr_t year;
  int day_of_week;
  int day_of_year;
  int is_dst;
  int zone_offset;
  char *zone_name; /* can be NULL; otherwise, free it */
} rktio_date_t;

RKTIO_EXTERN intptr_t rktio_get_milliseconds(void);
RKTIO_EXTERN double rktio_get_inexact_milliseconds(void);

RKTIO_EXTERN intptr_t rktio_get_process_milliseconds(rktio_t *rktio);
RKTIO_EXTERN intptr_t rktio_get_process_children_milliseconds(rktio_t *rktio);

RKTIO_EXTERN rktio_timestamp_t rktio_get_seconds(rktio_t *rktio);
RKTIO_EXTERN rktio_date_t *rktio_seconds_to_date(rktio_t *rktio, rktio_timestamp_t seconds, int nanoseconds, int get_gmt);

/*************************************************/
/* Errors                                        */

RKTIO_EXTERN int rktio_get_last_error_kind(rktio_t *rktio);

/* Kinds of error values: */
enum {
  RKTIO_ERROR_KIND_POSIX,
  RKTIO_ERROR_KIND_WINDOWS,
  RKTIO_ERROR_KIND_GAI,
  RKTIO_ERROR_KIND_RACKET
};

RKTIO_EXTERN int rktio_get_last_error(rktio_t *rktio);

/* Error IDs of kind RKTIO_ERROR_KIND_RACKET */
enum {
  RKTIO_ERROR_UNSUPPORTED = 1,
  RKTIO_ERROR_DOES_NOT_EXIST,
  RKTIO_ERROR_EXISTS,
  RKTIO_ERROR_LINK_FAILED,
  RKTIO_ERROR_NOT_A_LINK,
  RKTIO_ERROR_BAD_PERMISSION,
  RKTIO_ERROR_IS_A_DIRECTORY,
  RKTIO_ERROR_NOT_A_DIRECTORY,
  RKTIO_ERROR_UNSUPPORTED_TEXT_MODE,
  RKTIO_ERROR_CANNOT_FILE_POSITION,
  RKTIO_ERROR_NO_TILDE,
  RKTIO_ERROR_ILL_FORMED_USER,
  RKTIO_ERROR_UNKNOWN_USER,
  RKTIO_ERROR_INIT_FAILED,
  RKTIO_ERROR_LTPS_NOT_FOUND,
  RKTIO_ERROR_LTPS_REMOVED, /* indicates success, instead of failure */
  RKTIO_ERROR_CONNECT_TRYING_NEXT, /* indicates that failure is not (yet) premanent */
  RKTIO_ERROR_ACCEPT_NOT_READY,
  RKTIO_ERROR_HOST_AND_PORT_BOTH_UNSPECIFIED,
  RKTIO_ERROR_INFO_TRY_AGAIN, /* for UDP */
  RKTIO_ERROR_TRY_AGAIN, /* for UDP */
  RKTIO_ERROR_TRY_AGAIN_WITH_IPV4, /* for TCP listen */
  RKTIO_ERROR_TIME_OUT_OF_RANGE,
  RKTIO_ERROR_NO_SUCH_ENVVAR,
};

RKTIO_EXTERN const char *rktio_get_last_error_string(rktio_t *rktio);
RKTIO_EXTERN const char *rktio_get_error_string(rktio_t *rktio, int kind, int errid);
/* The returned strings for `rktio_...error_string()` should not be
   deallocated, but it only lasts reliably until the next call to
  either of the functions. */

/*************************************************/

#endif
