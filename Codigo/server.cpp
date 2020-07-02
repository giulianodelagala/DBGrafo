//ss.cpp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//#include <netdb.h>
//#include <errno.h>
//#include <ctime>

#include <iostream>

#include "RDT.h"

#define PORT     8080
#define MAXLINE 512


using std::cout; using std::cin;
using std::string;

unsigned int secuencia_udp = 0; //Numero de secuencia UDP
unsigned int flujo = 0;

RDT Rdt;

void EnviarPaquete(string cadena, int sockfd,
     sockaddr_in* cliaddr, unsigned int len);

void EnvioArchivo(string file_name, int sockfd,
     sockaddr_in* cliaddr, unsigned int len)
{
    int secuencia_ini = secuencia_udp;
    string cadena_envio = Rdt.File2String(file_name);
    std::vector<string> vec_envio = Rdt.PreparacionMensaje(cadena_envio,
            secuencia_udp, flujo); //TODO Retirar Vector usar solo map
    int secuencia_fin = secuencia_udp; //Número actual luego de creacion datagram

    for (int sec= secuencia_ini; sec < secuencia_fin; ++sec)
    {
        EnviarPaquete(Rdt.map_secuencias[sec], sockfd,
        cliaddr, len);
    }
}

void EnviarPaquete(string cadena, int sockfd,
     sockaddr_in* cliaddr, unsigned int len)
{
    sendto(sockfd, cadena.c_str(), cadena.length(),
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
             len);
        cout << "\nHello message sent.\n";
}

int main()
{
    struct sockaddr_in servaddr, cliaddr; 
    int sockfd; 
	char buffer[MAXLINE]; 
	unsigned int len;
    int n;
    string msgToChat, comando;

    std::map<string, int> com = {
        {"AR", 1} //Envio de Archivo
    };
    
    

    std::vector<string> vec_cadena;

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    } 
	
    memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
    servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(PORT); 
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    len = sizeof(cliaddr);
    //vec_cadena = Rdt::PreparacionMensaje("MensajeRarisimoHola", secuencia_udp, flujo); //Prueba
    //cout << vec_cadena[0]; //Prueba
    string file_cadena = Rdt.File2String("lena.jpg");
    cout << file_cadena;

    for(;;)
    {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                                &len);
        //buffer[n] = '\0';
        //cout << "\nClient : %s\n" << buffer;
        //cout << "add" << cliaddr.sin_addr.s_addr;
        string cadena(buffer);
        comando = cadena.substr(0,2);

        switch (com[comando])
        {
        case 1: //AR
        {
            int size_name = stoi(cadena.substr(2,2));
            string file_name = cadena.substr(5,size_name);
            EnvioArchivo(file_name, sockfd, &cliaddr, len);
            break;
        }
            
        
        default:
            break;
        }

        //cout << "\nType Something (q or Q to quit):";
        //getline(cin, msgToChat);
        //sendto(sockfd, msgToChat.c_str(), msgToChat.length(),
        //    MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
        //     len);
        //cout << "\nHello message sent.\n";

    }
    
    return 0;
}