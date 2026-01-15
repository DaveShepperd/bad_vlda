/*
        badvlda.c - Program to look through files to see if there is a record
	structure within.

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

static int walkBuf(uint8_t *buf, size_t len)
{
	uint8_t *bp = buf;
	uint16_t recLen, pad;
	
	while ( bp < buf+len )
	{
		recLen = (bp[1] << 8) | bp[0];
		pad = recLen&1;
		if ( recLen < 3 || recLen >= 16384 )
			return 1;			/* failed */
		bp = bp + 2 + recLen + pad;
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
	fd = open(argv[1],O_RDONLY);
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

