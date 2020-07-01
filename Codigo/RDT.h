//Utilidades RDT

#include <iostream>
#include <string>

using std::string;

class RDT
{
public:
    static const int pos_secuencia = 5; //primeros 5 bytes capa transporte
    static const int pos_flujo = 3;  //primeros 3 bytes capa aplicacion
    static const int pos_sec_en_flujo = 2; //secuencia dentro de flujo
    static const int divisor = 10; //para generación de checksum

    static void PreparacionMensaje(string mensaje, unsigned int max_size_msg );
    static string PadZeros(int number, int longitud);
    static int GenChecksum(string cadena);
    static bool VerificaChecksum(string cadena)
    
};

string RDT::PadZeros(int number, int longitud)
{
  string num_letra = std::to_string(number);
  for (int i = num_letra.length(); i < longitud; ++i)
    num_letra = "0" + num_letra;
  
  return num_letra;
}



void RDT::PreparacionMensaje(string mensaje, unsigned int max_size_msg )
{
    std::cout << pos_secuencia + pos_flujo;

}
