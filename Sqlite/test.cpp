#include <iostream>
#include <string.h>
//#include <sqlite3.h>
#include "BD.h"

using namespace std;


int main()
{
    string response = "";
    BD Test("test.db");
    Test.execSELECT(response, "nodo2");
    //cout << response;
    return 0;
}