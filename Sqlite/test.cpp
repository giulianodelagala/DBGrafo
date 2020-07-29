#include "BD.h"

int main()
{ 
    DB Sql;

    /*
    {
        //exec(connDB,"update into nodo2 values(5,'alexander','pinto')");
        //exec(connDB,"update nodo2 set dni='felix' where id = 3");
        //exec(connDB,"delete from nodo2 where id = 5");
        execSELECT(connDB,"Nodos", " id ", "nombre = 'Maria'");
        execSELECT(connDB, "Nodos" );
        //CrearNodo(connDB, "Jose");
        //cout << SelectIdNodo(connDB, "Jose");
    }
    */

    //Sql.execSELECT("Nodos", " id ", "nombre = 'Jose'");
    //cout << Sql.GetID("Jose") << "\n";
    //Sql.InsertAtributo("Alberto", "Direccion", "Calle Bolivar 14");
    //Sql.InsertRelacion("Ana", "Alberto");
    /*
    vector<Atr> vec_atri;
    vec_atri = Sql.ReadAtributosNodo("Alberto");

    for (auto i: vec_atri)
    {
        cout << i.value;
    }
    */
   /*
    vector<int> vec_friends;
    vec_friends = Sql.GetIDFriends("Ana");
    for (auto i: vec_friends)
    {
        cout << i;
    }
    */

    //Sql.UpdateAtributo("Ana", "Edad", "20");

    //Sql.DeleteNodo("Maria");
    //Sql.DeleteAtributo("Alberto","Direccion");
    Sql.DeleteRelacion ("Ana", "Jose");

    return 0;

}