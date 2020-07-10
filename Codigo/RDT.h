//Utilidades RDT

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <math.h>
#include <sys/types.h>

#include <fstream>

using std::string; using std::cout;
using std::queue;
using std::vector;

class RDT;

class CT //Constantes
{
public:
  static const int ind_secuencia = 0; //indice secuencia
  static const int pos_secuencia = 5; //primeros 5 bytes capa transporte
  
  static const int ind_pid = 5; //indice process id
  static const int pos_pid = 5; //5 bytes process id
  
  static const int ind_flujo = 10; //indice flujo
  static const int pos_flujo = 3;  //primeros 3 bytes capa aplicacion
  
  static const int ind_sec_flujo = 13; //indice de pos_sec_flujo
  static const int pos_sec_en_flujo = 2; //secuencia dentro de flujo
  
  static const int ind_flag = 15; //indice flag
  static const int pos_flag = 1; //secuencia dentro de flujo

  static const int ind_payload = 16; //indice payload

  static const int divisor = 10; //para generación de checksum
  static const int max_size_msg = 490;

};



struct Package
{
  unsigned int secuence;
  unsigned int pid;
  unsigned int flujo;
  unsigned int sec_flujo;
  bool flag;
  string payload;

  Package(unsigned int secuence,
  unsigned int pid,
  unsigned int flujo,
  unsigned int sec_flujo,
  string payload)
  {
    this->secuence = secuence;
    this->pid = pid;
    this->flujo = flujo;
    this->sec_flujo = sec_flujo;
    this->payload = payload;
  }

  Package(string paquete)
  {
    this->secuence = stoi(paquete.substr(CT::ind_secuencia, CT::pos_secuencia));
    this->pid = stoi(paquete.substr(CT::ind_pid, CT::pos_pid) );
    this->flujo = stoi(paquete.substr(CT::ind_flujo, CT::pos_flujo));
    this->sec_flujo = stoi(paquete.substr(CT::ind_sec_flujo, CT::pos_sec_en_flujo));
    this->flag = stoi(paquete.substr(CT::ind_flag, CT::pos_flag));

    this->payload = paquete.substr(CT::ind_payload);
    this->payload = this->payload.substr(0,this->payload.length()-1); //Retirar Checksum
  }
};

struct Flujo
{
  unsigned int pid = -1;
  unsigned int sec_fin = -1;
  unsigned int num_flujo = -1;
  unsigned int contador = -1;

  std::map<int, Package*> map_paquetes;

  Flujo(unsigned int num_flujo, unsigned int pid)
  {
    this->num_flujo = num_flujo;
    this->pid = pid;
  }

  Flujo(unsigned int sec_ini,
  unsigned int sec_fin,
  unsigned int flujo)
  {
    this->sec_fin;
    this->num_flujo;
  }

  ~Flujo()
  {
    for (int i = 0; i< sec_fin; ++i)
    {
       delete map_paquetes[i];
    }
  }

  bool InsertarPackage(Package* paquete)
  {
    if (paquete->flujo != num_flujo)
      return false;
    else
    {
      if (map_paquetes.count(paquete->sec_flujo))
      {
        //paquete existe
        return true;
      }
      else
      {
        map_paquetes[paquete->sec_flujo] = paquete;
        contador++;
        //Actualizar atributos de Flujo
        if (paquete->flag == 1)
        { //Paquete final de flujo
          this->sec_fin = paquete->sec_flujo;
        }
        
        return true;
      }
    }
  }

  string ExtraerMensaje()
  {
    string mensaje ="";
    if (contador != sec_fin)
    { //Mensaje aun incompleto
      return "Error";
    }
    else
    {
      for (int i = 0; i < sec_fin+1; ++i)
      {
        mensaje+= map_paquetes[i]->payload;
      }
    }
    return mensaje;  
  }

  bool IsCompleto()
  {
    return contador == sec_fin;
  }

};

class RDT
{
public:
  const unsigned int MAX_FLUJOS = 999;
  const unsigned int MAX_SECUENCIAS = 99999;
  const unsigned int CONTROL_ACK = 100; //llevar control de ultimos 100 paquetes

  vector<string>* VEC_SECUENCIAS_OUT;
  vector<bool>* VEC_SECUENCIAS_IN; //control 

  vector<Flujo*>* VEC_FLUJOS_OUT;
  vector<Flujo*>* VEC_FLUJOS_IN;

  queue<int> cola_flujos_in;

  unsigned int SECUENCIA_OUT_ACTUAL = 0; 
  unsigned int FLUJO_OUT_ACTUAL = 0; //Numero de flujo salida

  //unsigned int SECUENCIA_ACK_ACTUAL = 100; //Enviar ack de los ultimos 100 paquetes
  

  string PadZeros(int number, int longitud);
  int GenChecksum(string cadena);
  bool VerificarChecksum(string cadena);
  int GetPID();

  void PreparacionMensaje(string mensaje);
  bool RecepcionPaquete(string Paquete);

  string PrepararACK(); //TODO
  
  RDT()
  {
    VEC_FLUJOS_OUT = new vector<Flujo*>(MAX_FLUJOS,nullptr);
    VEC_SECUENCIAS_OUT = new vector<string>(MAX_SECUENCIAS,"");

    VEC_FLUJOS_IN = new vector<Flujo*>(MAX_FLUJOS,nullptr);
    VEC_SECUENCIAS_IN = new vector<bool>(MAX_SECUENCIAS,false);
  }
};

string RDT::PadZeros(int number, int longitud)
{
  string num_letra = std::to_string(number);
  for (int i = num_letra.length(); i < longitud; ++i)
    num_letra = "0" + num_letra;
  
  return num_letra;
}

int RDT::GenChecksum(string cadena)
{
  int suma=0;
	for (int i = 0; i < 6; i++){
		suma+=int(cadena[i]);
	}
	suma=suma%CT::divisor;
	return suma;
}

bool RDT::VerificarChecksum(string cadena){
  int suma=0;
	suma = GenChecksum(cadena);
  string temp=cadena.substr(cadena.length()-1);

	if(suma == stoi(cadena.substr(cadena.length()-1)))
		return true;
  else
	  return false;
}

void RDT::PreparacionMensaje(string mensaje)
{
  string flag_fin = "0";
  std::vector<string> vec_paquete;
 
  int num_secuencias = ceil ( mensaje.length() / (float) CT::max_size_msg);

  // i es secuencia en flujo
  for (int i = 0; i < num_secuencias; ++i)
  {
    if (i == num_secuencias -1 )
      flag_fin = "1";

    string cadena = "";
    string temp = mensaje.substr(i*CT::max_size_msg, CT::max_size_msg);
    
    cadena += PadZeros(SECUENCIA_OUT_ACTUAL, CT::pos_secuencia) +
              PadZeros(GetPID(), CT::pos_pid) +
              PadZeros(FLUJO_OUT_ACTUAL, CT::pos_flujo) +
              PadZeros(i, CT::pos_sec_en_flujo) +
              flag_fin +
              temp;
    cadena += std::to_string(GenChecksum(cadena));
    //Agregar al vector de secuencias de salida para posible uso posterior
    //llevar control de numero secuencia inicial y final 
    VEC_SECUENCIAS_OUT->at(SECUENCIA_OUT_ACTUAL) = cadena;
    vec_paquete.push_back(cadena);
    SECUENCIA_OUT_ACTUAL++;
  }
  FLUJO_OUT_ACTUAL++;
}

int RDT::GetPID()
{
  return (int) gettid();
}

bool RDT::RecepcionPaquete(string mensaje)
{
  if (VerificarChecksum(mensaje))
  {
    cout << "\nPaquete Verificado Checksum";
  }
  else
  {
    cout << "Paquete Erroneo Checksum";
    return false;
  }
  
  //Crear Paquete
  Package* pkg = new Package (mensaje);
  Flujo* flujo = VEC_FLUJOS_IN->at(pkg->flujo);
  //Verificar si el flujo existe
  if ( flujo == nullptr)
  {
    //Flujo no existe : Crear Flujo
    cout << "\nFlujo creado";
    flujo = new Flujo(pkg->flujo, pkg->pid);
    VEC_FLUJOS_IN->at(pkg->flujo) = flujo;
    cola_flujos_in.push(pkg->flujo);
  }
  
  if (flujo->InsertarPackage(pkg))
  {
    cout << "\nPkg Insert Flujo:" << pkg->flujo << " Sec " << pkg->sec_flujo << " PID: " << pkg->pid;
    //VEC_SECUENCIAS_IN->at(pkg->secuence) = true; //Marcado como recibido correctamente 
    return true;
  }
  else
  {
    cout << "\nPkg NO insert Flujo:" << pkg->flujo << "Sec" << pkg->sec_flujo;
    delete pkg;
    return false;
  }
}

string RDT::PrepararACK()
{
  return "AA"; //Prueba de Envio ACK
}

