#include "naive_ais.h"
#include <time.h>
static const char RcsId[] = "$Id$";

extern int opt_aivdm_decode;
extern int opt_timestamp;
extern int opt_verbose;
int	sig_level;

void pr_timestamp()
	{
	struct timeval tv;
	struct tm *tmp;
	time_t t;

	if (gettimeofday(&tv, NULL) < 0)
		{
		perror("No time!\n");
		return;
		}
	t = tv.tv_sec;
	tmp = localtime(&t);
	/* "2014-10-13 06:00:01......." */
	fprintf(stdout, "%04d-%02d-%02d %02d:%02d:%02d.%06d ", 1900+tmp->tm_year, tmp->tm_mon+1, tmp->tm_mday+1, tmp->tm_hour,
			tmp->tm_min, tmp->tm_sec, tv.tv_usec);
	}
void pr_verbose()
	{
	char	wherestr[80];
	extern int log_tell, log_tell_len;

	sprintf(wherestr, "%d@%d ", log_tell_len, log_tell);
	fprintf(stdout, "%-13s ", wherestr);
	fprintf(stdout, "%5d ", sig_level);
	}
char S6(int i)
	{
	return((i>=40)?i+56:i+48);
	}
void aivdm_chksum(char *cp)
	{
	uchar	chksum = 0;
	int	nibble;

	cp += 1; /* skip '!' */
	while (*cp)
		chksum ^= *cp++;
	*cp++ = '*';
	nibble = (chksum & 0xFF) >> 4;
	*cp++ = (nibble < 10) ? '0'+nibble : 'A'+nibble-10;
	nibble = chksum & 0x0F;
	*cp++ = (nibble < 10) ? '0'+nibble : 'A'+nibble-10;
	*cp++ = '\n';
	*cp = '\0';
	}
void aivdm_dump(uchar *ucp, int len, char canal)
	{
	int	i;
	int	x;
	uchar	bufin[3];
	char	aivdm_string[128], *cp;

	sprintf(aivdm_string, "!AIVDM,1,1,,%c,", canal);
	cp = aivdm_string + strlen(aivdm_string);
	for (i = 0; i < len; i += 3)
		{
		bufin[0] = ucp[i]; bufin[1] = ucp[i+1]; bufin[2] = ucp[i+2];
		x = bufin[0] >> 2;
		*cp++ = S6(x);
		x = ((bufin[0]&3)<<4)+(bufin[1]>>4);
		*cp++ = S6(x);
		x = ((bufin[1] & 0x0F)<<2)+(bufin[2]>>6);
		*cp++ = S6(x);
		x = bufin[2] & 0x3F;
		*cp++ = S6(x);
		}
	*cp++ = ','; *cp++ = '0'; *cp = '\0';
	aivdm_chksum(aivdm_string); /* add 4 chars */
	if (opt_timestamp)
		pr_timestamp();
	if (opt_verbose)
		pr_verbose();
	if (opt_aivdm_decode)
		aivdm_decode(aivdm_string);
	else	fprintf(stdout, "%s", aivdm_string);
	fflush(stdout);
	tcp_send(aivdm_string, cp + 4 - aivdm_string);
	}
/*
** aivdm decoding
*/
uchar U6(uchar c)	/* payload armoring */
	{
	c -= 48;
	if (c > 40)
		c -= 8;
	return(c);
	}
#define S6(c) ("@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^- !\"#$%&'()*+,-./0123456789:;<=>?"[c]) /* sixbit text */
int aivdm_bit(uchar *ucp, int bit)
	{
	uchar uc0, uc;

	uc0 = ucp[bit/6];
	uc = U6(uc0);
	bit = bit%6;
	return((uc >> (5-bit)) & 1);
	}
char *clean_str(char *cp) /* @ = end of string */
	{
	char *icp = cp;

	while (*icp && *icp != '@')
		icp++;
	*icp = '\0';
	return(cp);
	}
char *aivdm_str(uchar *ucp, int bit, int len) /* extract string; len in char */
	{
	static char buf[256];
	int i;
	uchar c;
	char *str;

	str = buf;
	while (--len >= 0)
		{
		c = 0;
		for (i = 0; i < 6; i++)
			{
			c = (c << 1) | aivdm_bit(ucp, bit++);
			}
		*str++ = S6(c);
		}
	*str = '\0';
	return(buf);
	}
int aivdm_int(uchar *ucp, int bit, int len) /* extract integer; len in bits */
	{
	int i, x, sign;
	uchar c;

	x = 0;
	if (aivdm_bit(ucp, bit))
		sign = -1;
	else	sign = 1;
	while (--len >= 0)
		{
		x = (x << 1) | aivdm_bit(ucp, bit++);
		}
	return(x);  /* sign? */
	}
float aivdm_lon(char *ucp) /* longitude in type 1,2,3 msgs */
	{
	float lon;

	lon = (float) aivdm_int(ucp, 61, 28);
	return(lon/600000.0);
	}
float aivdm_lat(char *ucp) /* latitude in type 1,2,3 msgs */
	{
	float lat;

	lat = (float) aivdm_int(ucp, 89, 27);
	return(lat/600000.0);
	}
int aivdm_decode(char *str)
	{
	char *cp;
	int  comma_cnt;

	/* !AIVDM,1,1,,B,177KQJ5000G?tO`K>RA1wUbN0TKH,0*5C */
	if (strncmp(str, "!AIVDM", 5))
		{
		fprintf(stdout, "hoho %d - %s\n", strncmp(str, "!AIVDM", 5), str);
		return(-1);
		}
	for (cp = str, comma_cnt = 0; comma_cnt < 5; ) /* -> payload */
		{
		if (*cp++ == ',')
			comma_cnt++;
		}
	fprintf(stdout, "mmsi:%08d  ", aivdm_int(cp, 8, 30));
	switch (*cp)
		{
		case '1':
		case '2':
		case '3':
			fprintf(stdout, "lat:%.4f - lon:%.4f\n", aivdm_lat(cp),  aivdm_lon(cp));
			break;

		case '5':
			fprintf(stdout, "name: %s\n", clean_str(aivdm_str(cp, 112, 20)));
			break;

		case '8':
			fprintf(stdout, "dac:%d  fid:%d\n", aivdm_int(cp, 40, 10), aivdm_int(cp, 50, 6));
			break;

		default:
			fprintf(stdout, "Untreated type '%c' : %s\n", *cp, str);
			break;
		}
	return(0);
	}
