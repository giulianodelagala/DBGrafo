//Utilidades RDT

#include <iostream>
#include <string>
#include <vector>

using std::string;

class RDT
{
public:
    static const int pos_secuencia = 5; //primeros 5 bytes capa transporte
    static const int pos_flujo = 3;  //primeros 3 bytes capa aplicacion
    static const int pos_sec_en_flujo = 2; //secuencia dentro de flujo
    static const int divisor = 10; //para generación de checksum
    static const int max_size_msg = 500;

    static std::vector<string> PreparacionMensaje(string mensaje,
      unsigned int &secuencia, unsigned int flujo );
    static string PadZeros(int number, int longitud);
    static int GenChecksum(string cadena);
    static bool VerificaChecksum(string cadena);
    
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
  unsigned int sec_flujo = 0; //secuencia en flujo
  string flag_fin = "0";
  std::vector<string> vec_cadena;

  for (int i = 0; i < mensaje.length(); i+=max_size_msg)
  {
    string cadena = "";
    string temp = mensaje.substr(i, max_size_msg);
    secuencia++;
    cadena += PadZeros(secuencia, pos_secuencia) +
              PadZeros(flujo, pos_flujo) +
              PadZeros(sec_flujo, pos_sec_en_flujo) +
              flag_fin +
              temp;
    cadena += std::to_string(GenChecksum(cadena));

    vec_cadena.push_back(cadena);
  }

  return vec_cadena;
}
