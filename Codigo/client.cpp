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

//Variables de Red
int sockfd;
char buffer[MAXLINE];
struct sockaddr_in  servaddr;
socklen_t len;
///////////////////

RDT Rdt;

void EnviarPaquete(string cadena);

void EnviarMensaje(string mensaje)
{
    int secuencia_ini = Rdt.SECUENCIA_OUT_ACTUAL; //numero de secuencia antes de creacion datagram
    Rdt.PreparacionMensaje(mensaje);
    int secuencia_fin = Rdt.SECUENCIA_OUT_ACTUAL; //Número actual luego de creacion datagram

    for (int sec= secuencia_ini; sec < secuencia_fin; ++sec)
    {
        EnviarPaquete(Rdt.VEC_SECUENCIAS_OUT->at(sec));
    }
}

void EnviarPaquete(string cadena)
{
    sendto(sockfd, cadena.c_str(), cadena.length(),
            MSG_CONFIRM, (const struct sockaddr *) &servaddr,
             sizeof(servaddr));
        cout << "\nPaquete enviado";
}

string EsperaPorMensaje()
{
    bool completo = false;
    unsigned int flujo_actual;
    int n;
    string mensaje_in = "";

    while (!completo)
    {
        bzero(buffer, MAXLINE);
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                                MSG_WAITALL, (struct sockaddr *) &servaddr,
                                &len);
        if (Rdt.RecepcionPaquete(string(buffer)) )
        {
            cout << "\nPaquete Recibido";
        }
        //Verificar si tenemos mensaje completo
        flujo_actual = Rdt.cola_flujos_in.front();
        if (Rdt.VEC_FLUJOS_IN->at(flujo_actual)->IsCompleto())
        {
            mensaje_in = Rdt.VEC_FLUJOS_IN->at(flujo_actual)->ExtraerMensaje();
            //Eliminar flujo
            delete Rdt.VEC_FLUJOS_IN->at(flujo_actual);
            Rdt.VEC_FLUJOS_IN->at(flujo_actual) = nullptr;
            Rdt.cola_flujos_in.pop();
            completo = true;
        }
    }
    return mensaje_in;
}

int main()
{ 
    struct hostent *host;

    bool completo = false;
    unsigned int flujo_actual = -1;
    string mensaje_in = "", mensaje_out = "";

    string comando;
    
    std::map<string, int> com = {
        {"AR", 1} //Envio de Archivo
    };
 
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

    int n;
    
    for (;;)
    {
        cout << "\nNombre Archivo";
        getline(cin, mensaje_out);

        //Enviar nombre de archivo a transmitir
        //Por ahora incluir numero de caracteres del nombre
        //ejm: 09texto.txt
        EnviarMensaje("AR"+mensaje_out);

        //Espera de mensaje
        mensaje_in = EsperaPorMensaje();
    
        //Procesar mensaje payload
        //Extraccion de comando
        comando = mensaje_in.substr(0,2);
        
        switch (com[comando])
        {
        case 1: //AR 
        {
            EnviarMensaje("OK");
            mensaje_in = EsperaPorMensaje();
            cout << "\nMensaje Recibido:" << mensaje_in;
            break;
        }
              
        default:
            break;
        }
    }

    close(sockfd);
    return 0;
}