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
#define PORT_SLAVE 6060
#define PORT_SLAVE2 7070
#define PORT_SLAVE3 9090
#define MAXLINE 512

using std::cout; using std::cin;
using std::string;
using std::map;

//Variables de Red
struct sockaddr_in servaddr, cliaddr; 
int sockfd; 
char buffer[MAXLINE]; 
struct sockaddr_in  slaveaddr, slaveaddr2, slaveaddr3;
int NSLAVE = 3;
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

string EsperaPorMensaje(int &pid)
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
            pid = Rdt.VEC_FLUJOS_IN->at(flujo_actual)->pid;
            //Eliminar flujo
            delete Rdt.VEC_FLUJOS_IN->at(flujo_actual);
            Rdt.VEC_FLUJOS_IN->at(flujo_actual) = nullptr;
            Rdt.cola_flujos_in.pop();
            completo = true;
            
        }
    }
    return mensaje_in;
}

int funHash(string cadena)
{
        int hashVal, j;
        hashVal = (int) cadena[0];
        for (j = 1; j < cadena.size(); j++)
            hashVal += (int) cadena[j];
        return(hashVal % NSLAVE);
}

int ElegirSlave(string mensaje)
{
    //size y name de nodo
    int size_name = stoi(mensaje.substr(2,2));
    string name = mensaje.substr(4, size_name);
    return funHash(name);
}

vector<int> ElegirSlaveRelacion(string mensaje, string &new_mensaje)
{
    vector<int> slaves;
    
    //from_size y from_name de la relacion
    int from_size = stoi(mensaje.substr(2,2));
    string from_name = mensaje.substr(4, from_size);

    //to_size y to_name de la relacion
    int to_size = stoi(mensaje.substr(4+from_size,2));
    string to_name = mensaje.substr(6+from_size, to_size);

    new_mensaje = mensaje.substr(4+from_size,2) +
     to_name + mensaje.substr(2,2) + from_name;

    slaves.push_back(funHash(from_name));
    slaves.push_back(funHash(to_name));
    return slaves;
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

    //Slave2//
    struct hostent *slave2;
    slave2 = (struct hostent *)gethostbyname((char *)"127.0.0.1"); //"51.15.220.108"
    memset(&slaveaddr2, 0, sizeof(slaveaddr2));
    // Filling slave information
    slaveaddr2.sin_family = AF_INET;
    slaveaddr2.sin_port = htons(PORT_SLAVE2);
    slaveaddr2.sin_addr = *((struct in_addr *)slave2->h_addr);
    ////////////////

    //Slave3//
    struct hostent *slave3;
    slave3 = (struct hostent *)gethostbyname((char *)"127.0.0.1"); //"51.15.220.108"
    memset(&slaveaddr3, 0, sizeof(slaveaddr3));
    // Filling slave information
    slaveaddr3.sin_family = AF_INET;
    slaveaddr3.sin_port = htons(PORT_SLAVE3);
    slaveaddr3.sin_addr = *((struct in_addr *)slave3->h_addr);
    ////////////////


    std::map<int,sockaddr_in> map_slave{
        {0, slaveaddr},
        {1, slaveaddr2},
        {2, slaveaddr3}
    };

    std::map<int,sockaddr_in> map_cliente;

    int n, cliente_pid, slave_pid;
    string mensaje_in, comando;

    map<string,int> opciones {
        {"CN", 1}, //Creacion Nodo
        {"CR", 2}, //Creacion Relacion
        
        {"RN", 3}, //Leer atributos de Nodo 
        {"RF", 4}, //Leer relacion de nodo en niveles
        
        {"UA", 5}, //actualizacion de atributo nodo
        {"UR", 6}, //actualizacion de relacion CREO Q NO ES NECESARIO
        
        {"DN", 7}, //Eliminar nodo
        {"DA", 8}, //Eliminar atributo de nodo
        {"DR", 9}, //Eliminar relacion de nodo
        
        {"SN", 10}, //Mostrar informacion de nodo
        {"SA", 11}, //Mostrar atributo de nodo
        {"SR", 12}, //mostrar relacion de nodo
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
    //MONOCLIENTE MEJORAR


    for(;;)
    {
        cout << "En espera comando";
        mensaje_in = EsperaPorMensaje(cliente_pid);
        map_cliente[cliente_pid] = cliaddr;

        comando = mensaje_in.substr(0,2);
        

        switch (opciones[comando])
        {
        case 1: //Creacion Nodo
        {
            int cualslave = ElegirSlave(mensaje_in); 
            cout << "\nEnviando a Slave: " << cualslave;
            EnviarMensaje(mensaje_in, &map_slave[cualslave]);

            //Responder Consulta
            string mensaje_out = EsperaPorMensaje(slave_pid);
            cout << "\nResultado: " << mensaje_out;
            if (mensaje_out == "OK")
                EnviarMensaje("OK201", &map_cliente[cliente_pid]);
            else
                EnviarMensaje("ER400", &map_cliente[cliente_pid]);         
            break;
        }
        case 2: //Creacion Relacion
        {
            string new_mensaje;
            vector<int> slaves = ElegirSlaveRelacion(mensaje_in, new_mensaje);

            cout << "\nEnviando a Slave: " << slaves[0];
            EnviarMensaje(mensaje_in, &map_slave[slaves[0]]);
            string mensaje_out_1 = EsperaPorMensaje(slave_pid);
            cout << "\nResult: " << slaves[0] << mensaje_out_1;

            //Relacion Inversa
            cout << "\nEnviando a Slave: " << slaves[1];
            EnviarMensaje("CR"+new_mensaje, &map_slave[slaves[1]]);
            string mensaje_out_2 = EsperaPorMensaje(slave_pid);
            cout << "\nResult: " << slaves[1] << mensaje_out_2;

            //Responder Consulta
            if (mensaje_out_1 == "OK" && mensaje_out_2 == "OK")
                EnviarMensaje("OK202", &map_cliente[cliente_pid]);
            else
            {
               EnviarMensaje("ER400", &map_cliente[cliente_pid]);
            }
            break;
        }

        case 3: //Leer atributos de Nodo 
        {
            int cualslave = ElegirSlave(mensaje_in); 
            cout << "\nEnviando a Slave: " << cualslave;
            EnviarMensaje(mensaje_in, &map_slave[cualslave]);

            //Responder Consulta
            string mensaje_out = EsperaPorMensaje(slave_pid);
            cout << "\nResultado: " << mensaje_out;
            //if (mensaje_out == "OK")
            EnviarMensaje(mensaje_out, &map_cliente[cliente_pid]);
            //else
            //    EnviarMensaje("ER400", &map_cliente[cliente_pid]);         
            break;
        }

        case 4: //Leer relacion de nodo
        {
            //Un nivel 
            int cualslave = ElegirSlave(mensaje_in); 
            cout << "\nEnviando a Slave: " << cualslave;
            EnviarMensaje(mensaje_in, &map_slave[cualslave]);

            //Responder Consulta
            string mensaje_out = EsperaPorMensaje(slave_pid);
            cout << "\nResultado: " << mensaje_out;
            //if (mensaje_out == "OK")
            EnviarMensaje(mensaje_out, &map_cliente[cliente_pid]);
            //else
            //    EnviarMensaje("ER400", &map_cliente[cliente_pid]);         
            break;
        }

        case 5: //actualizacion de atributo nodo
        {
            int cualslave = ElegirSlave(mensaje_in); 
            cout << "\nEnviando a Slave: " << cualslave;
            EnviarMensaje(mensaje_in, &map_slave[cualslave]);

            //Responder Consulta
            string mensaje_out = EsperaPorMensaje(slave_pid);
            cout << "\nResultado: " << mensaje_out;
            if (mensaje_out == "OK")
                EnviarMensaje("OK201", &map_cliente[cliente_pid]);
            else
                EnviarMensaje("ER400", &map_cliente[cliente_pid]);         
            break;
        }
        
        case 6: //actualizacion de relacion CREO Q NO ES NECESARIO
        {
            break;
        }

        case 7: //Eliminar nodo
        {
            int cualslave = ElegirSlave(mensaje_in); 
            cout << "\nEnviando a Slave: " << cualslave;
            EnviarMensaje(mensaje_in, &map_slave[cualslave]);

            //Responder Consulta
            string mensaje_out = EsperaPorMensaje(slave_pid);
            cout << "\nResultado: " << mensaje_out;
            if (mensaje_out == "OK")
                EnviarMensaje("OK207", &map_cliente[cliente_pid]);
            else
                EnviarMensaje("ER400", &map_cliente[cliente_pid]);          
            break;
        }

        case 8: //Eliminar atributo de nodo
        {
            int cualslave = ElegirSlave(mensaje_in); 
            cout << "\nEnviando a Slave: " << cualslave;
            EnviarMensaje(mensaje_in, &map_slave[cualslave]);

            //Responder Consulta
            string mensaje_out = EsperaPorMensaje(slave_pid);
            cout << "\nResultado: " << mensaje_out;
            if (mensaje_out == "OK")
                EnviarMensaje("OK208", &map_cliente[cliente_pid]);
            else
                EnviarMensaje("ER400", &map_cliente[cliente_pid]);          
            break;
        }
        case 9: //Eliminar relacion de nodo
        {
            string new_mensaje;
            vector<int> slaves = ElegirSlaveRelacion(mensaje_in, new_mensaje);

            cout << "\nEnviando a Slave: " << slaves[0];
            EnviarMensaje(mensaje_in, &map_slave[slaves[0]]);
            string mensaje_out_1 = EsperaPorMensaje(slave_pid);
            cout << "\nResult: " << slaves[0] << mensaje_out_1;

            //Relacion Inversa
            cout << "\nEnviando a Slave: " << slaves[1];
            EnviarMensaje("DR"+new_mensaje, &map_slave[slaves[1]]);
            string mensaje_out_2 = EsperaPorMensaje(slave_pid);
            cout << "\nResult: " << slaves[1] << mensaje_out_2;

            //Responder Consulta
            if (mensaje_out_1 == "OK" && mensaje_out_2 == "OK")
                EnviarMensaje("OK209", &map_cliente[cliente_pid]);
            else
            {
               EnviarMensaje("ER400", &map_cliente[cliente_pid]);
            }
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