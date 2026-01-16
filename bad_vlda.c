/*
MIT License

Copyright (c) 2026 Dave Shepperd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

static int walkBuf(uint8_t *buf, size_t len)
{
	uint8_t *bp = buf;
	uint16_t recLen, pad;
	
	while ( bp < buf+len )
	{
		recLen = (bp[1] << 8) | bp[0];
		pad = recLen&1;
		if ( recLen < 1 || recLen >= 16384 )
			return 1;			/* failed */
		recLen += 2+pad;
		if (recLen > len )
			return 1;
		bp += recLen;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	struct stat st;
	int ii, sts, fd;
	size_t len;
	uint8_t *buf, *bufEnd, *rcd;
	uint16_t recLen;
	const char *fname = argv[1];
	
	sts = stat(fname,&st);
	if ( sts < 0 )
	{
		fprintf(stderr,"Failed to stat '%s': %s\n", fname, strerror(errno));
		return 1;
	}
	buf = (uint8_t *)malloc(st.st_size);
	if ( !buf )
	{
		fprintf(stderr,"Failed to malloc %ld bytes of memory\n", st.st_size);
		return 1;
	}
	fd = open(argv[1],O_RDONLY|O_BINARY);
	if ( fd < 0 )
	{
		fprintf(stderr,"Failed to open for input '%s': %s\n", fname, strerror(errno));
		return 1;
	}
	len = read(fd,buf,st.st_size);
	if ( len != st.st_size )
	{
		fprintf(stderr,"Failed to read %ld bytes from '%s'. Instead read %ld: %s\n",
				st.st_size, fname, len, strerror(errno));
		close(fd);
		free(buf);
		return 1;
	}
	close(fd);
	rcd = buf;
	bufEnd = buf+len;
	while ( rcd < bufEnd && len > 0 )
	{
		if ( !walkBuf(rcd,len) )
			break;
		rcd += 2;
		len -= 2;
	}
	if ( rcd < bufEnd )
		printf("Found potential starting record at offset %ld (0x%lX). Remaining file has %ld (0x%lX) bytes\n",
			   rcd-buf,
			   rcd-buf,
			   bufEnd-rcd,
			   bufEnd-rcd);
	else
		printf("No valid starting record found\n");
	free(buf);
	return 0;
}

