#include "naive_ais.h"
static const char RcsId[] = "$Id$";
/*
**    (c) xof, 2015
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
*/

rtlsdr_dev_t *devp;
int running = 1;
void onint(int signum)
	{
	running = 0;
	}
int rtl_init(int sps, int freq)
	{
	int numgains;
	int gains[100];

	signal(SIGINT, onint);
	if (rtlsdr_open(&devp, 0) < 0)
		{
		perror("Can't open rtlsdr device!");
		return(-1);
		}
	rtlsdr_set_tuner_gain_mode(devp, 1);		/* manual */

	numgains = rtlsdr_get_tuner_gains(devp, gains);
	rtlsdr_set_tuner_gain(devp, gains[numgains-1]); /* set max gain */
	fprintf(stderr, "Tuner gain = %d\n", rtlsdr_get_tuner_gain(devp));
	rtlsdr_set_freq_correction(devp, 0);		/* why not */
	rtlsdr_set_center_freq(devp, freq);
	rtlsdr_set_sample_rate(devp, sps);
	rtlsdr_reset_buffer(devp);

	return(0);
	}
#define BUFSIZE (256*1024)
int short_frames = 0; /* don't fill the log with tons of useless messages */
int fd_in = -1;
int refill(ushort *usp, int len)
	{
	int	rlen;

	if (fd_in >= 0)
		{
		if ((rlen = read(fd_in, usp, len)) < 0)
			{
			perror("refill(): raw IQ file read\n");
			return(-1);
			}
		}
	else	{
		if (rtlsdr_read_sync(devp, usp, len, &rlen) < 0)
			{
			perror("refill(): rtlsdr_read_sync");
			return(-1);
			}
		}
	return(rlen);
	}
int main(int argc, char *argv[])
	{
	ushort	buf[BUFSIZE];
	ushort s;
	int	len, i, cnt;
	int	sps, freq;
	int	mag, tot_mag;
	extern int sig_level;

	fprintf(stderr, "%s\n", RcsId);

	ais_opt(argc, argv);

	sps = 9600*SpS; /* 9600 bits/sec * Samples per Symbol */
	freq = 161975000; /* AIS Channel 'A' */
	if (sps > 2024000) /* 2Msps */
		{
		fprintf(stderr, "sps too high\nrtl_grab: usage is 'rtl_grab sps freq'\n");
		exit(-1);
		}
	fprintf(stderr, "naive_ais : %d samples/sec -- freq = %d Hz (<ctrl>-C to stop)\n", sps, freq);
	signal(SIGINT, onint);
	if (fd_in < 0) /* initialized in ais_opt() */
		{ /* use the dongle */
		if (rtl_init(sps, freq) < 0)
			exit(-1);
		}

	mk_maglut();
	mk_philut();
	mk_train_coef();

	for (len = 0, i = 0, cnt = 0, tot_mag = 0; running;)
		{
		if (i == len)
			{ /* refill */
			if ((len = refill(buf, sizeof(buf))) <= 0)
				{
				break;
				}
			else	{
				len = len/2; /* IQ samples */
				i = 0;
				}
			}
		while (i < len && (mag = maglut[(s=buf[i++])]) > SIG_THRESHOLD && cnt < sizeof(modbuf))
			{
			modbuf[cnt++] = s;  /* grab a frame */
			tot_mag += mag;
			}
		if (cnt == sizeof(modbuf))
			{
			fprintf(stderr, "Long frame!\n"); /* not possible.noise too high? */
			cnt = 0;  tot_mag = 0;
			}
		if (cnt > 0 && mag <= SIG_THRESHOLD)
			{ /* noise - end of signal */
			if (cnt > 4000)
				{ /* 168 bits = 4368 samples; there are many more bits in real frames */
				sig_level = tot_mag / cnt;
				demod(modbuf, cnt);
				}
			else	{
				if (short_frames++ < 50)
					fprintf(stderr, "Short frame!\n");
				}
			cnt = 0; tot_mag = 0; /* wait for a new frame */
			}
		}
	rtlsdr_close(devp);
	return(0);
	}

