//Utilidades RDT

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <fstream>

using std::string;

class RDT;

class CT //Constantes
{
public:
  static const int ind_secuencia = 0; //indice secuencia
  static const int pos_secuencia = 5; //primeros 5 bytes capa transporte
  
  static const int ind_flujo = 5; //indice flujo
  static const int pos_flujo = 3;  //primeros 3 bytes capa aplicacion
  
  static const int ind_sec_flujo = 8; //indice de pos_sec_flujo
  static const int pos_sec_en_flujo = 2; //secuencia dentro de flujo
  
  static const int ind_flag = 10; //indice flag
  static const int pos_flag = 1; //secuencia dentro de flujo

  static const int ind_payload = 11; //indice payload

  static const int divisor = 10; //para generación de checksum
  static const int max_size_msg = 500;

};



struct Package
{
  unsigned int secuence;
  unsigned int flujo;
  unsigned int sec_flujo;
  bool flag;
  string payload;

  Package(unsigned int secuence,
  unsigned int flujo,
  unsigned int sec_flujo,
  string payload)
  {
    this->secuence = secuence;
    this->flujo = flujo;
    this->sec_flujo = sec_flujo;
    this->payload = payload;
  }

  Package(string paquete)
  {
    this->secuence = stoi(paquete.substr(CT::ind_secuencia, CT::pos_secuencia));
    this->flujo = stoi(paquete.substr(CT::ind_flujo, CT::pos_flujo));
    this->sec_flujo = stoi(paquete.substr(CT::ind_sec_flujo, CT::pos_sec_en_flujo));
    this->flag = stoi(paquete.substr(CT::ind_flag, CT::pos_flag));

    this->payload = paquete.substr(CT::ind_payload);
    this->payload = this->payload.substr(0,this->payload.length()-1); 
  }
};

struct Flujo
{
  unsigned int sec_ini;
  unsigned int sec_fin;
  unsigned int flujo;
  unsigned int contador = 0;

  std::map<int, Package*> map_paquetes;

  Flujo(unsigned int sec_ini,
  unsigned int sec_fin,
  unsigned int flujo)
  {
    this->sec_ini;
    this->sec_fin;
    this->flujo;
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
    if (paquete->flujo != flujo)
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
/*
    static const int ind_secuencia = 0; //indice secuencia
    static const int pos_secuencia = 5; //primeros 5 bytes capa transporte
    
    static const int ind_flujo = 5; //indice flujo
    static const int pos_flujo = 3;  //primeros 3 bytes capa aplicacion
    
    static const int ind_sec_flujo = 8; //indice de pos_sec_flujo
    static const int pos_sec_en_flujo = 2; //secuencia dentro de flujo
    
    static const int ind_flag = 10; //indice flag
    static const int pos_flag = 1; //secuencia dentro de flujo

    static const int ind_payload = 11; //indice payload

    static const int divisor = 10; //para generación de checksum
    static const int max_size_msg = 500;
*/
    std::map<int,string> map_secuencias;

    std::vector<string> PreparacionMensaje(string mensaje,
      unsigned int &secuencia, unsigned int flujo );
    string PadZeros(int number, int longitud);
    int GenChecksum(string cadena);
    bool VerificaChecksum(string cadena);
    string File2String (string filename);

    //bool RecepcionPaquete(string Paquete, Package* package); //Recibe un paquete y devuelve si es final de flujo
    
};

string RDT::PadZeros(int number, int longitud)
{
  string num_letra = std::to_string(number);
  for (int i = num_letra.length(); i < longitud; ++i)
    num_letra = "0" + num_letra;
  
  return num_letra;
}

int RDT::GenChecksum(string cadena)
{// TODO
  return 0;
}


std::vector<string> RDT::PreparacionMensaje(string mensaje,
 unsigned int &secuencia, unsigned int flujo)
{
  //unsigned int sec_flujo = 0; //secuencia en flujo
  string flag_fin = "0";
  std::vector<string> vec_cadena;

  // i es secuencia en flujo
  for (int i = 0; i < mensaje.length(); i+=CT::max_size_msg)
  {
    string cadena = "";
    string temp = mensaje.substr(i, CT::max_size_msg);
    
    cadena += PadZeros(secuencia, CT::pos_secuencia) +
              PadZeros(flujo, CT::pos_flujo) +
              PadZeros(i, CT::pos_sec_en_flujo) +
              flag_fin +
              temp;
    cadena += std::to_string(GenChecksum(cadena));
    //Agregar la cadena a un map para su uso posterior
    //TODO Eliminar el vector y 
    //llevar control de numero secuencia inicial y final 
    //para recuperar cadena desde map
    map_secuencias[secuencia] = cadena;
    vec_cadena.push_back(cadena);
    secuencia++;
  }
  return vec_cadena;
}
/*
bool RDT::RecepcionPaquete(string paquete, Package* package)
{
  //TODO Verificar Checksum
  bool flag = true;
  package->secuence = stoi(paquete.substr(0, pos_secuencia));
  package->flujo = stoi(paquete.substr(pos_secuencia, pos_flujo));
  package->sec_flujo = stoi(paquete.substr(ind_sec_flujo, pos_sec_en_flujo));
  package->payload = paquete.substr(ind_sec_flujo+pos_sec_en_flujo);
  package->payload = package->payload.substr(0,package->payload.length()-1);
  return flag;
}
*/

string RDT::File2String (string filename)
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
