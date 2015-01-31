#include "naive_ais.h"
static const char RcsId[] = "$Id$";

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

char	*tcp_host = "localhost";
int	tcp_s = -1;
int tcp_init(char *port_str)
	{
	struct addrinfo ai, *aip;
	int	ls;

	tcp_s = -1;
	bzero(&ai, sizeof(ai));
	ai.ai_family = AF_INET;
	ai.ai_socktype = SOCK_STREAM;
	ai.ai_protocol = IPPROTO_TCP;
	ai.ai_flags = AI_PASSIVE;

/* should use gethostbyname() ? */
	if (getaddrinfo(tcp_host, port_str, &ai, &aip) < 0)
		{
		perror("tcp_getaddrinfo()");
		return(-1);
		}
	if ((ls = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol)) < 0)
		{
		perror("tcp_socket(listen)");
		return(-1);
		}
	if (bind(ls, aip->ai_addr, (int)aip->ai_addrlen) < 0)
		{
		perror("tcp_bind()");
		return(-1);
		}	
	freeaddrinfo(aip);
	fprintf(stderr, "Waiting tcp connexion...\n");
	if (listen(ls, 1) < 0)
		{
		perror("tcp_listen()");
		return(-1);
		}
	if ((tcp_s = accept(ls, NULL, NULL)) < 0)
		{
		perror("tcp_accept()\n");
		return(-1);
		}
	fprintf(stderr, "tcp connected!\n");
	return(0);
	}
void tcp_send(char *cp, int len)
	{
	if (tcp_s > 0)
		{
		if (send(tcp_s, cp, len, 0) < 0)
			{
			perror("tcp_send()");
			}
		}
	}

