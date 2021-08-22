/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#define HAVE_ALLOCA_H 1

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if your struct stat has st_blksize.  */
#define HAVE_ST_BLKSIZE 1

/* Define if your struct stat has st_blocks.  */
#define HAVE_ST_BLOCKS 1

/* Define if your struct stat has st_rdev.  */
#define HAVE_ST_RDEV 1

/* Define if your struct tm has tm_zone.  */
#define HAVE_TM_ZONE 1

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
/* #undef HAVE_TZNAME */

/* Define if utime(file, NULL) sets file's timestamp to the present.  */
#define HAVE_UTIME_NULL 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define if your C compiler doesn't accept -c and -o together.  */
/* #undef NO_MINUS_C_MINUS_O */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* #undef HAVE_RECODE */
/* #undef USE_BCOPY */
/* #undef HAVE_FTP */

/* #undef ushort */

/* Define if you want dmalloc checking */
#define HAVE_DMALLOC 0

/* Define if you want to have PCRE support */
#define HAVE_PCRE 1

/* Define if you have the libmhash library */
/* #undef HAVE_LIBMHASH */

/* Define if you have the libmcrypt library */
/* #undef HAVE_LIBMCRYPT */

/* This is the default configuration file to read */
#define CONFIGURATION_FILE_PATH "/usr/local/lib"
#define USE_CONFIG_FILE 1

/* #undef HAVE_CPDFLIB */
/* #undef ptrdiff_t */

#define NDBM_DB1_NDBM_H 0
#define NDBM_NDBM_H 0
#define DB2_DB2_DB_H 0
#define DB2_DB_DB2_H 0
#define DB2_DB_H 0
#define DB2_DB2_H 0
#define HAVE_DBA 0
#define DBA_GDBM 0
#define DBA_NDBM 0
#define DBA_DBOPEN 0
#define DBA_DB2 0
#define DBA_DBM 0
#define DBA_CDB 0

#define HAVE_GDBM_H 0

/* Some global constants defined by conigure */
#define PHP_BUILD_DATE "2019-12-10"
#define PHP_OS "Linux"
#define PHP_UNAME "Linux seclab4 4.9.0-9-amd64 #1 SMP Debian 4.9.168-1+deb9u4 (2019-07-19) x86_64 GNU/Linux" 

/* define uint by configure if it is missed (QNX and BSD derived) */
/* #undef uint */

/* define ulong by configure if it is missed (most probably is) */
/* #undef ulong */

/* type check for in_addr_t */
/* #undef in_addr_t */

/* crypt capability checks */
#define PHP3_STD_DES_CRYPT 1
#define PHP3_EXT_DES_CRYPT 0
#define PHP3_MD5_CRYPT 1
#define PHP3_BLOWFISH_CRYPT 0

/* Solaris YP check */
/* #undef SOLARIS_YP */

/* Netscape LDAP SDK check */
/* #undef HAVE_NSLDAP */

/* ImageMagick check */
/* #undef HAVE_LIBMAGICK */

/* Define if your struct tm has tm_gmtoff.  */
#define HAVE_TM_GMTOFF 1

/* Define if you have struct flock */
#define HAVE_STRUCT_FLOCK 1

/* Define if you have the resolv library (-lresolv). */
#define HAVE_LIBRESOLV 1

/* Define if you have the gd library (-lgd). */
#define HAVE_LIBGD 0

/* #undef HAVE_GD_PNG */ 
/* #undef HAVE_GD_GIF */
/* #undef HAVE_GD_JPG */
/* #undef HAVE_GD_LZW */
/* #undef HAVE_GD_COLORRESOLVE */
/* #undef HAVE_GD_ANCIENT */

/* Define if you have the GNU gettext library (-lintl). */
#define HAVE_LIBINTL 0

/* Define if you have the zlib library */
#define HAVE_ZLIB 0

/* Define if you want safe mode enabled by default. */
#define PHP_SAFE_MODE 0

/* Set to the path to the dir containing safe mode executables */
#define PHP_SAFE_MODE_EXEC_DIR "/usr/local/php/bin"

/* Define if you want POST/GET/Cookie track variables by default */
#define PHP_TRACK_VARS 0

/* Define if PHP to setup it's own SIGCHLD handler */
#define PHP_SIGCHILD 0

/* Define if you want POST/GET/Cookie track variables by default */
#define PHP_TRACK_VARS 0

/* Undefine if you want stricter XML/SGML compliance by default */
/* (this disables "<?expression?>" by default) */
#define DEFAULT_SHORT_OPEN_TAG 1

/* Undefine if you do not want PHP by default to escape "'" */
/* in GET/POST/Cookie data */
#define MAGIC_QUOTES 0

/* Define if you an ndbm compatible library (-ldbm).  */
#define NDBM 0

/* Define if you have the gdbm library (-lgdbm).  */
#define GDBM 0

/* Define if you want the Perl-regex library */
#define HAVE_BUNDLED_PCRE 0
#define HAVE_PCRE 1

/* Define both of these if you want the bundled REGEX library */
#define REGEX 1
#define HSREGEX 1

/* Define if you want Solid database support */
#define HAVE_SOLID 0

/* Define if you want IBM DB2 database support */
#define HAVE_IBMDB2 0

/* Define if you want to use the supplied dbase library */
#define DBASE 0

/* Define if you want Hyperwave support */
#define HYPERWAVE 0

/* Define if you have the Oracle database client libraries */
#define HAVE_ORACLE 0

/* Define if you have the Oracle version 8 database client libraries */
#define HAVE_OCI8 0

/* Define if you want to use the iODBC database driver */
#define HAVE_IODBC 0

/* Define if you want to use the OpenLink ODBC database driver */
#define HAVE_OPENLINK 0

/* Define if you have the AdabasD client libraries */
#define HAVE_ADABAS 0

/* Define if you want the LDAP directory interface */
#define HAVE_LDAP 0

/* Define if you want the Cybercash MCK support */
#define HAVE_MCK 0

/* Define if you want the IMAP directory interface */
#define HAVE_IMAP 0

/* Define if you want MCAL */
#define HAVE_MCAL 0 

/* Define if you want the IMSP options/addressbook interface */
#define HAVE_IMSP 0

/* Define if you want the ASPELL interface */
#define HAVE_ASPELL 0

/* Define if you want to use a custom ODBC database driver */
#define HAVE_CODBC 0

/* Define to use the unified ODBC interface */
#define HAVE_UODBC 0

/* Define if you have libdl (used for dynamic linking) */
#define HAVE_LIBDL 1

/* Define if you have libdnet_stub (used for Sybase support) */
#define HAVE_LIBDNET_STUB 0

/* Define if you have and want to use libcrypt */
#define HAVE_LIBCRYPT 1

/* Define if you have and want to use libnsl */
#define HAVE_LIBNSL 0

/* Define if you have and want to use libsocket */
#define HAVE_LIBSOCKET 0

/* Define if you have and want to use libpam */
#define HAVE_LIBPAM 1

/* Define if you have the sendmail program available */
#define HAVE_SENDMAIL 1

/* Define if you are compiling PHP as an Apache module */
#define APACHE 0

/* Define if you are compiling PHP as an Apache module with mod_charset patch applied (aka Russian Apache)*/
#define USE_TRANSFER_TABLES 0

/* Define if you are compiling PHP as an fhttpd module */
#define FHTTPD 0

/* Define if your Apache creates an ap_config.h header file (only 1.3b6 and later) */
#define HAVE_AP_CONFIG_H 0

/* Define if your Apache has src/include/compat.h */
#define HAVE_OLD_COMPAT_H 0

/* Define if your Apache has src/include/ap_compat.h */
#define HAVE_AP_COMPAT_H 0

#ifndef HAVE_EMPRESS
#define HAVE_EMPRESS 0
#endif

#define HAVE_SYBASE 0
#define HAVE_SYBASE_CT 0

#ifndef HAVE_MYSQL
#define HAVE_MYSQL 0
#endif

#ifndef HAVE_MSQL
#define HAVE_MSQL 0
#endif

#ifndef HAVE_PGSQL
#define HAVE_PGSQL 0
#endif

#ifndef HAVE_VELOCIS
#define HAVE_VELOCIS 0
#endif

#ifndef HAVE_IFX
#define HAVE_IFX 0
#endif
#ifndef HAVE_IFX_IUS
#define HAVE_IFX_IUS 0
#endif
#ifndef IFX_VERSION
#define IFX_VERSION 0
#endif

#ifndef HAVE_IBASE
#define HAVE_IBASE 0
#endif

#ifndef HAVE_PQCMDTUPLES
#define HAVE_PQCMDTUPLES 0
#endif

#define MSQL1 0
#define HAVE_FILEPRO 0
#define HAVE_SOLID 0
#define DEBUG 0

/* Define if you want to enable the PHP TCP/IP debugger (experimental) */
#define PHP_DEBUGGER 0

/* Define if you want to enable bc style precision math support */
#define WITH_BCMATH 1

/* Define if you want to prevent the CGI from working unless REDIRECT_STATUS is defined in the environment */
#define FORCE_CGI_REDIRECT 0

/* Define if you want to prevent the CGI from using path_info and path_translated */
#define DISCARD_PATH 0

/* Define if you want to enable memory limit support */
#define MEMORY_LIMIT 0

/* Define if you want include() and other functions to be able to open
 * http and ftp URLs as files.
 */
#define PHP3_URL_FOPEN 1

/* Define if you want System V semaphore support.
 */
#define HAVE_SYSVSEM 0

/* Define if you have union semun.
 */
#define HAVE_SEMUN 0

/* Define if you want System V shared memory support.  */
#define HAVE_SYSVSHM 0

/* Define if you want to enable displaying source support. */
#define HAVE_DISPLAY_SOURCE 1

/* Define if you have broken header files like SunOS 4 */
#define MISSING_FCLOSE_DECL 0

/* Define if you have broken sprintf function like SunOS 4 */
#define BROKEN_SPRINTF 0

/* Define if you have the expat (XML Parser Toolkit) library */
#define HAVE_LIBEXPAT 0

/* Define if your xml include files are not in an dir named xml */
#define RAW_XML_INCLUDEPATH 0

/* Define if you have the pdflib library */
#define HAVE_PDFLIB 0

/* Define if you have the pdflib library */
#define HAVE_PDFLIB2 0

/* Define if you have the fdftk library */
#define HAVE_FDFLIB 0

/* Define to compile with mod_dav support */
#define HAVE_MOD_DAV 0

/* Define to enable yp/nis support */
#define HAVE_YP 0

/* Define if you have the t1 library (-lt1).  */
#define HAVE_LIBT1 0
/* #undef HAVE_LIBT1_OUTLINE */

#define HAVE_LIBTTF 0
#define HAVE_LIBFREETYPE 0

#define HAVE_POSIX 1

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a long.  */
#define SIZEOF_LONG 8

/* Define if you have the crypt function.  */
#define HAVE_CRYPT 1

/* Define if you have the ctermid function.  */
#define HAVE_CTERMID 1

/* Define if you have the cuserid function.  */
#define HAVE_CUSERID 1

/* Define if you have the flock function.  */
#define HAVE_FLOCK 1

/* Define if you have the gcvt function.  */
#define HAVE_GCVT 1

/* Define if you have the getlogin function.  */
#define HAVE_GETLOGIN 1

/* Define if you have the getopt function.  */
#define HAVE_GETOPT 1

/* Define if you have the getpgid function.  */
#define HAVE_GETPGID 1

/* Define if you have the getrlimit function.  */
#define HAVE_GETRLIMIT 1

/* Define if you have the getrusage function.  */
#define HAVE_GETRUSAGE 1

/* Define if you have the getsid function.  */
#define HAVE_GETSID 1

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the inet_aton function.  */
#define HAVE_INET_ATON 1

/* Define if you have the link function.  */
#define HAVE_LINK 1

/* Define if you have the lockf function.  */
#define HAVE_LOCKF 1

/* Define if you have the lrand48 function.  */
#define HAVE_LRAND48 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the mkfifo function.  */
#define HAVE_MKFIFO 1

/* Define if you have the putenv function.  */
#define HAVE_PUTENV 1

/* Define if you have the random function.  */
#define HAVE_RANDOM 1

/* Define if you have the regcomp function.  */
#define HAVE_REGCOMP 1

/* Define if you have the rint function.  */
#define HAVE_RINT 1

/* Define if you have the setitimer function.  */
#define HAVE_SETITIMER 1

/* Define if you have the setlocale function.  */
#define HAVE_SETLOCALE 1

/* Define if you have the setsid function.  */
#define HAVE_SETSID 1

/* Define if you have the setsockopt function.  */
#define HAVE_SETSOCKOPT 1

/* Define if you have the setvbuf function.  */
#define HAVE_SETVBUF 1

/* Define if you have the shutdown function.  */
#define HAVE_SHUTDOWN 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Define if you have the srand48 function.  */
#define HAVE_SRAND48 1

/* Define if you have the srandom function.  */
#define HAVE_SRANDOM 1

/* Define if you have the statfs function.  */
#define HAVE_STATFS 1

/* Define if you have the statvfs function.  */
#define HAVE_STATVFS 1

/* Define if you have the strcasecmp function.  */
#define HAVE_STRCASECMP 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the symlink function.  */
#define HAVE_SYMLINK 1

/* Define if you have the tempnam function.  */
#define HAVE_TEMPNAM 1

/* Define if you have the truncate function.  */
#define HAVE_TRUNCATE 1

/* Define if you have the tzset function.  */
#define HAVE_TZSET 1

/* Define if you have the unsetenv function.  */
#define HAVE_UNSETENV 1

/* Define if you have the usleep function.  */
#define HAVE_USLEEP 1

/* Define if you have the utime function.  */
#define HAVE_UTIME 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the <crypt.h> header file.  */
#define HAVE_CRYPT_H 1

/* Define if you have the <db.h> header file.  */
/* #undef HAVE_DB_H */

/* Define if you have the <db1/ndbm.h> header file.  */
/* #undef HAVE_DB1_NDBM_H */

/* Define if you have the <dbm.h> header file.  */
/* #undef HAVE_DBM_H */

/* Define if you have the <default_store.h> header file.  */
/* #undef HAVE_DEFAULT_STORE_H */

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <dlfcn.h> header file.  */
#define HAVE_DLFCN_H 1

/* Define if you have the <errmsg.h> header file.  */
/* #undef HAVE_ERRMSG_H */

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <features.h> header file.  */
#define HAVE_FEATURES_H 1

/* Define if you have the <grp.h> header file.  */
#define HAVE_GRP_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file.  */
#define HAVE_LOCALE_H 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <mysql.h> header file.  */
/* #undef HAVE_MYSQL_H */

/* Define if you have the <ndbm.h> header file.  */
/* #undef HAVE_NDBM_H */

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <netinet/in.h> header file.  */
#define HAVE_NETINET_IN_H 1

/* Define if you have the <pwd.h> header file.  */
#define HAVE_PWD_H 1

/* Define if you have the <signal.h> header file.  */
#define HAVE_SIGNAL_H 1

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/resource.h> header file.  */
#define HAVE_SYS_RESOURCE_H 1

/* Define if you have the <sys/socket.h> header file.  */
#define HAVE_SYS_SOCKET_H 1

/* Define if you have the <sys/statfs.h> header file.  */
#define HAVE_SYS_STATFS_H 1

/* Define if you have the <sys/statvfs.h> header file.  */
#define HAVE_SYS_STATVFS_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <sys/varargs.h> header file.  */
/* #undef HAVE_SYS_VARARGS_H */

/* Define if you have the <sys/wait.h> header file.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the <syslog.h> header file.  */
#define HAVE_SYSLOG_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <unix.h> header file.  */
/* #undef HAVE_UNIX_H */

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Define if you have the resolv library (-lresolv).  */
#define HAVE_LIBRESOLV 1

/* Whether system headers declare timezone */
#define HAVE_DECLARED_TIMEZONE 1

/*   */
/* #undef HAVE_SNMP */

/*   */
/* #undef UCD_SNMP_HACK */

