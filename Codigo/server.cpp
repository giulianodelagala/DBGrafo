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
#include <sstream> //stringstream

#include "RDT.h"

#define PORT     8080
#define MAXLINE 512

using std::cout; using std::cin;
using std::string;

//Variables de Red
struct sockaddr_in servaddr, cliaddr; 
int sockfd; 
char buffer[MAXLINE]; 
unsigned int len;
/////////////////

RDT Rdt;

string File2String (string filename);
string Txt2String (string filename);
void EnviarPaquete(string cadena);

//TODO ELIMINAR NO ES NECESARIO PRIMERO GENERAR MENSAJE
/*
void EnvioArchivo(string file_name)
{
    int secuencia_ini = Rdt.SECUENCIA_OUT_ACTUAL; //numero de secuencia antes de creacion datagram
    string cadena_envio = File2String(file_name);
    Rdt.PreparacionMensaje(cadena_envio);
    int secuencia_fin = Rdt.SECUENCIA_OUT_ACTUAL; //Número actual luego de creacion datagram

    for (int sec= secuencia_ini; sec < secuencia_fin; ++sec)
    {
        EnviarPaquete(Rdt.VEC_SECUENCIAS_OUT->at(sec));
    }
}
*/
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
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
             len);
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
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                              MSG_WAITALL, ( struct sockaddr *) &cliaddr,
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
    //struct sockaddr_in servaddr, cliaddr; 
    //int sockfd; 
	//char buffer[MAXLINE]; 
	//unsigned int len;
    int n;
    string mensaje_in, comando;

    std::map<string, int> com = {
        {"AR", 1} //Envio de Archivo
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

    len = sizeof(cliaddr);
    
    //string file_cadena = Rdt.File2String("lena.jpg");
    //cout << file_cadena;

    for(;;)
    {
        mensaje_in = EsperaPorMensaje();

        //n = recvfrom(sockfd, (char *)buffer, MAXLINE,
         //                       MSG_WAITALL, ( struct sockaddr *) &cliaddr,
           //                     &len);
        //buffer[n] = '\0';
        //cout << "\nClient : %s\n" << buffer;
        //cout << "add" << cliaddr.sin_addr.s_addr;
        //string cadena(buffer);
        comando = mensaje_in.substr(0,2);

        switch (com[comando])
        {
        case 1: //AR
        {
            int size_name = stoi(mensaje_in.substr(2,2));
            string file_name = mensaje_in.substr(4,size_name);
            EnviarMensaje("AR");
            if (EsperaPorMensaje().substr(0,2) == "OK")
            {
                string texto = Txt2String(file_name);
                EnviarMensaje(texto);
            }
                
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

string File2String (string filename)
{
  std::streampos size;
  char* memblock; //bloque de memoria donde se albergara file

  std::ifstream file(filename, std::ios::in|std::ios::binary|std::ios::ate);
  if (file.is_open())
  {
    size = file.tellg();
    memblock = new char[size];
    file.seekg(0, std::ios::beg);
    file.read (memblock, size);
    file.close();
    string filecadena(memblock);
    delete[] memblock;
    return filecadena;
  }
  else
    return "Error";
}

string Txt2String (string filename)
{
    std::ifstream t(filename);
    std::stringstream cadena;
    cadena << t.rdbuf();
    return cadena.str();
}