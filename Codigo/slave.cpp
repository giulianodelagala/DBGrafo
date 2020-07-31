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

#define MAXLINE 512

//Variables de Red
struct sockaddr_in slaveaddr, masteraddr; 
int sockfd; 
char buffer[MAXLINE];
/////////////////

RDT Rdt;

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

bool CreacionAtributo(string mensaje)
{
    //size y name de nodo
    int size_name = stoi(mensaje.substr(2,2));
    string name = mensaje.substr(4, size_name);
    //Size y nombre atributo
    int size_atrib = stoi(mensaje.substr(4+size_name,2));
    string name_atrib = mensaje.substr(6 + size_name, size_atrib);
    //size y nombre de valor
    int size_value = stoi(mensaje.substr(6+ size_name + size_atrib, 2));
    string name_value = mensaje.substr(8+ size_name+size_atrib, size_value);

    cout << name << "," << name_atrib <<"," << name_value;
    
    if (Sql.InsertAtributo(name, name_atrib, name_value))
        return true;
    else
    {
        return false;
    } 
}
      
bool CreacionRelacion (string mensaje)
{
    //from_size y from_name de la relacion
    int from_size = stoi(mensaje.substr(2,2));
    string from_name = mensaje.substr(4, from_size);
    //to_size y to_name de la relacion
    int to_size = stoi(mensaje.substr(4+from_size,2));
    string to_name = mensaje.substr(6+from_size, to_size);

    cout << from_name << "," << to_name << "\n";
    if (Sql.InsertRelacion(from_name,to_name))
        return true;
    else
        return false;
}
      
    
string LeerAtributos (string mensaje)
{
    //size y name de nodo
    int size_name = stoi(mensaje.substr(2,2));
    string name = mensaje.substr(4, size_name); 

    vector <Atr> consulta = Sql.ReadAtributosNodo(name);
    vector <string> esquema={"atributo","valor"};
    string result="[";
    for(auto i:consulta){
        result+="{";
        result=result+esquema[0]+":"+string(i.atrib)+",";
        result=result+esquema[1]+":"+string(i.value)+"},";
    }
    result[result.size()-1]=']';
    result = "RN" + Rdt.PadZeros(result.length(), 3) + result;
    return result;
}
       
string LeerAmigos (string mensaje)
{
    string msg = "";
    //size y name de nodo
    int size_name = stoi(mensaje.substr(2,2));
    string name = mensaje.substr(4, size_name);

    vector<string> id_nodos = Sql.GetNameFriends(name);
    for(int j = 0; j < id_nodos.size(); j++){
        msg += id_nodos[j] + ",";
    }
    msg = msg.substr(0, msg.length()-1);
    msg = "RF" + Rdt.PadZeros(msg.length(), 3) + msg;
    cout << msg;
    return msg;
}
      
bool ActualizarAtributo (string mensaje)
{
    //size y name de nodo
    int size_name = stoi(mensaje.substr(2,2));
    string name = mensaje.substr(4, size_name); 
    //size y name del tipo de atributo
    int size_tipo_atrib = stoi(mensaje.substr(4+size_name,2));
    string tipo_atrib = mensaje.substr(6+size_name, size_tipo_atrib);

    //size y nuevo valor del atributo
    int size_new_val = stoi(mensaje.substr(6+size_name+size_tipo_atrib,2));
    string new_val = mensaje.substr(8+size_name+size_tipo_atrib, size_new_val);

    cout << name << "," << tipo_atrib << "," << new_val << "\n";
    if (Sql.UpdateAtributo(name,tipo_atrib,new_val))
        return true;
    else
        return false;
}

bool EliminarNodo (string mensaje)
{
    //size y name de nodo
    int size_name = stoi(mensaje.substr(2,2));
    string name = mensaje.substr(4, size_name); 

    cout << name << "\n";
    if (Sql.DeleteNodo(name))
        return true;
    else
        return false;
}
    
bool EliminarAtributo(string mensaje)
{
    //size y name de nodo
    int size_name = stoi(mensaje.substr(2,2));
    string name = mensaje.substr(4, size_name); 
    //size y name del tipo de atributo
    int a_size = stoi(mensaje.substr(4+size_name,2));
    string a_name= mensaje.substr(6+size_name, a_size);

    cout << name << "," << a_name << "\n";
    if (Sql.DeleteAtributo(name, a_name))
        return true;
    else 
        return false;
}
     
bool EliminarRelacion (string mensaje)
{
    //size y name de nodo
    int size_name = stoi(mensaje.substr(2,2));
    string name = mensaje.substr(4, size_name); 
    //size y name de la relacion
    int to_size = stoi(mensaje.substr(4+size_name,2));
    string to_name= mensaje.substr(6+size_name, to_size);

    cout << name << "," << to_name << "\n";
    if (Sql.DeleteRelacion(name, to_name))
        return true;
    else 
        return false;
}

string RecepcionConsulta(string mensaje)
{
    string comando = mensaje.substr(0,2);

    map<string,int> opciones {
        {"CN", 1}, //Creacion Nodo
        {"CR", 2}, //Creacion Relacion
        
        {"RN", 3}, //Leer atributos de Nodo 
        {"RF", 4}, //Leer relacion de nodo en niveles
        
        {"UA", 5}, //actualizacion de atributo nodo
        
        {"CA", 6}, //creacion de atributo en nodo
        
        {"DN", 7}, //Eliminar nodo
        {"DA", 8}, //Eliminar atributo de nodo
        {"DR", 9}, //Eliminar relacion de nodo
        
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
        if (CreacionRelacion (mensaje))
            return "OK";
        else
            return "ER";
        break; 
    case 3:
        return LeerAtributos (mensaje);
        break;   
    case 4:
        return LeerAmigos (mensaje);
        break;
    case 5:
        if (ActualizarAtributo (mensaje))
            return "OK";
        else
            return "ER";
        break;
    case 6:
        if (CreacionAtributo(mensaje))
            return "OK";
        else 
            return "ER";
    case 7:
        if (EliminarNodo (mensaje))
            return "OK";
        else
            return "ER";
        break;
    case 8:
        if (EliminarAtributo(mensaje))
            return "OK";
        else
            return "ER";
        break;
    case 9:
        if (EliminarRelacion (mensaje))
            return "OK";
        else 
            return "ER";
        break;

    default:
        return "ER";
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
    
    std::map<int,int> map_puertos{
        {0,9090},
        {1,9191},
        {2,9292},
        {3,9393}
    };
    PORT_SLAVE = map_puertos[PORT_];

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

    //std::thread(ACKTimeout).detach();

    for(;;)
    {
        cout << "En espera\n";
        mensaje_in = EsperaPorMensaje();
        cout << mensaje_in;
        string rpta = RecepcionConsulta(mensaje_in);
        EnviarMensaje(rpta, &masteraddr);
    }
    
    return 0;
}
