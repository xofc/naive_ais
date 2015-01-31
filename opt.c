#include "naive_ais.h"
static const char RcsId[] = "$Id$";

int	opt_aivdm_decode = 0;	/* do not decode aivdm */
int	opt_timestamp = 0;	/* no time-stamp */
int	opt_verbose = 0;	/* not verbose */

void usage(char *prog_name)
	{
	fprintf(stderr, "%s: usage is '%s [-i filename] [-o filename] [-t tcp_port]\n", prog_name, prog_name);
	fprintf(stderr, "\t -d          : decode AIVDM;\n");
	fprintf(stderr, "\t -t          : print timestamp;\n");
	fprintf(stderr, "\t -v          : verbose;\n");
	fprintf(stderr, "\t -i filename : use raw I/Q file 'filename' as input;\n");
	fprintf(stderr, "\t -o filename : write raw I/Q file 'filename';\n");
	fprintf(stderr, "\t -p tcp-port : send AIVDM data to tcp:tcp-port.\n");
	fprintf(stderr, "\t (-f frequency : used by the rtl tuner (default is 161975000 - AIS channel A).\n");
	fprintf(stderr, "\t (-l level : minimal signal level (default is 6000).\n");
	fprintf(stderr, "\t (-s samples : number of samples per symbol (default is 26).\n");
	}
ais_opt(int argc, char **argv)
	{
	int	opt;
	extern char *optarg;
	int	sps, freq;
	extern int fd_in;

	/*
	** default options
	*/
	sps = 9600*SpS;		/* 9600 bits/sec * Samples per Symbol */
	freq = 161975000;	/* AIS Channel 'A' */
	fd_in = -1;		/* no input file */
	opt_aivdm_decode = 0;	/* do not decode aivdm */
	opt_timestamp = 0;	/* no time-stamp */
	opt_verbose = 0;	/* not verbose */
	/*
	** options
	*/
	while ((opt = getopt(argc, argv, "dtvi:o:p:")) != -1)
		{
		switch (opt)
			{
			case 'd':
				opt_aivdm_decode = 1;
				break;

			case 't':
				opt_timestamp = 1;
				break;

			case 'v':
				opt_verbose = 1;
				break;

			case 'i':
				if ((fd_in = open(optarg, S_IREAD)) < 0)
					{
					perror(optarg);
					exit(-1);
					}
				else	fprintf(stderr, "%s: using %s as input file.\n", argv[0], optarg);
				break;

			case 'o':
				if (log_init(optarg) < 0)
					fprintf(stderr, "log_init(%s) failed.  Continuing without loging...\n", optarg);
				break;

			case 'p':
				if (tcp_init(optarg) < 0)
					fprintf(stderr, "tcp_init(localhost, %s) failed.  Continuing without tcp...\n", optarg);
				else	fprintf(stderr, "sending TCP data to localhost:%s.\n", optarg);
				break;

			default: /* '?' */
				fprintf(stderr, "%s : unknown '%c' option.\n", argv[0], opt);
				usage(argv[0]);
				exit(-1);
               		}
		}
	}
