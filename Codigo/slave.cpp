//Slave

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

#include <thread>
#include <map>
#include <string>

#include "BD.h"
#include "RDT.h"

using std::string; using std::map;

//#define PORT_SLAVE     9090
#define MAXLINE 512

//using std::cout; using std::cin;
//using std::string;

//Variables de Red
struct sockaddr_in slaveaddr, masteraddr; 
int sockfd; 
char buffer[MAXLINE];
//int PORT_SLAVE = 9090;
//char* db_file;
//unsigned int len;
/////////////////

RDT Rdt;

//DB Sql("test1.db");
DB Sql;

bool CreacionNodo(string mensaje)
{
    //size y name de nodo
    int size_name = stoi(mensaje.substr(2,2));
    string name = mensaje.substr(4, size_name);
    //Num atributo = 1 TODO
    int num_atrib = stoi(mensaje.substr(4+size_name, 2));
    //Size y nombre atributo
    int size_atrib = stoi(mensaje.substr(6+size_name,2));
    string name_atrib = mensaje.substr(8 + size_name, size_atrib);
    //size y nombre de valor
    int size_value = stoi(mensaje.substr(8+ size_name + size_atrib, 2));
    string name_value = mensaje.substr(10+ size_name+size_atrib, size_value);

    cout << name << "," << name_atrib <<"," << name_value;
    Sql.CrearNodo(name);
    if (Sql.InsertAtributo(name, name_atrib, name_value))
        return true;
    else
    {
        return false;
    }
    
}
      
    
string CreacionRelacion (string mensaje)
{

}
      
    
string LeerAtributos (string mensaje)
{

}
       
string LeerRelacionNiveles (string mensaje)
{

}
      
string ActualizarAtributo (string mensaje)
{

}

string EliminarNodo (string mensaje)
{

}
    
string EliminarAtributo(string mensaje)
{

}
     
string EliminarRelacion (string mensaje)
{

}

string RecepcionConsulta(string mensaje)
{
    string comando = mensaje.substr(0,2);

    map<string,int> opciones {
        {"CN", 1}, //Creacion Nodo
        {"CR", 2}, //Creacion Relacion
        
        {"RN", 3}, //Leer atributos de Nodo 
        {"RR", 4}, //Leer relacion de nodo en niveles
        
        {"UA", 5}, //actualizacion de atributo nodo
        {"UR", 6}, //actualizacion de relacion CREO Q NO ES NECESARIO
        
        {"DN", 7}, //Eliminar nodo
        {"DA", 8}, //Eliminar atributo de nodo
        {"DR", 9}, //Eliminar relacion de nodo
        
        {"SN", 10}, //Mostrar informacion de nodo
        {"SA", 11}, //Mostrar atributo de nodo
        {"SR", 12} //mostrar relacion de nodo
    };

    switch (opciones[comando])
    {
    case 1: 
        if (CreacionNodo(mensaje))
            return "OK";
        else
            return "ER";
        break;
    case 2:
        CreacionRelacion (mensaje);
        break; 
    case 3:
        LeerAtributos (mensaje);
        break;   
    case 4:
        LeerRelacionNiveles (mensaje);
        break;
    case 5:
        ActualizarAtributo (mensaje);
        break;
    case 7:
        EliminarNodo (mensaje);
        break;
    case 8:
        EliminarAtributo(mensaje);
        break;
    case 9:
        EliminarRelacion (mensaje);
        break;

    
    default:
        break;
    }
}

string create_json(vector <string> c,vector<string> t){
    string result="{";
    auto row_atributo=t.begin();
    for(auto i:c){
        if(*row_atributo == t[t.size()-1]){
            result=result+*row_atributo+":'"+i+"',";
            row_atributo=t.begin();    
            continue;
        }
        result=result+*row_atributo+":'"+i+"',";
        row_atributo++;       
    }
    result[result.size()-1]='}';
    return result;
}

void PrepararRespuesta()
{

}


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
    len = sizeof(masteraddr);

    while (!completo)
    {
        bzero(buffer, MAXLINE);
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                              MSG_WAITALL, ( struct sockaddr *) &masteraddr,
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
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cout << "Parametros incompletos PORT DB";
        exit(1);
    }

    char* db_file = argv[2];
    int PORT_ = atoi(argv[1]);
    int PORT_SLAVE;
    
    Sql.Conexion(db_file);
    
    switch (PORT_)
    {
    case 1:
        PORT_SLAVE = 6060;
        break;
    case 2:
        PORT_SLAVE = 7070;
        break;
    case 3:
        PORT_SLAVE = 9090;
        break;
    
    default:
        break;
    }

    int n;
    string mensaje_in, comando;

    std::map<string, int> com = {
        {"AR", 1}, //Envio de Archivo
        {"XX", 99} //Error de Checksum
    };
        
    vector<string> vec_cadena;

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    } 
	
    memset(&slaveaddr, 0, sizeof(slaveaddr)); 
	memset(&masteraddr, 0, sizeof(masteraddr)); 
	
    slaveaddr.sin_family = AF_INET; // IPv4 
	slaveaddr.sin_addr.s_addr = INADDR_ANY; 
	slaveaddr.sin_port = htons(PORT_SLAVE); 
	if ( bind(sockfd, (const struct sockaddr *)&slaveaddr, sizeof(slaveaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //len = sizeof(masteraddr);

    //std::thread(ACKTimeout).detach();

    for(;;)
    {
        cout << "En espera\n";
        mensaje_in = EsperaPorMensaje();
        cout << mensaje_in;
        string rpta = RecepcionConsulta(mensaje_in);
        EnviarMensaje(rpta, &masteraddr);

        
        /*
        comando = mensaje_in.substr(0,2);

        switch (com[comando])
        {
        case 1: //AR
        {
            int size_name = stoi(mensaje_in.substr(2,2));
            string file_name = mensaje_in.substr(4,size_name);
            
            
            EnviarMensaje("AR", &masteraddr);
            if (EsperaPorMensaje().substr(0,2) == "OK")
            {   
                // Crear Thread para envío de mensajes
                sockaddr_in* cliente = new sockaddr_in();
                *cliente = masteraddr;
                
                std::thread(EnviarArchivoTxt, file_name, cliente).detach();                
                //EnviarMensaje(Rdt.PrepararACK(), cliente);              
            }
            else
            {
                cout << "Error";
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
        */

    }
    
    return 0;
}
