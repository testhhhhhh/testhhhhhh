
/* On Unix-like systems config.in is converted by "configure" into config.h.
Some other environments also support the use of "configure". PCRE is written in
Standard C, but there are a few non-standard things it can cope with, allowing
it to run on SunOS4 and other "close to standard" systems.

On a non-Unix-like system you should just copy this file into config.h, and set
up the macros the way you need them. You should normally change the definitions
of HAVE_STRERROR and HAVE_MEMMOVE to 1. Unfortunately, because of the way
autoconf works, these cannot be made the defaults. If your system has bcopy()
and not memmove(), change the definition of HAVE_BCOPY instead of HAVE_MEMMOVE.
If your system has neither bcopy() nor memmove(), leave them both as 0; an
emulation function will be used. */

/* If you are compiling for a system that uses EBCDIC instead of ASCII
character codes, define this macro as 1. On systems that can use "configure",
this can be done via --enable-ebcdic. */

#ifndef EBCDIC
#define EBCDIC 0
#endif

/* If you are compiling for a system other than a Unix-like system or Win32,
and it needs some magic to be inserted before the definition of a function that
is exported by the library, define this macro to contain the relevant magic. If
you do not define this macro, it defaults to "extern" for a C compiler and
"extern C" for a C++ compiler on non-Win32 systems. This macro apears at the
start of every exported function that is part of the external API. It does not
appear on functions that are "external" in the C sense, but which are internal
to the library. */

/* #define PCRE_DATA_SCOPE */

/* Define the following macro to empty if the "const" keyword does not work. */

#undef const

/* Define the following macro to "unsigned" if <stddef.h> does not define
size_t. */

#undef size_t

/* The following two definitions are mainly for the benefit of SunOS4, which
does not have the strerror() or memmove() functions that should be present in
all Standard C libraries. The macros HAVE_STRERROR and HAVE_MEMMOVE should
normally be defined with the value 1 for other systems, but unfortunately we
cannot make this the default because "configure" files generated by autoconf
will only change 0 to 1; they won't change 1 to 0 if the functions are not
found. */

#define HAVE_STRERROR 1
#define HAVE_MEMMOVE  1

/* There are some non-Unix-like systems that don't even have bcopy(). If this
macro is false, an emulation is used. If HAVE_MEMMOVE is set to 1, the value of
HAVE_BCOPY is not relevant. */

#define HAVE_BCOPY    0

/* The value of NEWLINE determines the newline character. The default is to
leave it up to the compiler, but some sites want to force a particular value.
On Unix-like systems, "configure" can be used to override this default. */

#ifndef NEWLINE
#define NEWLINE '\n'
#endif

/* The value of LINK_SIZE determines the number of bytes used to store links as
offsets within the compiled regex. The default is 2, which allows for compiled
patterns up to 64K long. This covers the vast majority of cases. However, PCRE
can also be compiled to use 3 or 4 bytes instead. This allows for longer
patterns in extreme cases. On systems that support it, "configure" can be used
to override this default. */

#ifndef LINK_SIZE
#define LINK_SIZE   2
#endif

/* When calling PCRE via the POSIX interface, additional working storage is
required for holding the pointers to capturing substrings because PCRE requires
three integers per substring, whereas the POSIX interface provides only two. If
the number of expected substrings is small, the wrapper function uses space on
the stack, because this is faster than using malloc() for each call. The
threshold above which the stack is no longer used is defined by POSIX_MALLOC_
THRESHOLD. On systems that support it, "configure" can be used to override this
default. */

#ifndef POSIX_MALLOC_THRESHOLD
#define POSIX_MALLOC_THRESHOLD 10
#endif

/* PCRE uses recursive function calls to handle backtracking while matching.
This can sometimes be a problem on systems that have stacks of limited size.
Define NO_RECURSE to get a version that doesn't use recursion in the match()
function; instead it creates its own stack by steam using pcre_recurse_malloc()
to obtain memory from the heap. For more detail, see the comments and other
stuff just above the match() function. On systems that support it, "configure"
can be used to set this in the Makefile (use --disable-stack-for-recursion). */

/* #define NO_RECURSE */

/* The value of MATCH_LIMIT determines the default number of times the internal
match() function can be called during a single execution of pcre_exec(). There
is a runtime interface for setting a different limit. The limit exists in order
to catch runaway regular expressions that take for ever to determine that they
do not match. The default is set very large so that it does not accidentally
catch legitimate cases. On systems that support it, "configure" can be used to
override this default default. */

#ifndef MATCH_LIMIT
#define MATCH_LIMIT 10000000
#endif

/* The above limit applies to all calls of match(), whether or not they
increase the recursion depth. In some environments it is desirable to limit the
depth of recursive calls of match() more strictly, in order to restrict the
maximum amount of stack (or heap, if NO_RECURSE is defined) that is used. The
value of MATCH_LIMIT_RECURSION applies only to recursive calls of match(). To
have any useful effect, it must be less than the value of MATCH_LIMIT. There is
a runtime method for setting a different limit. On systems that support it,
"configure" can be used to override this default default. */

#ifndef MATCH_LIMIT_RECURSION
#define MATCH_LIMIT_RECURSION MATCH_LIMIT
#endif

/* These three limits are parameterized just in case anybody ever wants to
change them. Care must be taken if they are increased, because they guard
against integer overflow caused by enormously large patterns. */

#ifndef MAX_NAME_SIZE
#define MAX_NAME_SIZE 32
#endif

#ifndef MAX_NAME_COUNT
#define MAX_NAME_COUNT 10000
#endif

#ifndef MAX_DUPLENGTH
#define MAX_DUPLENGTH 30000
#endif

/* End */
