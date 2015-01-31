#include "naive_ais.h"
static const char RcsId[] = "$Id: demod.c,v 1.1 2015/01/22 13:11:08 xof Exp xof $";
/*
** 
*/
ushort maglut[256*256];
void mk_maglut()
	{
	int	i, q;

	for (i = 0; i <= 255; i++)
		{
		for (q = 0; q <= 255; q++)
			{
			int mag, mag_i, mag_q;

			mag_i = (i * 2) - 255;
			mag_q = (q * 2) - 255;

			mag = (int) round((sqrt((mag_i*mag_i)+(mag_q*mag_q)) * 258.433254) - 365.4798);

			maglut[(i*256)+q] = (ushort) ((mag < 65535) ? mag : 65535);
			}
		}
	}
short philut[256*256];
void mk_philut()
	{
	int	i, q, phi;

	for (i = 0; i <= 255; i++)
		{
		for (q = 0; q <= 255; q++)
			{
			float mag_i, mag_q;

			mag_i = (i * 2) - 255;
			mag_q = (q * 2) - 255;

			phi = atanf(mag_q/mag_i)*180.0/3.141592;
			if (mag_i > 0.0)
				{
				if (mag_q < 0)
					phi += 360;
				}
			else	{
				phi += 180;
				}
			philut[(i*256)+q] = (short) phi;
			}
		}
	}
/*
** transition detection
**   there are 26 samples/symbol
**   the training sequence is 52 high - 52 low - 52 high - ...
**	we sum high's and substract low's for various shifts
**		the maximum gives the best transition
*/
int train_coef_tab[SpS*20];
void mk_train_coef()
	{
	int	i, j;
	for (i = 0; i < 10; i++)
		{
		for (j = 0; j < SpS; j++)
			train_coef_tab[2*i*SpS+j] = 1;
		for (j = SpS; j < (2 * SpS); j++)
			train_coef_tab[2*i*SpS+j] = -1;
		}
	}
#define nelem(tab) (sizeof(tab)/sizeof(tab[0]))
int merge(ushort *sp)
	{
	int	sum = 0;
	int	i;

	for (i = 0; i < nelem(train_coef_tab); i++)
		sum += train_coef_tab[i] * sp[i];
	return(sum);
	}
#define RAMPUP 10
int symbol_sync(ushort *sp)
	{
	int	i, mx_index;
	int	mx, x;

	mx_index = 0;
	mx = 0;
	for (i = RAMPUP; i < (RAMPUP+2*SpS); i++)
		{
		x = merge(sp+i);
		if (x > mx)
			{
			mx_index = i;
			mx = x;
			}
		}
	return(mx_index % (2*SpS));
	}
/*----------------------*/
void dumpi16(ushort *sp, int len)
	{
	while (--len > 0)
		printf("%d\n", *sp++);
	printf("0\n0\n0\n0\n");
	}

ushort buf[256*1024];
ushort buft[256*1024];
ushort modbuf[16*1024]; /* 512 bits (440+) -> 512*26=13312 samples */
#define S10(p) (p[0]+p[1]+p[2]+p[3]+p[4]+p[5]+p[6]+p[7]+p[8]+p[9])


void analog2bits(ushort *sp, int mean10, int len, char canal)
	{
	int	ti;
	int	left, right;
	int	bit;
	uchar byte;
	int	byte_count, bit_count, bit_stuff;
	uchar hdlc_buf[256]; /* ususally 32 bytes but... */

/* mean10 = 700; /**/
	byte_count = 0;
	bit_stuff = 0;
	for (ti = SpS; ti < len; ti += SpS)
		{
		left = S10((sp+ti-18));
		right = S10((sp+ti+8));
		if ((left < mean10 && right < mean10) || (left > mean10 && right > mean10))
			{
			bit = 0x80; /* no transition -> '1' ; LSB first */
			bit_stuff += 1;
			}
		else	{
			bit = 0x00; /* transition -> '0' */
			if (bit_stuff == 5)
				{ /* a zero is inserted */
				bit_stuff = 0;
				continue; /* ignore this bit */
				}
			bit_stuff = 0;
			}
		byte = (byte >> 1) + bit; bit_count += 1; /* LSb first */
		if (byte == 0x7E)
			{
			if (byte_count == 0)
				{ /* start of HDLC frame */
				hdlc_buf[byte_count++] = byte;  /* 0x7E */
				bit_count = 0;
				}
			else	{ /* end of HDLC frame */
				hdlc_buf[byte_count++] = byte;  /* 0x7E */
				check_hdlc(hdlc_buf, byte_count, canal);
				break;
				}
			}
		if (byte_count)
			{
			if (bit_count == 8)
				{
				bit_count = 0;
				hdlc_buf[byte_count++] = byte;
				byte = 0;
				}
			}
		}
	}
void demod(ushort *sp, int len)
	{
	static int msg = 0;
	int	i;
	int	phi, phi0;
	int	t;
	int	t0;
	int	mean;

	log_rec((uchar *)sp, 2 * len);
	/*
	**  IQ2DPhi
	*/
	for (i = 1; i < len; i += 1)
		{
		phi = (360 + philut[sp[i]] - phi0) % 360;
		if (phi > 180)
			phi = 360 - phi;
		sp[i-1] = phi;
		phi0 = philut[sp[i]];
		}
	/*
	** buf[i] = delta(phi)
	*/
	t0 = symbol_sync(sp);
	for (mean = 0, i = RAMPUP; i < (RAMPUP+5*SpS); i++)
		mean += sp[i];
	mean = (mean) / (SpS/2);	/* 10 samples */
	if (mean < 500)  /* Canal B? */
		analog2bits(sp + t0, mean, len - t0, 'B');
	else	analog2bits(sp + t0, mean, len - t0, 'A');
	}

