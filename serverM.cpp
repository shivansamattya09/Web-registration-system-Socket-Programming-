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
#define TCPPORT "25472" // the port users will be connecting to
#define UDPPORT "24472"
#define BACKLOG 10 // how many pending connections queue will hold
#define MAXDATASIZE 100
#define MAXBUFLEN 1024
#define CPORT "21472"
#define CSPORT "22472"
#define EEPORT "23472"

using namespace std;

// char text[50];
char x;
int l;
string username;

string encrypt(const char *data) // encryption code  * https://www.tutorialspoint.com/cplusplus-program-to-implement-caesar-cypher
{
    char *text = (char *)data;
    for (int i = 0; text[i] != '\0'; ++i)
    {
        x = text[i];
        // encrypt for lowercase letter
        if (x >= 'a' && x <= 'z')
        {
            x = x + 4;
            if (x > 'z')
            {
                x = x - 'z' + 'a' - 1;
            }
            text[i] = x;
        }
        // encrypt for uppercase letter
        else if (x >= 'A' && x <= 'Z')
        {
            x = x + 4;
            if (x > 'Z')
            {
                x = x - 'Z' + 'A' - 1;
            }
            text[i] = x;
        }
        if (x >= '0' && x <= '9')
        {
            x = x + 4;
            if (x > '9')
            {
                x = x - '9' + '0' - 1;
            }
        }
        text[i] = x;
    }

    // cout << "Encrypted message: " << text << endl;
    return text;
}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void sendtcp(int new_fd, const char *datatosend) // sending via TCP taken from beej
{
    // if (!fork())

    // { // this is the child processes
    // close(sockfd); // child doesn't need the listener
    if (send(new_fd, datatosend, strlen(datatosend), 0) == -1)
        perror("send");
    // close(new_fd);
    // }
    // recv(new_fd);
    // username and password
}
string recv(int sockfd)
{
    char buf[MAXDATASIZE];
    int numbytes;
    memset(buf, 0, sizeof buf);
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
    {
        perror("recv");
        // exit(1);
    }
    // cout << "client: received : " << buf << endl;
    return buf;
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

    if ((rv = getaddrinfo(NULL, UDPPORT, &hints, &servinfo)) != 0)
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
              get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    // printf("listener: packet is %d bytes long\n", numbytes);

    // printf("listener: packet contains \"%s\"\n", buf);
    // cout << "listener: packet contains :" << buf << endl;
    close(sockfd);
    return buf;
}
void send_udptoC(const char *datatosend) // function to send UDP taken from beej
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo("localhost", CPORT, &hints, &servinfo)) != 0)
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
        // exit(1);
    }
    freeaddrinfo(servinfo);

    // printf("talker: sent %d bytes to %s\n", numbytes, "localhost");
    close(sockfd);
}
void send_udptoCS(const char *datatosend) // function to send UDP taken from beej
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo("localhost", CSPORT, &hints, &servinfo)) != 0)
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
        // exit(1);
    }
    freeaddrinfo(servinfo);

    // printf("talker: sent %d bytes to %s\n", numbytes, "localhost");
    close(sockfd);
}
void send_udptoEE(const char *datatosend) // function to send via UDP taken from beej
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo("localhost", EEPORT, &hints, &servinfo)) != 0)
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
        // exit(1);
    }
    freeaddrinfo(servinfo);

    //    printf("talker: sent %d bytes to %s\n", numbytes, "localhost");
    close(sockfd);
}

int main()
{
    cout << "The main server is up and running." << endl;
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, TCPPORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("setsockopt");
            // exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        // exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        // exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        // exit(1);
    }

    // printf("server: waiting for connections...\n");
    while (1)
    { // main accept() loop
        cout << "in" << endl;
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

        // printf("server: got connection from %s\n", s);

        username = recv(new_fd);

        string password = recv(new_fd);
        cout << "The main server received the authentication for " << username << " using TCP overport 25472. " << endl;

        username = encrypt(username.c_str());
        password = encrypt(password.c_str());

        send_udptoC(username.c_str()); // sending function via UDO
        usleep(1000);                  // Suspend execution for an interval
        send_udptoC(password.c_str()); // sending function via UDO
        cout << "The main server sent an authentication request to serverC." << endl;
        string a = recv_udp();
        cout << "The main server received the result of the authentication request from ServerC using UDP over port 24472." << endl;
        cout << "The main server sent the authentication result to the client." << endl;
        sendtcp(new_fd, a.c_str()); // send via TCP

        close(new_fd); // parent doesn't need this

        // return 0;
        if (a == "authentication sucessful")
        {
            break;
        }
    }
    while (1)
    {
        cout << "second" << endl;
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

        // printf("server: got connection from %s\n", s);

        string coursecode = recv(new_fd);

        string query = recv(new_fd);
        cout << "The main server received from " << username << " to query course " << coursecode << " about " << query << " using TCP over port 25472." << endl;
        if (coursecode.c_str()[0] == 'C' && coursecode.c_str()[1] == 'S')
        {
            send_udptoCS(coursecode.c_str()); // sending function via UDO
            usleep(1000);                     // Suspend execution for an interval
            send_udptoCS(query.c_str());      // sending function via UDO
            cout << "The main server sent a request to serverCS." << endl;
            string a = recv_udp();
            cout << "The main server received the response from serverCS using UDP over port 24472. " << endl;
            printf("The main server sent the query information to the client.\n");
            sendtcp(new_fd, a.c_str()); // sending via TCP
        }
        else if (coursecode.c_str()[0] == 'E' && coursecode.c_str()[1] == 'E')
        {
            send_udptoEE(coursecode.c_str()); // sending function via UDO
            usleep(1000);                     // Suspend execution for an interval
            send_udptoEE(query.c_str());      // sending function via UDO
            cout << "The main server sent a request to serverEE." << endl;
            string a = recv_udp();
            cout << "The main server received the response from serverEE using UDP over port 24472. " << endl;
            printf("The main server sent the query information to the client.\n");
            sendtcp(new_fd, a.c_str()); // sending via TCP
        }

        close(new_fd);
    }
}