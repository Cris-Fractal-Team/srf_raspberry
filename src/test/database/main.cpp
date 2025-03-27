
#include <iostream>
#include "test/DbTest.h"


using namespace std;

int main( int argc, char* argv[] )
{                    
    // desactiva el error de pipe
    cout << "*** Inicia ejecucion de test Database" << endl;
    
    // testMultiplesConexiones();

    // testConfiguraTabla();

    // testInsertaRegistro();

    // testActualizaRegistro();

    // testInsertaRegSinSql();

    //for(int i=0;i<10;i++)
    //{
    //    testBorrarSinSql();
    //}    

    // testDb();

    testConsultaSQL();
    
    return 0;
}

