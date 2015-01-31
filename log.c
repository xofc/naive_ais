#include "naive_ais.h"
static const char RcsId[] = "$Id$";

int	log_fd = -1;
int	log_offset = 0;
int log_tell, log_tell_len;
ushort	log_min_buf[32];	/* gap between records */
int log_init(char *fname)
	{
	if ((log_fd = open(fname, O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR)) < 0)
		{
		perror(fname);
		return(-1);
		}
	memset(log_min_buf, 0x7f, sizeof(log_min_buf));
	return(0);
	}
void log_rec(uchar *ucp, int len)
	{
	if (log_fd < 0)
		return;
	log_tell = log_offset; log_tell_len = len;
	if (write(log_fd, ucp, len) != len || write(log_fd, log_min_buf, sizeof(log_min_buf)) != sizeof(log_min_buf))
		perror ("log_rec");
	log_offset += len + sizeof(log_min_buf);
	}
void log_close()
	{
	if (log_fd >= 0)
		{
		if (close(log_fd) < 0)
			perror("log_close");
		}
	}

