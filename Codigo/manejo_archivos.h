//Manejo de Archivos
#include <iostream>
#include <sstream> //stringstream
#include <string>
#include <fstream>

using std::string;

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

void String2Txt (string filename, string mensaje)
{
  std::ofstream ofs;
  ofs.open (filename, std::ofstream::out | std::ofstream::app);
  ofs << mensaje;
  ofs.close();
}

void String2File (string filename, string mensaje)
{
  std::ofstream ofs;
  ofs.open (filename, std::ofstream::out | std::ios_base::binary);
  ofs.write(mensaje.c_str(), mensaje.size());
  ofs.close();
}