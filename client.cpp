#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define TCPPORT "25472" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once
// get sockaddr, IPv4 or IPv6:

char username[50];
char password[50];
char coursecode[100], query[100];

void *get_in_addr(struct sockaddr *sa) // taken from beej
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

string recv(int sockfd) // function to recieve UDP taken from beej
{
    char buf[MAXDATASIZE];
    int numbytes;
    memset(buf, 0, sizeof buf);
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
    {
        perror("recv");
        // exit(1);
    }
    return buf;
}

void sendtcp(int new_fd, const char *datatosend) // Function to send TCP taken from beej
{
    if (send(new_fd, datatosend, strlen(datatosend), 0) == -1)
        perror("send");
}

int main()
{
    cout << "The client is up and running." << endl;
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv, count = 0;
    char s[INET6_ADDRSTRLEN];

    while (true)
    {

        cout << "Please enter the username: ";
        bzero(username, sizeof(username));
        cin >> username;
        cout << "Please enter the password: ";
        bzero(password, sizeof(password));
        cin >> password;

        // int l = strlen(text);
        // char x;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((rv = getaddrinfo("Localhost", TCPPORT, &hints, &servinfo)) != 0)
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        // loop through all the results and connect to the first we can
        for (p = servinfo; p != NULL; p = p->ai_next)
        {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                                 p->ai_protocol)) == -1)
            {
                perror("client: socket");
                continue;
            }

            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
            {
                close(sockfd);
                perror("client: connect");
                continue;
            }

            break;
        }

        if (p == NULL)
        {
            fprintf(stderr, "client: failed to connect\n");
            return 2;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), // taken from beej
                  s, sizeof s);
        //  printf("client: connecting to %s\n", s);

        freeaddrinfo(servinfo); // all done with this structure

        sendtcp(sockfd, username);
        cout << username << " sent an authentication request to the main server." << endl;
        usleep(500);
        sendtcp(sockfd, password);
        string auth = recv(sockfd);

        if (auth == "authentication sucessful") // checking for authentication
        {
            cout << username << " received the result of authentication using TCP over port 25472. Authentication is successful." << endl;
            break;
        }
        if (auth == "Username not found")
        {
            cout << username << " received the result of authentication using TCP over port 25472. Authentication failed: Username Does not exist." << endl;
            count++;
            cout << "Attempts remaining:" << 3 - count << endl;
        }
        if (auth == "password doesnot match")
        {
            count++;

            cout << username << " received the result of authentication using TCP over port 25472.. Authentication failed: Password does not match." << endl;
            cout << "Attempts remaining:" << 3 - count << endl;
        }

        // cout << "count :" << count << endl;
        if (count == 3)
        {
            cout << "Authentication Failed for 3 attempts. Client will shut down." << endl;
            exit(0);
        }
    }
    while (true)
    {

        cout << "Please enter the course code to query:";
        bzero(coursecode, sizeof(coursecode));
        cin >> coursecode;
        cout << "Please enter the category (Credit / Professor / Days / CourseName:";
        bzero(query, sizeof(query));
        cin >> query;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((rv = getaddrinfo("Localhost", TCPPORT, &hints, &servinfo)) != 0) // taken from beej/
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        // loop through all the results and connect to the first we can
        for (p = servinfo; p != NULL; p = p->ai_next)
        {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                                 p->ai_protocol)) == -1)
            {
                perror("client: socket");
                continue;
            }

            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
            {
                close(sockfd);
                perror("client: connect");
                continue;
            }

            break;
        }

        if (p == NULL)
        {
            fprintf(stderr, "client: failed to connect\n");
            return 2;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
                  s, sizeof s);
        // printf("client: connecting to %s\n", s);

        freeaddrinfo(servinfo); // all done with this structure

        sendtcp(sockfd, coursecode);
        usleep(500);

        sendtcp(sockfd, query);
        cout << username << " sent a request to the main server." << endl;
        string info = recv(sockfd);
        cout << "The client received the response from the Main server using TCP over port 25472." << endl;
        if (info == "not found")
        {
            cout << "Didn’t find the course: " << coursecode << endl;
        }
        else
        {
            cout << "The " << query << " of " << coursecode << " is " << info << endl
                 << endl;
        }
        cout << "-----Start a new request----" << endl;
    }
    close(sockfd);
}