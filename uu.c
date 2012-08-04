/* phucked to fit into mega-happy-sprite SigFLUP */
/*	$OpenBSD: uudecode.c,v 1.14 2004/04/09 22:54:02 millert Exp $	*/
/*	$FreeBSD: uudecode.c,v 1.49 2003/05/03 19:44:46 obrien Exp $	*/

/*-
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


/*
 * Create the specified file, decoding as you go.
 * Used with uuencode.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *infile, *outfile;
static FILE *infp;
static int base64, rflag;

static int	decode(char *name);
static int	decode2(char *name);
static int	uu_decode(void);

int already_found;

unsigned char buffer[0xffff];
int buffer_pos;

void encode(void);

FILE *output;
int mode;

int in_len;
int in_pos;
char *in_buf;
char *in_name;

int out_char(char in) {
 if(buffer_pos>0xffff)
  return 0;
 buffer[buffer_pos++] = in;
 return 0;
}

int uudec(char *filename, char *name) {
 base64 = 0;
 buffer_pos = 0;
 infp= fopen(filename, "r");
 decode(name);
 if(already_found == 0) 
  return -1;
 else
  return 1;
}

static int
decode(char *name)
{
	int r, v;
	already_found = 0;
	v = decode2(name);
	if (v == EOF) 
	 return 0;
	for (r = v;already_found!=1; r |= v) {
		v = decode2(name);
		if (v == EOF)
			break;
	}
	return (r);
}

static int
decode2(char *name)
{
	int mode;
	size_t n;
	char *p, *q;
	char buf[MAXPATHLEN];

	base64 = 0;
	/* search for header line */
	for (;;) {
		if (fgets(buf, sizeof(buf), infp) == NULL)
			return (EOF);
		p = buf;
		if (strncmp(p, "begin-base64 ", 13) == 0) {
			base64 = 1;
			p += 13;
		} else if (strncmp(p, "begin ", 6) == 0)
			p += 6;
		else
			continue;
		/* p points to mode */
		q = strchr(p, ' ');
		if (q == NULL)
			continue;
		*q++ = '\0';
		/* q points to filename */
		n = strlen(q);
		while (n > 0 && (q[n-1] == '\n' || q[n-1] == '\r'))
			q[--n] = '\0';
		/* found valid header? */
		if (n > 0)
			break;
	}

	if(strcmp(name,q)!=0) 
	 return 0;
	else
	 already_found = 1;


	mode = 664;

	outfile = q;

	outfile = name;

	return (uu_decode());
}

static int
getline2(char *buf, size_t size)
{
	if (fgets(buf, size, infp) != NULL)
		return (2);
	if (rflag)
		return (0);
	warnx("%s: %s: short file", infile, outfile);
	return (1);
}

static int
uu_decode(void)
{
	int i, ch;
	char *p;
	char buf[MAXPATHLEN];

	/* for each input line */
	for (;;) {
		switch (getline2(buf, sizeof(buf))) {
		case 0:
			return (0);
		case 1:
			return (1);
		}

#define	DEC(c)	(((c) - ' ') & 077)		/* single character decode */
#define IS_DEC(c) ( (((c) - ' ') >= 0) && (((c) - ' ') <= 077 + 1) )

#define OUT_OF_RANGE do {						\
	warnx("%s: %s: character out of range: [%d-%d]",		\
	    infile, outfile, 1 + ' ', 077 + ' ' + 1);			\
	return (1);							\
} while (0)

		/*
		 * `i' is used to avoid writing out all the characters
		 * at the end of the file.
		 */
		p = buf;
		if ((i = DEC(*p)) <= 0)
			break;
		for (++p; i > 0; p += 4, i -= 3)
			if (i >= 3) {
				if (!(IS_DEC(*p) && IS_DEC(*(p + 1)) &&
				     IS_DEC(*(p + 2)) && IS_DEC(*(p + 3))))
					OUT_OF_RANGE;

				ch = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
				out_char(ch);
				ch = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
				out_char(ch);
				ch = DEC(p[2]) << 6 | DEC(p[3]);
				out_char(ch);
			}
			else {
				if (i >= 1) {
					if (!(IS_DEC(*p) && IS_DEC(*(p + 1))))
						OUT_OF_RANGE;
					ch = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
					out_char(ch);
				}
				if (i >= 2) {
					if (!(IS_DEC(*(p + 1)) &&
					    IS_DEC(*(p + 2))))
						OUT_OF_RANGE;

					ch = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
					out_char(ch);
				
				}
				if (i >= 3) {
					if (!(IS_DEC(*(p + 2)) &&
					    IS_DEC(*(p + 3))))
						OUT_OF_RANGE;
					ch = DEC(p[2]) << 6 | DEC(p[3]);
					out_char(ch);
				}
			}
	}
	switch (getline2(buf, sizeof(buf))) {
	case 0:
		return (0);
	case 1:
		return (1);
	}
 return 0;
}



int in_read(char *out_buf, int len) {
 int i;
 if(in_pos>in_len)
  return -1;
 for(i=0;i<len;i++) {
  if(in_pos>in_len) 
   return i-1; 
  out_buf[i] = in_buf[in_pos++];
 }
 return i;
}

int uuenc(FILE *fp, char *name, unsigned char *buf, int len)
{
	int base64;

	base64 = 0;
	output = fp;
	mode = 644;
	in_name = name;
	in_pos = 0;
	in_len = len;
	in_buf = (char *)buf;
	encode();	
	return 0;
}
/* ENC is the basic 1 character encoding function to make a char printing */
#define	ENC(c) ((c) ? ((c) & 077) + ' ': '`')

/*
 * Copy from in to out, encoding as you go along.
 */
void
encode(void)
{
	int ch, n;
	char *p;
	char buf[80];

	(void)fprintf(output, "begin %d %s\n", mode, in_name);
	while ((n = in_read(buf, 45))!=-1) {
		ch = ENC(n);
		if (fputc(ch, output) == EOF)
			break;
		for (p = buf; n > 0; n -= 3, p += 3) {
			/* Pad with nulls if not a multiple of 3. */
			if (n < 3) {
				p[2] = '\0';
				if (n < 2)
					p[1] = '\0';
			}
			ch = *p >> 2;
			ch = ENC(ch);
			if (fputc(ch, output) == EOF)
				break;
			ch = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
			ch = ENC(ch);
			if (fputc(ch, output) == EOF)
				break;
			ch = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
			ch = ENC(ch);
			if (fputc(ch, output) == EOF)
				break;
			ch = p[2] & 077;
			ch = ENC(ch);
			if (fputc(ch, output) == EOF)
				break;
		}
		if (fputc('\n', output) == EOF)
			break;
	}
	if (ferror(stdin)) {
	  printf("uu encode read error\n");
	  exit(-1);
	}	 
	(void)fprintf(output, "%c\nend\n", ENC('\0'));
}

