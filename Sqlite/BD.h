#include <string>
#include <iostream>
#include <sqlite3.h>

using std::string;
using std::cout; using std::endl;

class BD
{
private:
    /* data */
public:
    sqlite3 *connDB;
    sqlite3_stmt *stmt;

    bool execSELECT(string &response, string table, string columns);
    bool execCREATE(string table, string object);
    bool execUPDATE(string table, string column, string object, string condition);
    bool execDELETE(string table, string condition);
    void exec(const char* sql);

    BD(string db_file);
    ~BD();
};

BD::BD(string db_file)
{
    const char* db_file_ = db_file.c_str();
    if (sqlite3_open(db_file_, &connDB) == SQLITE_OK)
    {
        //exec(connDB,"update into nodo2 values(5,'alexander','pinto')");
        //exec(connDB,"update nodo2 set dni='felix' where id = 3");
        //exec(connDB,"delete from nodo2 where id = 5");
        //execSELECT(connDB,"nodo2");
        cout << "conexion ok: " << db_file <<"\n";
    }
    else
        cout << "Failed to open db\n";
}

BD::~BD()
{
    sqlite3_close(connDB);
}

bool BD::execSELECT(string &response, string table, string columns = " *")
{
    sqlite3_stmt *stmt;
    string query = "SELECT " + columns + " FROM " + table;
    const char *sql = query.c_str();
    if (sqlite3_prepare_v2(connDB, sql , strlen(sql), &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            response +=  std::to_string(id) += " ";
            for (int i = 1; i < sqlite3_column_count(stmt); i++ )
            {
                const unsigned char* dato = sqlite3_column_text(stmt, i);
                string dato_s(reinterpret_cast<const char*>(dato));
                response += dato_s + " ";
            }
            response += "\n"; 
        }
        sqlite3_finalize(stmt);
        return true; // << "successfully query \n";
    }
    else
    {
        sqlite3_finalize(stmt);
        return false;
    }
}

void BD::exec(const char* sql)
{
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(connDB, sql, strlen(sql), &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_step(stmt);
    }
    else
        cout << "Error exec: " << sqlite3_errmsg(connDB) << endl;
    sqlite3_finalize(stmt);
}