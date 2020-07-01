//Cliente UDP RDT

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
//#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>

#define PORT    8080
#define MAXLINE 512

using std::cout; using std::cin;
using std::string;




int main()
{
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in  servaddr;
    struct hostent *host;
    
    host = (struct hostent *)gethostbyname((char *)"127.0.0.1");

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr = *((struct in_addr *)host->h_addr);
    //servaddr.sin_addr.s_addr = INADDR_ANY;

    string msgToChat;
    int n;
    socklen_t len;
    
    for (;;)
    {
        cout << "\nType Something (q or Q to quit):";
        getline(cin, msgToChat);

        //if ((strcmp(msgToChat, "q") == 0) || strcmp(msgToChat, "Q") == 0)
        //break;

        sendto(sockfd, msgToChat.c_str() , msgToChat.length(),
                MSG_CONFIRM, (const struct sockaddr *) &servaddr,
                        sizeof(servaddr));

        cout << "Hello message sent.\n";

        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                                MSG_WAITALL, (struct sockaddr *) &servaddr,
                                &len);
        cout << "n" << n;
        buffer[n] = '\0';
        printf("Server : %s\n", buffer);

    }

    close(sockfd);
    return 0;
}