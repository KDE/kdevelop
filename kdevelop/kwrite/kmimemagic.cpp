/**
 * KMimeMagic is inspired by the code of the
 * Apache Web Server.
 *
 * Rewritten for KDE by Fritz Elfert
 * fritz@kde.org
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <utime.h>
#include <stdarg.h>
#include <qregexp.h>
#include "kmimemagic.h"

/*
 * data structures and related constants
 */
// Stephan: the CC doesn't support such macros
/*
#define MIME_MAGIC_DEBUG  0
#if MIME_MAGIC_DEBUG
#define debug(fmt, args...)  debugT(fmt "\n" , ## args)
#else
#define debug(fmt, args...)
#endif
*/
#define debug debugT

#define DECLINED 999
#define ERROR    998
#define OK         0

/*
 * Buitltin Mime types
 */
#define MIME_BINARY_UNKNOWN    "application/octet-stream"
#define MIME_BINARY_UNREADABLE "application/x-unreadable"
#define MIME_BINARY_ZEROSIZE   "application/x-zerosize"
#define MIME_TEXT_UNKNOWN      "text/plain"
#define MIME_INODE_DIR         "inode/directory"
#define MIME_INODE_CDEV        "inode/chardevice"
#define MIME_INODE_BDEV        "inode/blockdevice"
#define MIME_INODE_FIFO        "inode/fifo"
#define MIME_INODE_LINK        "inode/link"
#define MIME_INODE_SOCK        "inode/socket"
// Following should go in magic-file - Fritz
#define MIME_APPL_TROFF        "application/x-troff"
#define MIME_APPL_TAR          "application/x-tar"
#define MIME_TEXT_FORTRAN      "text/x-fortran"

#define MAXMIMESTRING        256

#define HOWMANY 1024            /* big enough to recognize most WWW files */
#define MAXDESC   50            /* max leng of text description */
#define MAXstring 64            /* max leng of "string" types */

typedef union VALUETYPE {
	unsigned char b;
	unsigned short h;
	unsigned long l;
	char s[MAXstring];
	unsigned char hs[2];	/* 2 bytes of a fixed-endian "short" */
	unsigned char hl[4];	/* 2 bytes of a fixed-endian "long" */
} VALUETYPE;

struct magic {
	struct magic *next;     /* link to next entry */
	int lineno;             /* line number from magic file */

	short flag;
#define INDIR    1              /* if '>(...)' appears,  */
#define UNSIGNED 2              /* comparison is unsigned */
	short cont_level;       /* level of ">" */
	struct {
		char type;      /* byte short long */
		long offset;    /* offset from indirection */
	} in;
	long offset;            /* offset to magic number */
	unsigned char reln;     /* relation (0=eq, '>'=gt, etc) */
	char type;              /* int, short, long or string. */
	char vallen;            /* length of string value, if any */
#define BYTE       1
#define SHORT      2
#define LONG       4
#define STRING     5
#define DATE       6
#define BESHORT    7
#define BELONG     8
#define BEDATE     9
#define LESHORT   10
#define LELONG    11
#define LEDATE    12
	VALUETYPE value;        /* either number or string */
	unsigned long mask;     /* mask before comparison with value */
	char nospflag;          /* supress space character */

	/* NOTE: this string is suspected of overrunning - find it! */
	char desc[MAXDESC];     /* description */
};

/*
 * data structures for tar file recognition
 * --------------------------------------------------------------------------
 * Header file for public domain tar (tape archive) program.
 *
 * @(#)tar.h 1.20 86/10/29    Public Domain. Created 25 August 1985 by John
 * Gilmore, ihnp4!hoptoad!gnu.
 *
 * Header block on tape.
 *
 * I'm going to use traditional DP naming conventions here. A "block" is a big
 * chunk of stuff that we do I/O on. A "record" is a piece of info that we
 * care about. Typically many "record"s fit into a "block".
 */
#define RECORDSIZE    512
#define NAMSIZ    100
#define TUNMLEN    32
#define TGNMLEN    32

union record {
	char charptr[RECORDSIZE];
	struct header {
		char name[NAMSIZ];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char linkflag;
		char linkname[NAMSIZ];
		char magic[8];
		char uname[TUNMLEN];
		char gname[TGNMLEN];
		char devmajor[8];
		char devminor[8];
	} header;
};

/* The magic field is filled with this if uname and gname are valid. */
#define    TMAGIC        "ustar  "	/* 7 chars and a null */

/*
 * file-function prototypes
 */
static int is_tar(unsigned char *, int);
static unsigned long signextend(struct magic *, unsigned long);
static int getvalue(struct magic *, char **);
static int hextoint(int);
static char *getstr(char *, char *, int, int *);
static int parse(char *, int);
static int mget(union VALUETYPE *, unsigned char *, struct magic *, int);
static int mcheck(union VALUETYPE *, struct magic *);
static int mconvert(union VALUETYPE *, struct magic *);
static long from_oct(int, char *);

/*
 * includes for ASCII substring recognition formerly "names.h" in file
 * command
 *
 * Original notes: names and types used by ascmagic in file(1).
 * These tokens are
 * here because they can appear anywhere in the first HOWMANY bytes, while
 * tokens in /etc/magic must appear at fixed offsets into the file. Don't
 * make HOWMANY too high unless you have a very fast CPU.
 */

/* these types are used calculate index to 'types': keep em in sync! */
/* HTML inserted in first because this is a web server module now */
#define L_HTML   0x001          /* HTML */
#define L_C      0x002          /* first and foremost on UNIX */
#define L_MAKE   0x004          /* Makefiles */
#define L_PLI    0x008          /* PL/1 */
#define L_MACH   0x010          /* some kinda assembler */
#define L_ENG    0x020          /* English */
#define L_PAS    0x040          /* Pascal */
#define L_JAVA   0x080          /* Java source */
#define L_CPP    0x100          /* C++ */
#define L_MAIL   0x200          /* Electronic mail */
#define L_NEWS   0x400          /* Usenet Netnews */

#define P_HTML   0          /* HTML */
#define P_C      1          /* first and foremost on UNIX */
#define P_MAKE   2          /* Makefiles */
#define P_PLI    3          /* PL/1 */
#define P_MACH   4          /* some kinda assembler */
#define P_ENG    5          /* English */
#define P_PAS    6          /* Pascal */
#define P_JAVA   7          /* Java source */
#define P_CPP    8          /* C++ */
#define P_MAIL   9          /* Electronic mail */
#define P_NEWS  10          /* Usenet Netnews */

typedef struct asc_type {
	char *type;
	int  kwords;
	double  weight;
} asc_type;

static asc_type types[] = {
	{ "text/html",         10, 1.2 },
	{ "text/x-c",           9, 1.3 },
	{ "text/x-makefile",    4, 1.9 },
	{ "text/x-pli",         1, 3 },
	{ "text/x-assembler",   6, 2.1 },
	{ "text/english",       2, 0.2 },
	{ "text/x-pascal",      1, 1 },
	{ "text/x-java",       14, 1 },
	{ "text/x-c++",        14, 1 },
	{ "message/rfc822",     4, 1.9 },
	{ "message/news",       3, 2 }
};

#define NTYPES (sizeof(types)/sizeof(asc_type))

static struct names {
	char *name;
	short type;
} names[] = {

	/* These must be sorted by eye for optimal hit rate */
	/* Add to this list only after substantial meditation */
	{
		"<html>", L_HTML
	},
	{
		"<HTML>", L_HTML
	},
	{
		"<head>", L_HTML
	},
	{
		"<HEAD>", L_HTML
	},
	{
		"<title>", L_HTML
	},
	{
		"<TITLE>", L_HTML
	},
	{
		"<h1>", L_HTML
	},
	{
		"<H1>", L_HTML
	},
	{
		"<!--", L_HTML
	},
	{
		"<!DOCTYPE HTML", L_HTML
	},
	{
		"/*", L_C|L_CPP|L_JAVA
	},                      /* must precede "The", "the", etc. */
	{
		"//", L_CPP|L_JAVA
	},                      /* must precede "The", "the", etc. */
	{
		"#include", L_C|L_CPP
	},
	{
		"char", L_C|L_CPP|L_JAVA
	},
	{
		"The", L_ENG
	},
	{
		"the", L_ENG
	},
	{
		"double", L_C|L_CPP|L_JAVA
	},
	{
		"extern", L_C|L_CPP
	},
	{
		"float", L_C|L_CPP|L_JAVA
	},
	{
		"real", L_C|L_CPP|L_JAVA
	},
	{
		"struct", L_C|L_CPP
	},
	{
		"union", L_C|L_CPP
	},
	{
		"implements", L_JAVA
	},
	{
		"package", L_JAVA
	},
	{
		"super", L_JAVA
	},
	{
		"import", L_JAVA
	},
	{
		"this", L_CPP|L_JAVA
	},
	{
		"class", L_CPP|L_JAVA
	},
	{
		"public", L_CPP|L_JAVA
	},
	{
		"private", L_CPP|L_JAVA
	},
	{
		"CFLAGS", L_MAKE
	},
	{
		"LDFLAGS", L_MAKE
	},
	{
		"all:", L_MAKE
	},
	{
		".PRECIOUS", L_MAKE
	},
	/*
	 * Too many files of text have these words in them.  Find another way
	 * to recognize Fortrash.
	 */
	{
		".ascii", L_MACH
	},
	{
		".asciiz", L_MACH
	},
	{
		".byte", L_MACH
	},
	{
		".even", L_MACH
	},
	{
		".globl", L_MACH
	},
	{
		"clr", L_MACH
	},
	{
		"(input", L_PAS
	},
	{
		"dcl", L_PLI
	},
	{
		"Received:", L_MAIL
	},
	{
		">From", L_MAIL
	},
	{
		"Return-Path:", L_MAIL
	},
	{
		"Cc:", L_MAIL
	},
	{
		"Newsgroups:", L_NEWS
	},
	{
		"Path:", L_NEWS
	},
	{
		"Organization:", L_NEWS
	},
	{
		NULL, 0
	}
};

/* current config */
typedef struct {
	char *magicfile;        /* where magic be found      */
	struct magic *magic,    /* head of magic config list */
	*last;
} config_rec;

static int accuracy;

config_rec *conf;

void  error( const char *msg, ... )
{
    va_list ap;
    va_start( ap, msg );  
    // use variable arg list
    QString tmp = msg;
    tmp += "\n";
    vfprintf( stderr, tmp.data() , ap );
    va_end( ap );
    accuracy = 0;
}

#if (MIME_MAGIC_DEBUG > 1)
static void
test_table()
{
	struct magic *m;
	struct magic *prevm = NULL;

	// debug("%s: started", __FUNCTION__);
	for (m = conf->magic; m; m = m->next) {
		if (isprint((((unsigned long) m) >> 24) & 255) &&
		    isprint((((unsigned long) m) >> 16) & 255) &&
		    isprint((((unsigned long) m) >> 8) & 255) &&
		    isprint(((unsigned long) m) & 255)) {
		    //debug("%s: POINTER CLOBBERED! "
		    //"m=\"%c%c%c%c\" line=%d", __FUNCTION__,
			      (((unsigned long) m) >> 24) & 255,
			      (((unsigned long) m) >> 16) & 255,
			      (((unsigned long) m) >> 8) & 255,
			      ((unsigned long) m) & 255,
			      prevm ? prevm->lineno : -1);
			break;
		}
		prevm = m;
	}
}
#endif

#define    EATAB {while (isascii((unsigned char) *l) && \
	      isspace((unsigned char) *l))  ++l;}

static int
parse_line(char *line, int *rule, int lineno)
{
	int ws_offset;
	
	/* delete newline */
	if (line[0]) {
		line[strlen(line) - 1] = '\0';
	}
	/* skip leading whitespace */
	ws_offset = 0;
	while (line[ws_offset] && isspace(line[ws_offset])) {
		ws_offset++;
	}
	
	/* skip blank lines */
	if (line[ws_offset] == 0) {
		return 0;
	}
	/* comment, do not parse */
	if (line[ws_offset] == '#')
		return 0;
	
	/* if we get here, we're going to use it so count it */
	(*rule)++;

	/* parse it */
	return (parse(line + ws_offset, lineno) != 0);
}

/*
 * apprentice - load configuration from the magic file.
 */
static int
apprentice()
{
	FILE *f;
	char line[BUFSIZ + 1];
	int errs = 0;
	int lineno;
	int rule = 0;
	char *fname;

	if (!conf->magicfile)
		return -1;
	fname = conf->magicfile;
	f = fopen(fname, "r");
	if (f == NULL) {
		error("kmimelib: can't read magic file %s: %s",
		      fname, strerror(errno));
		return -1;
	}

	/* parse it */
	for (lineno = 1; fgets(line, BUFSIZ, f) != NULL; lineno++)
		if (parse_line(line, &rule, lineno))
			errs++;

	fclose(f);

#if (MIME_MAGIC_DEBUG > 1)
	debug("%s: conf=%p file=%s m=%s m->next=%s last=%s",
	      __FUNCTION__, conf,
	      conf->magicfile ? conf->magicfile : "NULL",
	      conf->magic ? "set" : "NULL",
	      (conf->magic && conf->magic->next) ? "set" : "NULL",
	      conf->last ? "set" : "NULL");
	debug("%s: read %d lines, %d rules, %d errors",
	      __FUNCTION__, lineno, rule, errs);

	test_table();
#endif

	return (errs ? -1 : 0);
}

static int
buff_apprentice(char *buff)
{
	char line[BUFSIZ + 2];
	int errs = 0;
	int lineno = 1;
	char *start = buff;
	char *end;
	int count = 0;
	int rule = 0;
	int len = strlen(buff) + 1;

	/* parse it */
	do {
		count = (len > BUFSIZ-1)?BUFSIZ-1:len;
		strncpy(line, start, count);
		line[count] = '\0';
		if ((end = strchr(line, '\n'))) {
			*(++end) = '\0';
			count = strlen(line);
		} else
		  strcat(line, "\n");
		start += count;
		len -= count;
		if (parse_line(line, &rule, lineno))
			errs++;
		lineno++;
	} while (len > 0);

#if (MIME_MAGIC_DEBUG > 1)
	debug("%s: conf=%p m=%s m->next=%s last=%s",
	      __FUNCTION__, conf,
	      conf->magic ? "set" : "NULL",
	      (conf->magic && conf->magic->next) ? "set" : "NULL",
	      conf->last ? "set" : "NULL");
	debug("%s: read %d lines, %d rules, %d errors",
	      __FUNCTION__, lineno, rule, errs);

	test_table();
#endif

	return (errs ? -1 : 0);
}

/*
 * extend the sign bit if the comparison is to be signed
 */
static unsigned long
signextend(struct magic *m, unsigned long v)
{
	if (!(m->flag & UNSIGNED))
		switch (m->type) {
				/*
				 * Do not remove the casts below.  They are vital.
				 * When later compared with the data, the sign
				 * extension must have happened.
				 */
			case BYTE:
				v = (char) v;
				break;
			case SHORT:
			case BESHORT:
			case LESHORT:
				v = (short) v;
				break;
			case DATE:
			case BEDATE:
			case LEDATE:
			case LONG:
			case BELONG:
			case LELONG:
				v = (long) v;
				break;
			case STRING:
				break;
			default:
				error("%s: can't happen: m->type=%d",
				      "signextend", m->type);
				return ERROR;
		}
	return v;
}

/*
 * parse one line from magic file, put into magic[index++] if valid
 */
static int
parse(char *l, int lineno)
{
	int i = 0;
	struct magic *m;
	char *t,
	*s;
	/* allocate magic structure entry */
	if ((m = (struct magic *) calloc(1, sizeof(struct magic))) == NULL) {
		error("parse: Out of memory.");
		return -1;
	}
	/* append to linked list */
	m->next = NULL;
	if (!conf->magic || !conf->last) {
		conf->magic = conf->last = m;
	} else {
		conf->last->next = m;
		conf->last = m;
	}

	/* set values in magic structure */
	m->flag = 0;
	m->cont_level = 0;
	m->lineno = lineno;

	while (*l == '>') {
		++l;            /* step over */
		m->cont_level++;
	}

	if (m->cont_level != 0 && *l == '(') {
		++l;            /* step over */
		m->flag |= INDIR;
	}
	/* get offset, then skip over it */
	m->offset = (int) strtol(l, &t, 0);
	if (l == t) {
		error("parse: offset %s invalid", l);
	}
	l = t;

	if (m->flag & INDIR) {
		m->in.type = LONG;
		m->in.offset = 0;
		/*
		 * read [.lbs][+-]nnnnn)
		 */
		if (*l == '.') {
			switch (*++l) {
				case 'l':
					m->in.type = LONG;
					break;
				case 's':
					m->in.type = SHORT;
					break;
				case 'b':
					m->in.type = BYTE;
					break;
				default:
					error("parse: indirect offset type %c invalid", *l);
					break;
			}
			l++;
		}
		s = l;
		if (*l == '+' || *l == '-')
			l++;
		if (isdigit((unsigned char) *l)) {
			m->in.offset = strtol(l, &t, 0);
			if (*s == '-')
				m->in.offset = -m->in.offset;
		} else
			t = l;
		if (*t++ != ')') {
			error("parse: missing ')' in indirect offset");
		}
		l = t;
	}
	while (isascii((unsigned char) *l) && isdigit((unsigned char) *l))
		++l;
	EATAB;

#define NBYTE       4
#define NSHORT      5
#define NLONG       4
#define NSTRING     6
#define NDATE       4
#define NBESHORT    7
#define NBELONG     6
#define NBEDATE     6
#define NLESHORT    7
#define NLELONG     6
#define NLEDATE     6

	if (*l == 'u') {
		++l;
		m->flag |= UNSIGNED;
	}
	/* get type, skip it */
	if (strncmp(l, "byte", NBYTE) == 0) {
		m->type = BYTE;
		l += NBYTE;
	} else if (strncmp(l, "short", NSHORT) == 0) {
		m->type = SHORT;
		l += NSHORT;
	} else if (strncmp(l, "long", NLONG) == 0) {
		m->type = LONG;
		l += NLONG;
	} else if (strncmp(l, "string", NSTRING) == 0) {
		m->type = STRING;
		l += NSTRING;
	} else if (strncmp(l, "date", NDATE) == 0) {
		m->type = DATE;
		l += NDATE;
	} else if (strncmp(l, "beshort", NBESHORT) == 0) {
		m->type = BESHORT;
		l += NBESHORT;
	} else if (strncmp(l, "belong", NBELONG) == 0) {
		m->type = BELONG;
		l += NBELONG;
	} else if (strncmp(l, "bedate", NBEDATE) == 0) {
		m->type = BEDATE;
		l += NBEDATE;
	} else if (strncmp(l, "leshort", NLESHORT) == 0) {
		m->type = LESHORT;
		l += NLESHORT;
	} else if (strncmp(l, "lelong", NLELONG) == 0) {
		m->type = LELONG;
		l += NLELONG;
	} else if (strncmp(l, "ledate", NLEDATE) == 0) {
		m->type = LEDATE;
		l += NLEDATE;
	} else {
		error("parse: type %s invalid", l);
		return -1;
	}
	/* New-style anding: "0 byte&0x80 =0x80 dynamically linked" */
	if (*l == '&') {
		++l;
		m->mask = signextend(m, strtol(l, &l, 0));
	} else
		m->mask = ~0L;
	EATAB;

	switch (*l) {
		case '>':
		case '<':
			/* Old-style anding: "0 byte &0x80 dynamically linked" */
		case '&':
		case '^':
		case '=':
			m->reln = *l;
			++l;
			break;
		case '!':
			if (m->type != STRING) {
				m->reln = *l;
				++l;
				break;
			}
			/* FALL THROUGH */
		default:
			if (*l == 'x' && isascii((unsigned char) l[1]) &&
			    isspace((unsigned char) l[1])) {
				m->reln = *l;
				++l;
				goto GetDesc;	/* Bill The Cat */
			}
			m->reln = '=';
			break;
	}
	EATAB;

	if (getvalue(m, &l))
		return -1;
	/*
	 * now get last part - the description
	 */
      GetDesc:
	EATAB;
	if (l[0] == '\b') {
		++l;
		m->nospflag = 1;
	} else if ((l[0] == '\\') && (l[1] == 'b')) {
		++l;
		++l;
		m->nospflag = 1;
	} else
		m->nospflag = 0;
	while ((m->desc[i++] = *l++) != '\0' && i < MAXDESC)
		/* NULLBODY */ ;

#if (MIME_MAGIC_DEBUG > 1)
	debug("%s: line=%d m=%p next=%p cont=%d desc=%s",
	      __FUNCTION__, lineno, m, m->next, m->cont_level,
	      m->desc ? m->desc : "NULL");
#endif                          /* MIME_MAGIC_DEBUG */

	return 0;
}

/*
 * Read a numeric value from a pointer, into the value union of a magic
 * pointer, according to the magic type.  Update the string pointer to point
 * just after the number read.  Return 0 for success, non-zero for failure.
 */
static int
getvalue(struct magic *m, char **p)
{
	int slen;

	if (m->type == STRING) {
		*p = getstr(*p, m->value.s, sizeof(m->value.s), &slen);
		m->vallen = slen;
	} else if (m->reln != 'x')
		m->value.l = signextend(m, strtol(*p, p, 0));
	return 0;
}

/*
 * Convert a string containing C character escapes.  Stop at an unescaped
 * space or tab. Copy the converted version to "p", returning its length in
 * *slen. Return updated scan pointer as function result.
 */
static char *
getstr(register char *s, register char *p, int plen, int *slen)
{
	char *origs = s,
	*origp = p;
	char *pmax = p + plen - 1;
	register int c;
	register int val;

	while ((c = *s++) != '\0') {
		if (isspace((unsigned char) c))
			break;
		if (p >= pmax) {
			error("String too long: %s", origs);
			break;
		}
		if (c == '\\') {
			switch (c = *s++) {

				case '\0':
					goto out;

				default:
					*p++ = (char) c;
					break;

				case 'n':
					*p++ = '\n';
					break;

				case 'r':
					*p++ = '\r';
					break;

				case 'b':
					*p++ = '\b';
					break;

				case 't':
					*p++ = '\t';
					break;

				case 'f':
					*p++ = '\f';
					break;

				case 'v':
					*p++ = '\v';
					break;

					/* \ and up to 3 octal digits */
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					val = c - '0';
					c = *s++;	/* try for 2 */
					if (c >= '0' && c <= '7') {
						val = (val << 3) | (c - '0');
						c = *s++;	/* try for 3 */
						if (c >= '0' && c <= '7')
							val = (val << 3) | (c - '0');
						else
							--s;
					} else
						--s;
					*p++ = (char) val;
					break;

					/* \x and up to 3 hex digits */
				case 'x':
					val = 'x';	/* Default if no digits */
					c = hextoint(*s++);	/* Get next char */
					if (c >= 0) {
						val = c;
						c = hextoint(*s++);
						if (c >= 0) {
							val = (val << 4) + c;
							c = hextoint(*s++);
							if (c >= 0) {
								val = (val << 4) + c;
							} else
								--s;
						} else
							--s;
					} else
						--s;
					*p++ = (char) val;
					break;
			}
		} else
			*p++ = (char) c;
	}
      out:
	*p = '\0';
	*slen = p - origp;
	return s;
}


/* Single hex char to int; -1 if not a hex char. */
static int
hextoint(int c)
{
	if (!isascii((unsigned char) c))
		return -1;
	if (isdigit((unsigned char) c))
		return c - '0';
	if ((c >= 'a') && (c <= 'f'))
		return c + 10 - 'a';
	if ((c >= 'A') && (c <= 'F'))
		return c + 10 - 'A';
	return -1;
}

/*
 * Convert the byte order of the data we are looking at
 */
static int
mconvert(union VALUETYPE *p, struct magic *m)
{
	char *rt;

	switch (m->type) {
		case BYTE:
			return 1;
		case STRING:
			/* Null terminate and eat the return */
			p->s[sizeof(p->s) - 1] = '\0';
			if ((rt = strchr(p->s, '\n')) != NULL)
				*rt = '\0';
			return 1;
#ifndef WORDS_BIGENDIAN
		case SHORT:
#endif
		case BESHORT:
			p->h = (short) ((p->hs[0] << 8) | (p->hs[1]));
			return 1;
#ifndef WORDS_BIGENDIAN
		case LONG:
		case DATE:
#endif
		case BELONG:
		case BEDATE:
			p->l = (long)
			    ((p->hl[0] << 24) | (p->hl[1] << 16) | (p->hl[2] << 8) | (p->hl[3]));
			return 1;
#ifdef WORDS_BIGENDIAN
		case SHORT:
#endif
		case LESHORT:
			p->h = (short) ((p->hs[1] << 8) | (p->hs[0]));
			return 1;
#ifdef WORDS_BIGENDIAN
		case LONG:
		case DATE:
#endif
		case LELONG:
		case LEDATE:
			p->l = (long)
			    ((p->hl[3] << 24) | (p->hl[2] << 16) | (p->hl[1] << 8) | (p->hl[0]));
			return 1;
		default:
			error("mconvert: invalid type %d", m->type);
			return 0;
	}
}


static int
mget(union VALUETYPE *p, unsigned char *s, struct magic *m,
     int nbytes)
{
	long offset = m->offset;
	if (offset + (int)sizeof(union VALUETYPE) > nbytes)
		 return 0;


	memcpy(p, s + offset, sizeof(union VALUETYPE));

	if (!mconvert(p, m))
		return 0;

	if (m->flag & INDIR) {

		switch (m->in.type) {
			case BYTE:
				offset = p->b + m->in.offset;
				break;
			case SHORT:
				offset = p->h + m->in.offset;
				break;
			case LONG:
				offset = p->l + m->in.offset;
				break;
		}

		if (offset + (int)sizeof(union VALUETYPE) > nbytes)
			 return 0;

		memcpy(p, s + offset, sizeof(union VALUETYPE));

		if (!mconvert(p, m))
			return 0;
	}
	return 1;
}

static int
mcheck(union VALUETYPE *p, struct magic *m)
{
	register unsigned long l = m->value.l;
	register unsigned long v;
	int matched;

	if ((m->value.s[0] == 'x') && (m->value.s[1] == '\0')) {
		error("BOINK");
		return 1;
	}
	switch (m->type) {
		case BYTE:
			v = p->b;
			break;

		case SHORT:
		case BESHORT:
		case LESHORT:
			v = p->h;
			break;

		case LONG:
		case BELONG:
		case LELONG:
		case DATE:
		case BEDATE:
		case LEDATE:
			v = p->l;
			break;

		case STRING:
			l = 0;
			/*
			 * What we want here is: v = strncmp(m->value.s, p->s,
			 * m->vallen); but ignoring any nulls.  bcmp doesn't give
			 * -/+/0 and isn't universally available anyway.
			 */
			v = 0;
			{
				register unsigned char *a = (unsigned char *) m->value.s;
				register unsigned char *b = (unsigned char *) p->s;
				register int len = m->vallen;

				while (--len >= 0)
					if ((v = *b++ - *a++) != 0)
						break;
			}
			break;
		default:
			error("mcheck: invalid type %d", m->type);
			return 0;	/* NOTREACHED */
	}
#if 0
	debug("Before signextend %08x", v);
#endif
	v = signextend(m, v) & m->mask;
#if 0
	debug("After signextend %08x", v);
#endif

	switch (m->reln) {
		case 'x':
			matched = 1;
			break;

		case '!':
			matched = v != l;
			break;

		case '=':
			matched = v == l;
			break;

		case '>':
			if (m->flag & UNSIGNED)
				matched = v > l;
			else
				matched = (long) v > (long) l;
			break;

		case '<':
			if (m->flag & UNSIGNED)
				matched = v < l;
			else
				matched = (long) v < (long) l;
			break;

		case '&':
			matched = (v & l) == l;
			break;

		case '^':
			matched = (v & l) != l;
			break;

		default:
			matched = 0;
			error("mcheck: can't happen: invalid relation %d.", m->reln);
			break;  /* NOTREACHED */
	}

	return matched;
}

/* hook for printf-type functions */
void
KMimeMagic::resultBufPrintf(char *str,...)
{
	va_list ap;

	char buf[MAXMIMESTRING];

	/* assemble the string into the buffer */
	va_start(ap, str);
	vsprintf(buf, str, ap);
	va_end(ap);

	/* add the buffer to the list */
	resultBuf += QString(buf);
}

/* states for the state-machine algorithm in finishResult() */
typedef enum {
	rsl_leading_space, rsl_type, rsl_subtype, rsl_separator, rsl_encoding
} rsl_states;

/* process resultBuf and set the MIME info in magicResult */
int
KMimeMagic::finishResult()
{
	int cur_pos,            /* current position within result */
	 type_pos,              /* content type starting point: position */
	 type_len,              /* content type length */
	 encoding_pos,          /* content encoding starting point: position */
	 encoding_len;          /* content encoding length */

	int state;
	/* start searching for the type and encoding */
	state = rsl_leading_space;
	type_pos = type_len = 0;
	encoding_pos = encoding_len = 0;
	/* loop through the characters in the result */
	for (cur_pos = 0; cur_pos < (int)resultBuf.length(); cur_pos++) {
		if (isspace(resultBuf.at(cur_pos))) {
			/* process whitespace actions for each state */
			if (state == rsl_leading_space) {
				/* eat whitespace in this state */
				continue;
			} else if (state == rsl_type) {
				/* whitespace: type has no slash! */
				return DECLINED;
			} else if (state == rsl_subtype) {
				/* whitespace: end of MIME type */
				state++;
				continue;
			} else if (state == rsl_separator) {
				/* eat whitespace in this state */
				continue;
			} else if (state == rsl_encoding) {
				/* whitespace: end of MIME encoding */
				/* we're done */
				break;
			} else {
				/* should not be possible */
				/* abandon malfunctioning module */
				error("KMimeMagic::finishResult: bad state %d (ws)", state);
				return DECLINED;
			}
			/* NOTREACHED */
		} else if (state == rsl_type &&
			   resultBuf.at(cur_pos) == '/') {
			/* copy the char and go to rsl_subtype state */
			type_len++;
			state++;
		} else {
			/* process non-space actions for each state */
			if (state == rsl_leading_space) {
				/* non-space: begin MIME type */
				state++;
				type_pos = cur_pos;
				type_len = 1;
				continue;
			} else if (state == rsl_type ||
				   state == rsl_subtype) {
				/* non-space: adds to type */
				type_len++;
				continue;
			} else if (state == rsl_separator) {
				/* non-space: begin MIME encoding */
				state++;
				encoding_pos = cur_pos;
				encoding_len = 1;
				continue;
			} else if (state == rsl_encoding) {
				/* non-space: adds to encoding */
				encoding_len++;
				continue;
			} else {
				/* should not be possible */
				/* abandon malfunctioning module */
				error(" KMimeMagic::finishResult: bad state %d (ns)", state);
				return DECLINED;
			}
			/* NOTREACHED */
		}
		/* NOTREACHED */
	}

	/* if we ended prior to state rsl_subtype, we had incomplete info */
	if (state != rsl_subtype && state != rsl_separator &&
	    state != rsl_encoding) {
		/* defer to other modules */
		return DECLINED;
	}
	/* save the info in the request record */
	if (state == rsl_subtype || state == rsl_encoding ||
	    state == rsl_encoding || state == rsl_separator) {
		magicResult->setContent(resultBuf.mid(type_pos, type_len));
	}
	if (state == rsl_encoding)
		magicResult->setEncoding(resultBuf.mid(encoding_pos,
						       encoding_len));
	/* detect memory allocation errors */
	if (!magicResult->getContent() ||
	    (state == rsl_encoding && !magicResult->getEncoding())) {
		return -1;
	}
	/* success! */
	return OK;
}

/*
 * magic_process - process input file fn. Opens the file and reads a
 * fixed-size buffer to begin processing the contents.
 */
void
KMimeMagic::process(const char * fn)
{
	int fd = 0;
	unsigned char buf[HOWMANY + 1];	/* one extra for terminating '\0' */
	struct utimbuf utbuf;
	struct stat sb;
	int nbytes = 0;         /* number of bytes read from a datafile */

	/*
	 * first try judging the file based on its filesystem status
	 */
	if (fsmagic(fn, &sb) != 0) {
		resultBuf += "\n";
		return;
	}
	if ((fd = open(fn, O_RDONLY)) < 0) {
		/* We can't open it, but we were able to stat it. */
		/*
		 * if (sb.st_mode & 0002) addResult("writable, ");
		 * if (sb.st_mode & 0111) addResult("executable, ");
		 */
		error("can't read `%s' (%s).", fn, strerror(errno));
		resultBuf += MIME_BINARY_UNREADABLE;
		return;
	}
	/*
	 * try looking at the first HOWMANY bytes
	 */
	if ((nbytes = read(fd, (char *) buf, HOWMANY)) == -1) {
		error("%s read failed (%s).", fn, strerror(errno));
		resultBuf += MIME_BINARY_UNREADABLE;
		return;
		/* NOTREACHED */
	}
	if (nbytes == 0) {
		resultBuf += MIME_BINARY_ZEROSIZE;
	} else {
		buf[nbytes++] = '\0';	/* null-terminate it */
		tryit(buf, nbytes);
	}

	/*
	 * Try to restore access, modification times if read it.
	 */
	utbuf.actime = sb.st_atime;
	utbuf.modtime = sb.st_mtime;
	(void) utime(fn, &utbuf);	/* don't care if loses */
	(void) close(fd);
	resultBuf += "\n";
}


void
KMimeMagic::tryit(unsigned char *buf, int nb)
{
	/*
	 * try tests in /etc/magic (or surrogate magic file)
	 */
	if (softmagic(buf, nb) == 1)
		return;

	/*
	 * try known keywords, check for ascii-ness too.
	 */
	if (ascmagic(buf, nb) == 1)
		return;

	/*
	 * abandon hope, all ye who remain here
	 */
	resultBuf += MIME_BINARY_UNKNOWN;
	accuracy = 0;
}

int
KMimeMagic::fsmagic(const char *fn, struct stat *sb)
{
	int ret = 0;

	/*
	 * Fstat is cheaper but fails for files you don't have read perms on.
	 * On 4.2BSD and similar systems, use lstat() to identify symlinks.
	 */
	ret = lstat(fn, sb);  /* don't merge into if; see "ret =" above */

	if (ret) {
		return 1;
	}
	/*
	 * if (sb->st_mode & S_ISUID) resultBuf += "setuid ";
	 * if (sb->st_mode & S_ISGID) resultBuf += "setgid ";
	 * if (sb->st_mode & S_ISVTX) resultBuf += "sticky ";
	 */

	switch (sb->st_mode & S_IFMT) {
		case S_IFDIR:
			resultBuf += MIME_INODE_DIR;
			return 1;
		case S_IFCHR:
			resultBuf += MIME_INODE_CDEV;
			return 1;
		case S_IFBLK:
			resultBuf += MIME_INODE_BDEV;
			return 1;
			/* TODO add code to handle V7 MUX and Blit MUX files */
#ifdef    S_IFIFO
		case S_IFIFO:
			resultBuf += MIME_INODE_FIFO;;
			return 1;
#endif
#ifdef    S_IFLNK
		case S_IFLNK:
			{
				char buf[BUFSIZ + BUFSIZ + 4];
				register int nch;
				struct stat tstatbuf;

				if ((nch = readlink(fn, buf, BUFSIZ - 1)) <= 0) {
					resultBuf += MIME_INODE_LINK;
					resultBuf += "\nunreadable";
					return 1;
				}
				buf[nch] = '\0'; /* readlink(2) forgets this */
				/* If broken symlink, say so and quit early. */
				if (*buf == '/') {
					if (stat(buf, &tstatbuf) < 0) {
						resultBuf += MIME_INODE_LINK;
						resultBuf += "\nbroken";
						return 1;
					}
				} else {
					char *tmp;
					char buf2[BUFSIZ + BUFSIZ + 4];
					
					strcpy(buf2, fn);
					if ((tmp = strrchr(buf2, '/')) == NULL) {
						tmp = buf; /* in current dir */
					} else {
						/* dir part plus (rel.) link */
						*++tmp = '\0';
						strcat(buf2, buf);
						tmp = buf2;
					}
					if (stat(tmp, &tstatbuf) < 0) {
						resultBuf += MIME_INODE_LINK;
						resultBuf += "\nbroken";
						return 1;
					} else
						strcpy(buf, tmp);
				}
				if (followLinks)
					process(buf);
				else
					resultBuf += MIME_INODE_LINK;
				return 1;
			}
			return 1;
#endif
#ifdef    S_IFSOCK
#ifndef __COHERENT__
		case S_IFSOCK:
			resultBuf += MIME_INODE_SOCK;
			return 1;
#endif
#endif
		case S_IFREG:
			break;
		default:
			error("KMimeMagic::fsmagic: invalid mode 0%o.", sb->st_mode);
			/* NOTREACHED */
	}

	/*
	 * regular file, check next possibility
	 */
	if (sb->st_size == 0) {
		resultBuf += MIME_BINARY_ZEROSIZE;
		return 1;
	}
	return 0;
}

/*
 * softmagic - lookup one file in database (already read from /etc/magic by
 * apprentice.c). Passed the name and FILE * of one file to be typed.
 */
int
KMimeMagic::softmagic(unsigned char *buf, int nbytes)
{
	if (match(buf, nbytes))
		return 1;

	return 0;
}

/*
 * Go through the whole list, stopping if you find a match.  Process all the
 * continuations of that match before returning.
 *
 * We support multi-level continuations:
 *
 * At any time when processing a successful top-level match, there is a current
 * continuation level; it represents the level of the last successfully
 * matched continuation.
 *
 * Continuations above that level are skipped as, if we see one, it means that
 * the continuation that controls them - i.e, the lower-level continuation
 * preceding them - failed to match.
 *
 * Continuations below that level are processed as, if we see one, it means
 * we've finished processing or skipping higher-level continuations under the
 * control of a successful or unsuccessful lower-level continuation, and are
 * now seeing the next lower-level continuation and should process it.  The
 * current continuation level reverts to the level of the one we're seeing.
 *
 * Continuations at the current level are processed as, if we see one, there's
 * no lower-level continuation that may have failed.
 *
 * If a continuation matches, we bump the current continuation level so that
 * higher-level continuations are processed.
 */
int
KMimeMagic::match(unsigned char *s, int nbytes)
{
#if (MIME_MAGIC_DEBUG > 1)
	int rule_counter = 0;
#endif
	int cont_level = 0;
	int need_separator = 0;
	union VALUETYPE p;
	struct magic *m;

#if (MIME_MAGIC_DEBUG > 1)
	debug("%s: conf=%p file=%s m=%s m->next=%s last=%s",
	      __FUNCTION__, conf,
	      conf->magicfile ? conf->magicfile : "NULL",
	      conf->magic ? "set" : "NULL",
	      (conf->magic && conf->magic->next) ? "set" : "NULL",
	      conf->last ? "set" : "NULL");
	for (m = conf->magic; m; m = m->next) {
		if (isprint((((unsigned long) m) >> 24) & 255) &&
		    isprint((((unsigned long) m) >> 16) & 255) &&
		    isprint((((unsigned long) m) >> 8) & 255) &&
		    isprint(((unsigned long) m) & 255)) {
			debug("%s: POINTER CLOBBERED! "
			      "m=\"%c%c%c%c\"", __FUNCTION__,
			      (((unsigned long) m) >> 24) & 255,
			      (((unsigned long) m) >> 16) & 255,
			      (((unsigned long) m) >> 8) & 255,
			      ((unsigned long) m) & 255);
			break;
		}
	}
#endif

	for (m = conf->magic; m; m = m->next) {
#if (MIME_MAGIC_DEBUG > 1)
		rule_counter++;
		debug("%s: line=%d desc=%s", __FUNCTION__,
		      m->lineno, m->desc);
#endif

		/* check if main entry matches */
		if (!mget(&p, s, m, nbytes) ||
		    !mcheck(&p, m)) {
			struct magic *m_cont;

			/*
			 * main entry didn't match, flush its continuations
			 */
			if (!m->next || (m->next->cont_level == 0)) {
				continue;
			}
			m_cont = m->next;
			while (m_cont && (m_cont->cont_level != 0)) {
#if (MIME_MAGIC_DEBUG > 1)
				rule_counter++;
				debug("%s: line=%d mc=%p mc->next=%p "
				      "cont=%d desc=%s",
				      __FUNCTION__, m_cont->lineno, m_cont,
				      m_cont->next, m_cont->cont_level,
				   m_cont->desc ? m_cont->desc : "NULL");
#endif
				/*
				 * this trick allows us to keep *m in sync
				 * when the continue advances the pointer
				 */
				m = m_cont;
				m_cont = m_cont->next;
			}
			continue;
		}
		/* if we get here, the main entry rule was a match */
		/* this will be the last run through the loop */
#if (MIME_MAGIC_DEBUG > 1)
		debug("%s: rule matched, line=%d type=%d %s",
		      __FUNCTION__, m->lineno, m->type,
		      (m->type == STRING) ? m->value.s : "");
#endif

		/* print the match */
		mprint(&p, m);

		/*
		 * If we printed something, we'll need to print a blank
		 * before we print something else.
		 */
		if (m->desc[0])
			need_separator = 1;
		/* and any continuations that match */
		cont_level++;
		/*
		 * while (m && m->next && m->next->cont_level != 0 && ( m =
		 * m->next ))
		 */
		m = m->next;
		while (m && (m->cont_level != 0)) {
#if (MIME_MAGIC_DEBUG > 1)
			debug("%s: line=%d cont=%d type=%d %s",
			      __FUNCTION__, m->lineno, m->cont_level, m->type,
			      (m->type == STRING) ? m->value.s : "");
#endif
			if (cont_level >= m->cont_level) {
				if (cont_level > m->cont_level) {
					/*
					 * We're at the end of the level
					 * "cont_level" continuations.
					 */
					cont_level = m->cont_level;
				}
				if (mget(&p, s, m, nbytes) &&
				    mcheck(&p, m)) {
					/*
					 * This continuation matched. Print
					 * its message, with a blank before
					 * it if the previous item printed
					 * and this item isn't empty.
					 */
					/* space if previous printed */
					if (need_separator
					    && (m->nospflag == 0)
					    && (m->desc[0] != '\0')
					    ) {
						resultBuf += " ";
						need_separator = 0;
					}
					mprint(&p, m);
					if (m->desc[0])
						need_separator = 1;

					/*
					 * If we see any continuations at a
					 * higher level, process them.
					 */
					cont_level++;
				}
			}
			/* move to next continuation record */
			m = m->next;
		}
#if (MIME_MAGIC_DEBUG > 1)
		debug("%s: matched after %d rules", __FUNCTION__, rule_counter);
#endif
		return 1;       /* all through */
	}
#if (MIME_MAGIC_DEBUG > 1)
	debug("%s: failed after %d rules", __FUNCTION__, rule_counter);
#endif
	return 0;               /* no match at all */
}

void
KMimeMagic::mprint(union VALUETYPE *p, struct magic *m)
{
	char *pp,
	*rt;
	unsigned long v;

	switch (m->type) {
		case BYTE:
			v = p->b;
			break;

		case SHORT:
		case BESHORT:
		case LESHORT:
			v = p->h;
			break;

		case LONG:
		case BELONG:
		case LELONG:
			v = p->l;
			break;

		case STRING:
			if (m->reln == '=') {
				resultBufPrintf(m->desc, m->value.s);
			} else {
				resultBufPrintf(m->desc, p->s);
			}
			return;

		case DATE:
		case BEDATE:
		case LEDATE:
			pp = ctime((time_t *) & p->l);
			if ((rt = strchr(pp, '\n')) != NULL)
				*rt = '\0';
			resultBufPrintf(m->desc, pp);
			return;
		default:
			error("KMimeMagic::mprint: invalid m->type (%d)", m->type);
			return;
	}

	v = signextend(m, v) & m->mask;
	resultBufPrintf(m->desc, (unsigned long) v);
}

/* an optimization over plain strcmp() */
#define    STREQ(a, b)    (*(a) == *(b) && strcmp((a), (b)) == 0)

int
KMimeMagic::ascmagic(unsigned char *buf, int nbytes)
{
	int i;
	double pct, maxpct, pctsum;
	double pcts[NTYPES];
	int mostaccurate, tokencount;
	int typeset, jonly, conly, jconly, cppcomm, ccomm;
	int has_escapes = 0;
	unsigned char *s;
	char nbuf[HOWMANY + 1]; /* one extra for terminating '\0' */
	char *token;
	register struct names *p;
	int typecount[NTYPES];

	/* these are easy, do them first */
	accuracy = 70;

	/*
	 * for troff, look for . + letter + letter or .\"; this must be done
	 * to disambiguate tar archives' ./file and other trash from real
	 * troff input.
	 */
	if (*buf == '.') {
		unsigned char *tp = buf + 1;

		while (isascii(*tp) && isspace(*tp))
			++tp;   /* skip leading whitespace */
		if ((isascii(*tp) && (isalnum(*tp) || *tp == '\\') &&
		     isascii(*(tp + 1)) && (isalnum(*(tp + 1)) || *tp == '"'))) {
			resultBuf += MIME_APPL_TROFF;
			return 1;
		}
	}
	if ((*buf == 'c' || *buf == 'C') &&
	    isascii(*(buf + 1)) && isspace(*(buf + 1))) {
		/* Fortran */
		resultBuf += MIME_TEXT_FORTRAN;
		return 1;
	}
	/* look for tokens - this is expensive! */
	/* make a copy of the buffer here because strtok() will destroy it */
	s = (unsigned char *) memcpy(nbuf, buf, nbytes);
	s[nbytes] = '\0';
	has_escapes = (memchr(s, '\033', nbytes) != NULL);
/*
 * Fritz:
 * Try a little harder on C/C++/Java.
 */
	memset(&typecount, 0, sizeof(typecount));
	typeset = 0;
	jonly = 0;
	conly = 0;
	jconly = 0;
	cppcomm = 0;
	ccomm = 0;
	tokencount = 0;
	// first collect all possible types and count matches
	while ((token = strtok((char *) s, " \t\n\r\f,;")) != NULL) {
		s = NULL;       /* make strtok() keep on tokin' */
		for (p = names; p->name ; p++) {
			if (STREQ(p->name, token)) {
			        tokencount++;
				typeset |= p->type;
				if (p->type == L_JAVA)
					jonly++;
				if ((p->type & (L_C|L_CPP|L_JAVA))
				    == (L_CPP|L_JAVA))
					jconly++;
				if ((p->type & (L_C|L_CPP|L_JAVA))
				    == (L_C|L_CPP))
					conly++;
				if (STREQ(token, "//"))
					cppcomm++;
				if (STREQ(token, "/*"))
					ccomm++;
				for (i = 0; i < (int)NTYPES; i++)
					if ((1 << i) & p->type)
						typecount[i]++;
			}
		}
	}

	if (typeset & (L_C|L_CPP|L_JAVA)) {
		accuracy = 40;
	        if (!(typeset & ~(L_C|L_CPP|L_JAVA))) {
			if (jonly && conly)
			  error("Oops, jonly && conly?!");
			if (jonly) {
				// A java-only token has matched
				resultBuf += QString(types[P_JAVA].type);
				return 1;
			}
			if (jconly) {
				// A non-C (only C++ or Java) token has matched.
				if (typecount[P_JAVA] > typecount[P_CPP])
				  resultBuf += QString(types[P_JAVA].type);
				else
				  resultBuf += QString(types[P_CPP].type);
				return 1;
			}
			if (conly) {
				// Either C or C++, rely on comments.
				if (cppcomm)
				  resultBuf += QString(types[P_CPP].type);
				else
				  resultBuf += QString(types[P_C].type);
				return 1;
			}
			if (ccomm) {
				resultBuf += QString(types[P_C].type);
				return 1;
			}
	      }
	}

	/* Neither C, C++ or Java (or all of them without able to distinguish):
	 * Simply take the token-class with the highest
	 * matchcount > 0
	 */
	mostaccurate = -1;
	maxpct = pctsum = 0.0;
	for (i = 0; i < (int)NTYPES; i++) {
	 	pct = (double)typecount[i] / (double)types[i].kwords *
		    (double)types[i].weight;
		pcts[i] = pct;
		pctsum += pct;
		if (pct > maxpct) {
		    maxpct = pct;
		    mostaccurate = i;
		  }
#if MIME_MAGIC_DEBUG
		  printf("%s has %d hits, %d kw, %f -> max = %f\n",
			 types[i].type, typecount[i], types[i].kwords,
			 pct, maxpct);
#endif
	}
	if (mostaccurate >= 0.0) {
		accuracy = (int)(pcts[mostaccurate] / pctsum * 60);
		resultBuf += QString(types[mostaccurate].type);
		return 1;
	}

	switch (is_tar(buf, nbytes)) {
		case 1:
			/* V7 tar archive */
			resultBuf += MIME_APPL_TAR;
			accuracy = 90;
			return 1;
		case 2:
			/* POSIX tar archive */
			resultBuf += MIME_APPL_TAR;
			accuracy = 90;
			return 1;
	}

	for (i = 0; i < nbytes; i++) {
		if (!isascii(*(buf + i)))
			return 0;	/* not all ascii */
	}

	/* all else fails, but it is ascii... */
	accuracy = 90;
	if (has_escapes) {
		/* text with escape sequences */
		/* we leave this open for further differentiation later */
		resultBuf += MIME_TEXT_UNKNOWN;
	} else {
		/* plain text */
		resultBuf += MIME_TEXT_UNKNOWN;
	}
	return 1;
}

/*
 * is_tar() -- figure out whether file is a tar archive.
 *
 * Stolen (by author of file utility) from the public domain tar program: Public
 * Domain version written 26 Aug 1985 John Gilmore (ihnp4!hoptoad!gnu).
 *
 * @(#)list.c 1.18 9/23/86 Public Domain - gnu $Id: mod_mime_magic.c,v 1.7
 * 1997/06/24 00:41:02 ikluft Exp ikluft $
 *
 * Comments changed and some code/comments reformatted for file command by Ian
 * Darwin.
 */

#define    isodigit(c)    ( ((c) >= '0') && ((c) <= '7') )

/*
 * Return 0 if the checksum is bad (i.e., probably not a tar archive), 1 for
 * old UNIX tar file, 2 for Unix Std (POSIX) tar file.
 */

static int
is_tar(unsigned char *buf, int nbytes)
{
	register union record *header = (union record *) buf;
	register int i;
	register long sum,
	 recsum;
	register char *p;

	if (nbytes < (int)sizeof(union record))
		 return 0;

	recsum = from_oct(8, header->header.chksum);

	sum = 0;
	p = header->charptr;
	for (i = sizeof(union record); --i >= 0;) {
		/*
		 * We can't use unsigned char here because of old compilers,
		 * e.g. V7.
		 */
		sum += 0xFF & *p++;
	}

	/* Adjust checksum to count the "chksum" field as blanks. */
	for (i = sizeof(header->header.chksum); --i >= 0;)
		sum -= 0xFF & header->header.chksum[i];
	sum += ' ' * sizeof header->header.chksum;

	if (sum != recsum)
		return 0;       /* Not a tar archive */

	if (0 == strcmp(header->header.magic, TMAGIC))
		return 2;       /* Unix Standard tar archive */

	return 1;               /* Old fashioned tar archive */
}


/*
 * Quick and dirty octal conversion.
 *
 * Result is -1 if the field is invalid (all blank, or nonoctal).
 */
static long
from_oct(int digs, char *where)
{
	register long value;

	while (isspace(*where)) {	/* Skip spaces */
		where++;
		if (--digs <= 0)
			return -1;	/* All blank field */
	}
	value = 0;
	while (digs > 0 && isodigit(*where)) {	/* Scan til nonoctal */
		value = (value << 3) | (*where++ - '0');
		--digs;
	}

	if (digs > 0 && *where && !isspace(*where))
		return -1;      /* Ended on non-space/nul */

	return value;
}

/*
 * Check for file-revision suffix
 *
 * This is for an obscure document control system used on an intranet.
 * The web representation of each file's revision has an @1, @2, etc
 * appended with the revision number.  This needs to be stripped off to
 * find the file suffix, which can be recognized by sending the name back
 * through a sub-request.  The base file name (without the @num suffix)
 * must exist because its type will be used as the result.
 */

/*
 * Don't know if we really need this within KDE?!
 * ... well i've to look into original file code.
 */
KMimeMagicResult *
KMimeMagic::revision_suffix(const char * fn)
{
	int suffix_pos;
	QString newfn = QString(fn);

#if (MIME_MAGIC_DEBUG > 2)
	debug("%s: checking %s", __FUNCTION__, fn);
#endif
	/* check for recognized revision suffix */
	suffix_pos = newfn.findRev(QRegExp("@[0-9]*$"));
#if (MIME_MAGIC_DEBUG > 2)
	debug("%s: suffix_pos=%d", __FUNCTION__, suffix_pos);
#endif
	if (suffix_pos == -1)
		return NULL;
	return findFileType((const char *)newfn.left(suffix_pos));
}

/*
 * The Constructor
 */
KMimeMagic::KMimeMagic(const char * _configfile)
{
	int result;
	conf = (config_rec *)calloc(1, sizeof(config_rec));

	/* set up the magic list (empty) */
	conf->magic = conf->last = NULL;
	magicResult = NULL;
	followLinks = FALSE;

	if (_configfile)
		conf->magicfile = strdup(_configfile);
	/* on the first time through we read the magic file */
	result = apprentice();
	if (result == -1)
		return;
#if (MIME_MAGIC_DEBUG > 1)
	test_table();
#endif
}

/*
 * The destructor.
 * Free the magic-table and other resources.
 */
KMimeMagic::~KMimeMagic()
{
	if (conf) {
		struct magic *p = conf->magic;
		struct magic *q;
		while (p) {
			q = p;
			p = p->next;
			free(q);
		}
		free(conf);
	}
	if (magicResult)
		delete magicResult;
}

bool
KMimeMagic::mergeConfig(const char * _configfile)
{
	int result;

	if (conf) {
		char * old_magicfile = conf->magicfile;

		if (_configfile)
			conf->magicfile = strdup(_configfile);
		else
			return false;
		result = apprentice();
		if (result == -1) {
			conf->magicfile = old_magicfile;
			return false;
		}
#if (MIME_MAGIC_DEBUG > 1)
		test_table();
#endif
		return true;
	}
	return false;
}

bool
KMimeMagic::mergeBufConfig(char * _configbuf)
{
	int result;

	if (conf) {
		result = buff_apprentice(_configbuf);
		if (result == -1)
			return false;
#if (MIME_MAGIC_DEBUG > 1)
		test_table();
#endif
		return true;
	}
	return false;
}

void
KMimeMagic::setFollowLinks( bool _enable )
{
	followLinks = _enable;
}

KMimeMagicResult *
KMimeMagic::findBufferType(const char * buffer, int nbytes)
{
	unsigned char buf[HOWMANY + 1];	/* one extra for terminating '\0' */

	resultBuf.resize(0);
	if (magicResult) {
		magicResult->setContent(QString());
		magicResult->setEncoding(QString());
	} else
	  magicResult = new KMimeMagicResult();
	accuracy = 100;
	
        if (nbytes > HOWMANY)
                nbytes = HOWMANY;
        memcpy(buf, buffer, nbytes);
        if (nbytes == 0) {
                resultBuf += MIME_BINARY_ZEROSIZE;
        } else {
                buf[nbytes++] = '\0';   /* null-terminate it */
                tryit(buf, nbytes);
        }
        resultBuf += "\n";
        /* if we have any results, put them in the request structure */
        finishResult();
	magicResult->setAccuracy(accuracy);
        return magicResult;
}

static void
refineResult(KMimeMagicResult *r, const char * _filename)
{
	QString tmp = r->getContent();
	if (tmp.isEmpty())
		return;
	if ((strcmp(tmp, "text/x-c") == 0) ||
	    (strcmp(tmp, "text/x-c++") == 0)   )
	{
		if ( QString(_filename).right(2) == ".h" )
			tmp += "-hdr";
		else
			tmp += "-src";
		r->setContent(tmp);
	}
}

KMimeMagicResult *
KMimeMagic::findBufferFileType( const char * buffer, int nbytes,
				const char * fn)
{
	KMimeMagicResult * r = findBufferType( buffer, nbytes );
	refineResult(r, fn);
        return r;
}
 
/*
 * Find the content-type of the given file.
 */
KMimeMagicResult *
KMimeMagic::findFileType(const char *fn)
{
        resultBuf.resize(0);
        if (magicResult) {
                magicResult->setContent(QString());
                magicResult->setEncoding(QString());
        } else
                magicResult = new KMimeMagicResult();
	accuracy = 100;

        /* try excluding file-revision suffixes */
        if (!revision_suffix(fn)) {
                /* process it based on the file contents */
                process(fn);
        }
        /* if we have any results, put them in the request structure */
        finishResult();
	magicResult->setAccuracy(accuracy);
	refineResult(magicResult, fn);
        return magicResult;
}
