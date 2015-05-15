#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mysql_cn_parser], [1.0], [https://github.com/guitarpoet/mysql_cn_parser])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/mysql_cn_parser.cc])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AC_ENABLE_SHARED
LT_INIT

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Check for mysql plugin folders
MYSQL_PLUGIN_FOLDERS="/usr/local/lib/mysql /usr/lib/mysql /usr/lib64/mysql /opt/local/lib/mysql /opt/local/lib/mysql55 /opt/local/lib/mysql56"

AC_MSG_CHECKING(mysql plugin folder)

if test -z "$MYSQL_PLUGIN_DIR"; then
	for i in $MYSQL_PLUGIN_FOLDERS; do
		if test -d $i/plugin; then
			MYSQL_PLUGIN_DIR=$i/plugin
		fi
	done
fi

if test -z "$MYSQL_PLUGIN_DIR"; then
	AC_MSG_RESULT(no)
	AC_MSG_ERROR(No mysql plugin folder found, please configure variable MYSQL_PLUGIN_DIR!)
else
	AC_MSG_RESULT(yes)
	AC_SUBST([pkgplugindir], [$MYSQL_PLUGIN_DIR])
fi

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lmmseg':
AC_CHECK_LIB([mmseg], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

MYSQL_SEARCH_DIRS="/usr/local/include /usr/include /opt/local/include /opt/local/include/mysql55 /opt/local/include/mysql56"

AC_MSG_CHECKING(mysql headers)
for i in $MYSQL_SEARCH_DIRS; do
	if test -f $i/mysql/mysql.h; then
		MYSQL_INCDIR=$i/mysql
	fi
done

if test -z "$MYSQL_INCDIR"; then
	AC_MSG_RESULT(no)
	AC_MSG_ERROR(No mysql header found!)
else
	AC_MSG_RESULT(yes)
fi

# Set the include flags

CPPFLAGS="-I$MYSQL_INCDIR -I$OBABEL_INCDIR $CPPFLAGS"

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL #Disable this macro for autoconf version 2.63
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([strtol])

AC_OUTPUT(Makefile src/Makefile)
