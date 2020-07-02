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

#include "RDT.h"

#define PORT    8080
#define MAXLINE 512

using std::cout; using std::cin;
using std::string;

unsigned int sec_udp_in, sec_udp_out; //Numero de secuencia UDP entrada/salida
unsigned int flujo_in, flujo_out;  //Numero de flujo entrada/salida

RDT Rdt;

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
        //Enviar nombre de archivo a transmitir

        sendto(sockfd, msgToChat.c_str() , msgToChat.length(),
                MSG_CONFIRM, (const struct sockaddr *) &servaddr,
                        sizeof(servaddr));

        cout << "Hello message sent.\n";

        //Recibir el total de paquetes a recibir
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                                MSG_WAITALL, (struct sockaddr *) &servaddr,
                                &len);
        cout << "n" << n;
        int sec_fin;
        //Creamos Flujo con numero de paquetes
        //TODO Modificar sec_inicial
        //TODO crearlo como puntero
        Flujo Archivo(0, sec_fin, 0);

        //Mientras no recibamos la totalidad de paquetes
        while ( ! Archivo.IsCompleto())
        {
            n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                                MSG_WAITALL, (struct sockaddr *) &servaddr,
                                &len);

            Package* paquete = new Package(string(buffer));
            if ( ! Archivo.InsertarPackage(paquete))
                //Si no fue insertado -> Destruir paquete
                delete paquete;            
        }
        //RecuperarMensaje
        string mensaje = Archivo.ExtraerMensaje();
        //TODO Convertir Mensaje a Archivo
        
        //buffer[n] = '\0';
        //printf("Server : %s\n", buffer);

    }

    close(sockfd);
    return 0;
}