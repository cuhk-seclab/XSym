#!/bin/sh
make 
echo "Normal to have ERROR in last step of make"
find . -name "*\.o" | while read line; do 
    extract-bc  $line
    echo "extract-bc $line"
done

llvm-link  \
 request_info.o.bc php3_hash.o.bc   safe_mode.o.bc \
 fopen-wrappers.o.bc  stack.o.bc operators.o.bc token_cache.o.bc \
 variables.o.bc constants.o.bc internal_functions.o.bc  \
 language-parser.tab.o.bc \
 language-scanner.o.bc \
 configuration-parser.tab.o.bc \
 list.o.bc \
 alloc.o.bc \
 pcrelib/pcre.o.bc \
 pcrelib/study.o.bc \
functions/adabasd.o.bc \
functions/apache.o.bc \
functions/aspell.o.bc \
functions/base64.o.bc \
functions/basic_functions.o.bc \
functions/bcmath.o.bc \
functions/browscap.o.bc \
functions/COM.o.bc \
functions/cpdf.o.bc \
functions/crypt.o.bc \
functions/cyr_convert.o.bc \
functions/datetime.o.bc \
functions/dav.o.bc \
functions/dba_cdb.o.bc \
functions/dba_db2.o.bc \
functions/dba_dbm.o.bc \
functions/dba_gdbm.o.bc \
functions/dba_ndbm.o.bc \
functions/dba.o.bc \
functions/dbase.o.bc \
functions/db.o.bc \
functions/dir.o.bc \
functions/dlist.o.bc \
functions/dl.o.bc \
functions/dns.o.bc \
functions/exec.o.bc \
functions/fdf.o.bc \
functions/fhttpd.o.bc \
functions/file.o.bc \
functions/filepro.o.bc \
functions/filestat.o.bc \
functions/formatted_print.o.bc \
functions/fsock.o.bc \
functions/ftp.o.bc \
functions/gdcache.o.bc \
functions/gd.o.bc \
functions/gdttf.o.bc \
functions/gettext.o.bc \
functions/head.o.bc \
functions/hg_comm.o.bc \
functions/html.o.bc \
functions/hw.o.bc \
functions/ifx.o.bc \
functions/image.o.bc \
functions/imap.o.bc \
functions/imsp.o.bc \
functions/info.o.bc \
functions/interbase.o.bc \
functions/iptc.o.bc \
functions/lcg.o.bc \
functions/ldap.o.bc \
functions/levenshtein.o.bc \
functions/link.o.bc \
functions/magick.o.bc \
functions/mail.o.bc \
functions/math.o.bc \
functions/mcrypt.o.bc \
functions/md5.o.bc \
functions/mhash.o.bc \
functions/microtime.o.bc \
functions/mime.o.bc \
functions/msql.o.bc \
functions/mysql.o.bc \
functions/number.o.bc \
functions/oci8.o.bc \
functions/oracle.o.bc \
functions/pack.o.bc \
functions/pageinfo.o.bc \
functions/parsedate.o.bc \
functions/pcre.o.bc \
functions/pdf.o.bc \
functions/pgsql.o.bc \
functions/php3_mcal.o.bc \
functions/php3_mckcrypt.o.bc \
functions/php_ftp.o.bc \
functions/posix.o.bc \
functions/post.o.bc \
functions/quot_print.o.bc \
functions/rand.o.bc \
functions/recode.o.bc \
functions/reg.o.bc \
functions/snmp.o.bc \
functions/solid.o.bc \
functions/soundex.o.bc \
functions/string.o.bc \
functions/sybase-ct.o.bc \
functions/sybase.o.bc \
functions/syslog.o.bc \
functions/sysvsem.o.bc \
functions/sysvshm.o.bc \
functions/type.o.bc \
functions/unified_odbc.o.bc \
functions/uniqid.o.bc \
functions/url.o.bc \
functions/var.o.bc \
functions/velocis.o.bc \
functions/wddx_a.o.bc \
functions/wddx.o.bc \
functions/xml.o.bc \
functions/yp.o.bc \
functions/zlib.o.bc \
main.o.bc -o test.bc
echo "====== test.bc generated"
sh runklee.sh "test.bc"

