// #pragma once

// #include <string>
// #include <map>

// class Errno {
// public:
//     static std::string str(int& num) {
//         static std::map<int, std::string> errnoMap = {
//             {EPERM, "EPERM"},               /* Operation not permitted */
//             {ENOENT, "ENOENT"},             /* No such file or directory */
//             {ESRCH, "ESRCH"},               /* No such process */
//             {EINTR, "EINTR"},               /* Interrupted system call */
//             {EIO, "EIO"},                   /* I/O error */
//             {ENXIO, "ENXIO"},               /* No such device or address */
//             {E2BIG, "E2BIG"},               /* Argument list too long */
//             {ENOEXEC, "ENOEXEC"},           /* Exec format error */
//             {EBADF, "EBADF"},               /* Bad file number */
//             {ECHILD, "ECHILD"},             /* No child processes */
//             {EAGAIN, "EAGAIN"},             /* Try again */
//             {ENOMEM, "ENOMEM"},             /* Out of memory */
//             {EACCES, "EACCES"},             /* Permission denied */
//             {EFAULT, "EFAULT"},             /* Bad address */
//             {ENOTBLK, "ENOTBLK"},           /* Block device required */
//             {EBUSY, "EBUSY"},               /* Device or resource busy */
//             {EEXIST, "EEXIST"},             /* File exists */
//             {EXDEV, "EXDEV"},               /* Cross-device link */
//             {ENODEV, "ENODEV"},             /* No such device */
//             {ENOTDIR, "ENOTDIR"},           /* Not a directory */
//             {EISDIR, "EISDIR"},             /* Is a directory */
//             {EINVAL, "EINVAL"},             /* Invalid argument */
//             {ENFILE, "ENFILE"},             /* File table overflow */
//             {EMFILE, "EMFILE"},             /* Too many open files */
//             {ENOTTY, "ENOTTY"},             /* Not a typewriter */
//             {ETXTBSY, "ETXTBSY"},           /* Text file busy */
//             {EFBIG, "EFBIG"},               /* File too large */
//             {ENOSPC, "ENOSPC"},             /* No space left on device */
//             {ESPIPE, "ESPIPE"},             /* Illegal seek */
//             {EROFS, "EROFS"},               /* Read-only file system */
//             {EMLINK, "EMLINK"},             /* Too many links */
//             {EPIPE, "EPIPE"},               /* Broken pipe */
//             {EDOM, "EDOM"},                 /* Math argument out of domain of func */
//             {ERANGE, "ERANGE"},             /* Math result not representable */
//             {EADDRINUSE, "EADDRINUSE"},     /* Address already in use */
//             {EAFNOSUPPORT, "EAFNOSUPPORT"}, /* Address family not supported by protocol */
//             {ECONNREFUSED, "ECONNREFUSED"}, /* Connection refused */
//             {ECONNRESET, "ECONNRESET"},     /* Connection reset by peer */
//             {EDESTADDRREQ, "EDESTADDRREQ"}, /* Destination address required */
//             {EHOSTUNREACH, "EHOSTUNREACH"}, /* No route to host */
//             {EINPROGRESS, "EINPROGRESS"},   /* Operation now in progress */
//             {EISCONN, "EISCONN"},           /* Transport endpoint is already connected */
//             {EMSGSIZE, "EMSGSIZE"},         /* Message too long */
//             {ENETDOWN, "ENETDOWN"},         /* Network is down */
//             {ENETUNREACH, "ENETUNREACH"},   /* Network is unreachable */
//             {ENOBUFS, "ENOBUFS"},           /* No buffer space available */
//             {ENOPROTOOPT, "ENOPROTOOPT"},   /* Protocol not available */
//             {ENOTCONN, "ENOTCONN"},         /* Transport endpoint is not connected */
//             {ENOTSOCK, "ENOTSOCK"},         /* Socket operation on non-socket */
//             {EOPNOTSUPP, "EOPNOTSUPP"},     /* Operation not supported on transport endpoint */
//             {EPROTONOSUPPORT, "EPROTONOSUPPORT"},   /* Protocol not supported */
//             {EPROTOTYPE, "EPROTOTYPE"},     /* Protocol wrong type for socket */
//         };
//         if(errnoMap.find(num) != errnoMap.end()) {
//             return errnoMap[num];
//         }
//         return "Unknown errno";
//     }
// };