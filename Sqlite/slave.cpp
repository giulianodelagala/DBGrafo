//Slave

#include <map>
#include <string>

#include "BD.h"

using std::string; using std::map;

DB Sql("test.db");

string CreacionNodo(string mensaje)
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
    Sql.InsertAtributo(name, name_atrib, name_value);
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

void RecepcionConsulta(string mensaje)
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
        CreacionNodo(mensaje);
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

void PrepararRespuesta()
{

}



int main()
{
    
    string consulta = "CN04Pepe0104Edad0222";
    RecepcionConsulta(consulta);

    return 0;
}