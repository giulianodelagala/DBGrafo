//ss.cpp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <netdb.h>
//#include <errno.h>
//#include <ctime>

#include <iostream>
#include <sstream> //stringstream
#include <thread>

#include "RDT.h"
#include "manejo_archivos.h"

#define PORT     8080
#define PORT_SLAVE 9090
#define MAXLINE 512

using std::cout; using std::cin;
using std::string;

//Variables de Red
struct sockaddr_in servaddr, cliaddr; 
int sockfd; 
char buffer[MAXLINE]; 
struct sockaddr_in  slaveaddr;
//unsigned int len;
/////////////////

RDT Rdt;

void EnviarPaquete(string cadena, sockaddr_in* cliente);

void EnviarMensaje(string mensaje, sockaddr_in* cliente)
{
    int secuencia_ini = Rdt.SECUENCIA_OUT_ACTUAL; //numero de secuencia antes de creacion datagram
    Rdt.PreparacionMensaje(mensaje);
    int secuencia_fin = Rdt.SECUENCIA_OUT_ACTUAL; //Número actual luego de creacion datagram

    for (int sec= secuencia_ini; sec < secuencia_fin; ++sec)
    {
        EnviarPaquete(Rdt.VEC_SECUENCIAS_OUT->at(sec), cliente);
    } 
}

void EnviarArchivoTxt(string filename, sockaddr_in* cliente)
{
    string texto = Txt2String(filename);
    EnviarMensaje (texto, cliente);
    EnviarMensaje ("Fin de Archivo", cliente);
}

void EnviarArchivo(string filename, sockaddr_in* cliente)
{
    string file_ = File2String(filename);
    EnviarMensaje (file_, cliente);
    EnviarMensaje ("Fin de Archivo", cliente);
}

void EnviarPaquete(string cadena, sockaddr_in* cliente)
{
    unsigned int len;
    len = sizeof(*cliente);
    sendto(sockfd, cadena.c_str(), cadena.length(),
            MSG_CONFIRM, (const struct sockaddr *) cliente,
             len);
        cout << "\nPaquete enviado";
}

string EsperaPorMensaje()
{
    bool completo = false;
    unsigned int flujo_actual;
    int n;
    string mensaje_in = "";
    unsigned int len;
    len = sizeof(cliaddr);

    while (!completo)
    {
        bzero(buffer, MAXLINE);
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                              MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                            &len);

        if (Rdt.RecepcionPaquete(string(buffer)) )
        {
            cout << "\nPaquete Recibido";
        }
        else
        {
            return "XX"; //Error de Checksum
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

/*
void ACKTimeout()
{
    while (true)
    {
        sleep(5);
        EnviarMensaje(Rdt.PrepararACK());
    } 
}
*/
int main()
{
    //Slave//
    struct hostent *slave;
    slave = (struct hostent *)gethostbyname((char *)"127.0.0.1"); //"51.15.220.108"
    memset(&slaveaddr, 0, sizeof(slaveaddr));
    // Filling slave information
    slaveaddr.sin_family = AF_INET;
    slaveaddr.sin_port = htons(PORT_SLAVE);
    slaveaddr.sin_addr = *((struct in_addr *)slave->h_addr);
    ////////////////

    int n;
    string mensaje_in, comando;

    std::map<string, int> com = {
        {"CN", 1}, //Envio de Archivo
        {"XX", 99} //Error de Checksum
    };
        
    vector<string> vec_cadena;

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

    //len = sizeof(cliaddr);

    //std::thread(ACKTimeout).detach();

    for(;;)
    {
        cout << "En espera";
        mensaje_in = EsperaPorMensaje();

        comando = mensaje_in.substr(0,2);

        switch (com[comando])
        {
        case 1: //AR
        {
                        
            EnviarMensaje(mensaje_in, &slaveaddr);

            /*
            if (EsperaPorMensaje().substr(0,2) == "OK")
            {   
                // Crear Thread para envío de mensajes
                sockaddr_in* cliente = new sockaddr_in();
                *cliente = cliaddr;
                
                std::thread(EnviarArchivoTxt, file_name, cliente).detach();                
                //EnviarMensaje(Rdt.PrepararACK(), cliente);              
            }
            else
            {
                cout << "Error";
            }
              */          
            break;
        }
        case 99: //Error de Checksum
        {
            cout << "Error de Recepción Checksum";
            break;
        }

        default:
            break;
        }

    }
    
    return 0;
}