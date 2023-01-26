#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <fstream>
#include "string"
#define MYPORT "21472" // the port users will be connecting to
#define BACKLOG 10	   // how many pending connections queue will hold
#define MAXDATASIZE 100
#define MAXBUFLEN 1024
#define SERVERPORT "24472"

using namespace std;
bool username_received, password_received, username_correct;
int j = 0;

void *get_in_addr(struct sockaddr *sa) // taken from beej
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

string authentication(string usernm, string pass) //  function for file authentication •	https://www.youtube.com/watch?v=LiyMDucaq2k
{
	struct user
	{
		string username, password;
	};

	user obj[5];
	ifstream in("cred.txt");
	while (!in.eof())
	{
		for (int i = 0; i < 5; ++i)
		{
			in >> obj[i].username;
		}
	}

	bool islogin = false;
	for (int i = 0; i < 5; ++i)
	{
		// cout << pass << "   " << obj[i].username.substr(obj[i].username.find(",") + 1, obj[i].username.length() - 1) << endl;
		if (usernm == obj[i].username.substr(0, obj[i].username.find(",")))
		{
			// cout << "username is correct" << endl;
			username_correct = true;
			if (pass == obj[i].username.substr(obj[i].username.find(",") + 1, obj[i].username.length() - 1))
			{
				// cout << "password is correct" << endl;
				islogin = true;
				break;
			}
		}

		else
		{
			islogin = false;
		}
	}
	if (!islogin)
	{
		/* code */
		if (username_correct)
		{
			return "password doesnot match";
		}
		else
		{
			return "Username not found";
		}
	}
	else
	{

		return "authentication sucessful";
	}
}

string recv_udp() // function to recieve UDP taken from beej
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	memset(buf, 0, sizeof buf);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) // taken from beej
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		// return 1;
	}
	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("listener: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (p == NULL)
	{
		fprintf(stderr, "listener: failed to bind socket\n");
	}
	freeaddrinfo(servinfo);
	// printf("listener: waiting to recvfrom...\n");
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
							 (struct sockaddr *)&their_addr, &addr_len)) == -1)
	{
		perror("recvfrom");
		exit(1);
	}
	// printf("listener: got packet from %s\n",
	inet_ntop(their_addr.ss_family,
			  get_in_addr((struct sockaddr *)&their_addr),
			  s, sizeof s);
	// printf("listener: packet is %d bytes long\n", numbytes);

	// printf("listener: packet contains \"%s\"\n", buf);
	// cout << "listener: packet contains :" << buf << endl;
	if (j == 0)
	{
		username_received = true;
		j = 1;
	}
	else if (j == 1)
	{
		password_received = true;
		j = 0;
	}
	close(sockfd);
	return buf;
}
void send_udp(const char *datatosend) // function to send UDP taken from beej
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("localhost", SERVERPORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		// return 1;
	}
	// loop through all the results and make a socket
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("talker: socket");
			continue;
		}
		break;
	}
	if (p == NULL)
	{
		fprintf(stderr, "talker: failed to create socket\n");
		// return 2;
	}
	if ((numbytes = sendto(sockfd, datatosend, strlen(datatosend), 0,
						   p->ai_addr, p->ai_addrlen)) == -1)
	{
		perror("talker: sendto");
		exit(1);
	}
	freeaddrinfo(servinfo);

	// printf("talker: sent %d bytes to %s\n", numbytes, "localhost");
	close(sockfd);
}

int main()
{
	cout << "The ServerC is up and running using UDP on port 21472." << endl;
	string encrypt_username, encrypt_password;
	int i = 0;
	while (true)
	{

		if (i == 0)
		{
			encrypt_username = recv_udp();
			// cout << "The string is: (" << encrypt_username << ")" << endl;
			i++;
		}
		else
		{
			encrypt_password = recv_udp();
			// cout << "The string is: (" << encrypt_password << ")" << endl;
			i = 0;
		}
		if (password_received && username_received)
		{
			cout << "The ServerC received an authentication request from the Main Server." << endl;
			string a = authentication(encrypt_username, encrypt_password);
			// cout << a << endl;
			send_udp(a.c_str());
			password_received = false;
			username_received = false;
			cout << "The ServerC finished sending the response to the Main Server." << endl;
		}
	}

	return 0;
}