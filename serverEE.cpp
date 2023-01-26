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
#define MYPORT "23472" // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold
#define MAXDATASIZE 100
#define MAXBUFLEN 1024
#define SERVERPORT "24472"

using namespace std;
bool courseCode_received, query_received;
int j = 0;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
string comp(string coursecode, string query) ////•	https://www.youtube.com/watch?v=LiyMDucaq2k
{
    struct cs
    {
        string code, credit, professor, days, coursename;
    };
    cs obj[5];
    ifstream in("ee.txt");
    int i = 0;
    while (in)
    {
        getline(in, obj[i].code, ',');
        getline(in, obj[i].credit, ',');
        getline(in, obj[i].professor, ',');
        getline(in, obj[i].days, ',');
        getline(in, obj[i].coursename);
        i++;
    }
    bool islogin = false;
    for (int i = 0; i < 5; i++)
    {
        if (coursecode == obj[i].code) // checking for condtions
        {
            if (query == "Credit")
            {
                cout << "The course information has been found: The " << query << " of " << coursecode << " is " << obj[i].credit << endl;
                return obj[i].credit;
                break;
            }
            if (query == "Professor")
            {
                cout << "The course information has been found: The " << query << " of " << coursecode << " is " << obj[i].professor << endl;
                return obj[i].professor;
                break;
            }
            if (query == "Days")
            {
                cout << "The course information has been found: The " << query << " of " << coursecode << " is " << obj[i].days << endl;
                return obj[i].days;
                break;
            }
            if (query == "CourseName")
            {
                cout << "The course information has been found: The " << query << " of " << coursecode << " is " << obj[i].coursename << endl;
                return obj[i].coursename;
                break;
            }
        }
    }
    cout << "Didn’t find the course: " << coursecode << endl;
    return "not found";
}
string recv_udp() // Function to recieve UDP taken from beej
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

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0)
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
        courseCode_received = true;
        j = 1;
    }
    else if (j == 1)
    {
        query_received = true;
        j = 0;
    }
    close(sockfd);
    return buf;
}
void send_udp(const char *datatosend) // Function to Send UDP taken from beej
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
    cout << "The ServerEE is up and running using UDP on port 23472." << endl;
    string courseCode, query;
    int i = 0;
    while (true)
    {
        if (i == 0)
        {
            courseCode = recv_udp();
            // cout << "The string is: (" << courseCode << ")" << endl;
            i++;
        }
        else
        {
            query = recv_udp();
            // cout << "The string is: (" << query << ")" << endl;
            i = 0;
        }
        if (courseCode_received && query_received)
        {
            cout << "The ServerEE received a request from the Main Server about the " << query << " of " << courseCode << endl;
            string a = comp(courseCode, query);
            // cout << a << endl;
            send_udp(a.c_str());
            courseCode_received = false;
            query_received = false;
            cout << "The ServerEE finished sending the response to the Main Server." << endl;
        }
    }

    return 0;
}