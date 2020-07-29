#include <iostream>
#include <string.h>
#include <sqlite3.h>
#include <vector>

using std::string; using std::to_string;
using std::cout;
using std::vector;

struct Atr
{
    int id;
    string atrib;
    string value;

    Atr(int id, string atrib, string value):
    id(id),
    atrib(atrib),
    value(value) 
    {}
};


class DB
{
private:
    sqlite3* connDB;
public:

    bool exec(const char* sql);
    void execSELECT(string table, string columns, string where);
    
    bool CrearNodo(string nombre_nodo);
    int GetID(string nombre_nodo);
    bool InsertAtributo(string nombre_nodo, string edad, string direccion);
    bool InsertRelacion(string nombre_from, string nombre_to);
    vector<Atr> ReadAtributosNodo(string nombre_nodo);
    vector<int> GetIDFriends(string nombre_nodo);

    bool UpdateAtributo(string nombre_nodo, string atributo, string valor);

    bool DeleteNodo(string nombre_nodo);
    bool DeleteAtributo(string nombre_nodo, string atributo);
    bool DeleteRelacion(string nombre_from, string nombre_to);

    DB()
    {
        if (sqlite3_open("./test.db", &connDB) == SQLITE_OK)
            cout << "Conexion a DB exitosa\n";
        else
            cout << "Failed to open db\n";
    };
    ~DB()
    {
        sqlite3_close(connDB);
    };
};

////////////////////////
/// CREATE ///////////
////////////////////////

void DB::execSELECT(string table, string columns = " *", string where = "")
{
    sqlite3_stmt *stmt;
    string query = "SELECT " + columns + " FROM '" + table + "' WHERE " + where +";";
    //cout << query;
    const char *sql = query.c_str();
    if (sqlite3_prepare_v2(connDB, sql , strlen(sql), &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            cout << id << " ";
            for (int i = 1; i < sqlite3_column_count(stmt); i++ )
            {
                const unsigned char* dato = sqlite3_column_text(stmt, i);
                cout << dato << " ";
            }
            cout << "\n";
        }
        cout << "successfully query \n";
    }
    else
        cout << "Error execSelect: " << sqlite3_errmsg(connDB) << "\n";
    sqlite3_finalize(stmt);
}

//Creacion de Nodo
bool DB::CrearNodo(string nombre_nodo)
{
    string query = "INSERT INTO NODOS ('NOMBRE') VALUES ('" + nombre_nodo + "');";
    return exec(query.c_str()); 
}

//Get ID de Nodo a partir de su nombre
int DB::GetID(string nombre_nodo)
{
    sqlite3_stmt *stmt;
    string query = "SELECT id FROM 'NODOS' WHERE NOMBRE='" + nombre_nodo +"';";
    //cout << query;
    const char *sql = query.c_str();
    if (sqlite3_prepare_v2(connDB, sql , strlen(sql), &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            return id;
        }
    }
    else
        cout << "Error execSelect: " << sqlite3_errmsg(connDB) << "\n";
    sqlite3_finalize(stmt);
    return -1;
}

bool DB::InsertAtributo(string nombre_nodo, string atributo, string valor)
{
    string id_nodo = to_string(GetID(nombre_nodo));

    string query = "INSERT INTO NODOATRIBUTOS ('ID_NODO','ATRIBUTO','VALOR') VALUES (" + id_nodo + ",'" + atributo + "','" + valor + "');";
    cout << query;

    return exec(query.c_str()); 
}

bool DB::InsertRelacion(string nombre_from, string nombre_to)
{
    string id_nodo_from = to_string(GetID(nombre_from));
    string id_nodo_to = to_string(GetID(nombre_to));

    string query = "INSERT INTO RELACION ('ID_NODO_FROM','ID_NODO_TO') VALUES (" + id_nodo_from + "," + id_nodo_to + ");";
    cout << query;

    return exec(query.c_str()); 
}

bool DB::exec(const char* sql)
{
    sqlite3_stmt *stmt;
    bool result = false;
    if (sqlite3_prepare_v2(connDB, sql, strlen(sql), &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_step(stmt);
        result = true;
    }
    else
    {
        cout << "Error exec: " << sqlite3_errmsg(connDB) << "\n";
        result = false;
    }
    sqlite3_finalize(stmt);
    return result;
}

////////////////////////
/// READ ///////////
////////////////////////

vector<Atr> DB::ReadAtributosNodo(string nombre_nodo)
{
    vector<Atr> vec_atributos;
    string id_nodo = to_string(GetID(nombre_nodo));
    
    sqlite3_stmt *stmt;
    string query = "SELECT * FROM 'NODOATRIBUTOS' WHERE ID_NODO=" + id_nodo +";";
    //cout << query;
    const char *sql = query.c_str();
    if (sqlite3_prepare_v2(connDB, sql , strlen(sql), &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            string tipo; tipo.assign((char*) sqlite3_column_text(stmt, 2));
            string valor; valor.assign((char*)sqlite3_column_text(stmt, 3));
            Atr atributo(id, tipo, valor);

            vec_atributos.push_back(atributo);
        }
        cout << "successfully query \n";
    }
    else
        cout << "Error execSelect: " << sqlite3_errmsg(connDB) << "\n";
    sqlite3_finalize(stmt);
    return vec_atributos;
}

vector<int> DB::GetIDFriends(string nombre_nodo)
{
    vector<int> vec_friends;
    string id_nodo = to_string(GetID(nombre_nodo));
    
    sqlite3_stmt *stmt;
    string query = "SELECT * FROM 'RELACION' WHERE ID_NODO_FROM=" + id_nodo +";";
    //cout << query;
    const char *sql = query.c_str();
    if (sqlite3_prepare_v2(connDB, sql , strlen(sql), &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 2);
            vec_friends.push_back(id);
        }
        cout << "successfully query \n";
    }
    else
        cout << "Error execSelect: " << sqlite3_errmsg(connDB) << "\n";
    sqlite3_finalize(stmt);
    return vec_friends;
}

////////////////////////
/// UPDATE ///////////
////////////////////////

bool DB::UpdateAtributo(string nombre_nodo, string atributo, string valor)
{
    string id_nodo = to_string(GetID(nombre_nodo));

    string query = "UPDATE NODOATRIBUTOS SET VALOR = " + valor + " WHERE ID_NODO = " + id_nodo + " AND ATRIBUTO = \"" + atributo + "\"";
    cout << query;

    return exec(query.c_str()); 
}

////////////////////////
/// DELETE ///////////
////////////////////////

bool DB::DeleteNodo(string nombre_nodo)
{
    //string id_nodo = to_string(GetID(nombre_nodo));

    string query = "DELETE FROM NODOS WHERE NOMBRE = \"" + nombre_nodo + "\"";
    cout << query;

    return exec(query.c_str()); 
}

bool DB::DeleteAtributo(string nombre_nodo, string atributo)
{
    string id_nodo = to_string(GetID(nombre_nodo));

    string query = "DELETE FROM NODOATRIBUTOS WHERE ID_NODO = " + id_nodo + " AND ATRIBUTO = \"" + atributo + "\"";
    cout << query;

    return exec(query.c_str()); 
}

bool DB::DeleteRelacion(string nombre_from, string nombre_to)
{
    string id_nodo_from = to_string(GetID(nombre_from));
    string id_nodo_to = to_string(GetID(nombre_to));

    string query = "DELETE FROM RELACION WHERE ID_NODO_FROM = " + id_nodo_from + " AND ID_NODO_TO = " + id_nodo_to;
    cout << query;

    return exec(query.c_str()); 
}