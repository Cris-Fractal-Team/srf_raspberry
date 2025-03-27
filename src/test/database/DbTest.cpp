
#include <iostream>

#include "test/DbTest.h"
#include "lib/database/GDbTypes.h"
#include "lib/database/GDbTableConfig.h"
#include "lib/database/GDbConnection.h"
#include "lib/database/GMySqlConnection.h"

using namespace std;


/**
 * Constructor
 */
ClusterCarasDMAP::ClusterCarasDMAP()
{   
    setTableName("clustercaras");

    addStringCol("nombre", 100, true);
    addStringCol("tipo", 3, true);
    addStringCol("feccrea", 20, true);
    addStringCol("fecmod", 20, false);
    addPkAutoInc("idclustercaras");
    addLongCol("codusucrea", 10, true);
    addLongCol("codusumod", 10, true);
}

/**
 * Retorna el valor string de una columa dado su nombre
 *  columnName: nombre de la columna
 */
GDbType* ClusterCarasDMAP::get( string columnName )
{
    if ( columnName.compare("nombre") == 0 ) return &this->nombre;
    if ( columnName.compare("tipo") == 0 ) return &this->tipo;
    if ( columnName.compare("feccrea") == 0 ) return &this->feccrea;
    if ( columnName.compare("fecmod") == 0 ) return &this->fecmod;
    if ( columnName.compare("idclustercaras") == 0 ) return &this->idclustercaras;
    if ( columnName.compare("codusucrea") == 0 ) return &this->codusucrea;
    if ( columnName.compare("codusumod") == 0 ) return &this->codusumod;

    return NULL;
}

/**
 * Retorna una conexion con la BD
 */
GMySqlConnection *getConTest()
{
    GMySqlConnection *con;
    string msg = "Error al crear conexion";


    con = new GMySqlConnection();
    con->dbServer = "192.168.3.100";
    con->dbName = "easysoft_aforobd_2";
    con->dbUser = "easysoft";
    con->dbPassword = "123456";

    try
    {
        con->connect();
    }
    catch(const std::exception& e)
    {
        cout << "Error al crear conexion" << endl;
        delete con;
        throw e;
    }
        

    return con;
}

/**
 * Crea 20 conexiones a la BD
 * se emplea para validar si abrir o cerrar conexiones y borrar el objeto
 * no deja bloques de memoria sin liberar
 */
void testMultiplesConexiones()
{
    GMySqlConnection *con;
    string msg = "Error al crear conexion";

    cout << "Inicia test de multiples conexiones" << endl;

    for(int i=0; i<20; i++)
    {
        cout << "Iteracion " << i << endl;

        con = new GMySqlConnection();
        con->dbServer = "192.168.3.100";
        con->dbName = "easysoft_aforobd_2";
        con->dbUser = "easysoft";
        con->dbPassword = "123456";

        cout << "Antes de crear la conexion" << endl;
        try    
        {
            con->connect();
            cout << "Conexion creada" << endl;
            con->close();        
            cout << "Conexion cerrada" << endl;
        }
        catch(const std::exception& e)
        {
            cout << "Error al acer consulta SQL :" << msg << e.what() << endl;
        }
        
        delete con;
    }
}

/**
 * Prueba si se puede configurar una clase asociada a una tabla
 */
void testConfiguraTabla()
{
    ClusterCarasDMAP cara;

    cara.codusucrea.value = 100;
    cara.feccrea.value = "2022-09-17";
    cara.nombre.value = "Test 01";
    cara.tipo.value = "ACT";
}


/**
 * Prueba insertar un registro
 */
void testInsertaRegistro()
{
    ClusterCarasDMAP cara;
    GVector parametros;
    string msgError;
    long id;

    cout << "Validando insercion de registro " << endl;
    
    cara.codusucrea = 1234567890;
    cara.feccrea = "2022-09-17";
    cara.nombre = "Test 01";
    cara.tipo = "C";

    cout << "Valor del codusucrea : " << cara.codusucrea.value << endl;

    GMySqlConnection *con;

    try
    {
        con = getConTest();
        cout << "Estado de la conexion : " << con->isConnected() << endl;

        parametros.add(cara.codusucrea.getDbParam());
        parametros.add(cara.feccrea.getDbParam());
        parametros.add(cara.nombre.getDbParam());
        parametros.add(cara.tipo.getDbParam());
        
        string sql = "INSERT INTO clustercaras (codusucrea,feccrea,nombre,tipo) VALUES (?,?,?,?)";
        
        msgError = "Error al insertar el registro";
        id = con->executeSqlUpdate(sql, parametros); 

        cout << "Insercion realizada, ID del registro: " << id << endl;
    }
    catch(const std::exception& e)
    {
        cout << msgError << " : " << e.what() << endl;
    }

    if ( con != NULL )
    {        
        delete con;
    }
    cout << "Fin de test de insercion" << endl;
}

/**
 * Prueba insertar un registro
 */
void testActualizaRegistro()
{
    ClusterCarasDMAP cara;
    GVector parametros;
    string msgError;
    long id;

    cout << "Validando actualizacion de registro " << endl;

    cara.idclustercaras = 11;
    cara.codusucrea = 12345;
    cara.feccrea = "2022-09-10 22:00:05";
    cara.nombre = "Test 05";
    cara.tipo = "X";

    long long valor = cara.idclustercaras;
    cout << "Valor ID: " << valor << endl;

    GMySqlConnection *con;

    try
    {
        con = getConTest();
        cout << "Estado de la conexion : " << con->isConnected() << endl;

        parametros.add(cara.codusucrea.getDbParam());
        parametros.add(cara.feccrea.getDbParam());
        parametros.add(cara.nombre.getDbParam());
        parametros.add(cara.tipo.getDbParam());
        parametros.add(cara.idclustercaras.getDbParam());
        
        string sql = "UPDATE clustercaras SET codusucrea = ?, feccrea = ?, nombre = ?, tipo = ? ";
        sql.append("WHERE idclustercaras = ?");
        
        msgError = "Error al actualizar el registro";
        id = con->executeSqlUpdate(sql, parametros); 

        cout << "Actualizacion realizada, Filas actualizadas : " << id << endl;
    }
    catch(const std::exception& e)
    {
        cout << msgError << " : " << e.what() << endl;
    }

    if ( con != NULL )
    {        
        delete con;
    }
    cout << "Fin de test de insercion" << endl;
}

/**
 * Prueba insertar un registro sin generar codigo SQL
 * solo en base al analisis de sus datos
 */
void testInsertaRegSinSql() 
{
    ClusterCarasDMAP cara;
    string msgError;

    cout << "Validando creacion de registro sin SQL" << endl;

    cara.codusucrea = 26;
    cara.feccrea = "2022-09-20 22:00:05";
    cara.nombre = "Test 001";
    cara.tipo = "C";
    cara.fecmod.isNull = true;
    cara.codusumod.isNull = true;

    long long valor = cara.idclustercaras;
    cout << "Valor ID: " << valor << endl;

    GMySqlConnection *con;

    msgError = "No se pudo crear la conexion con la BD";
    try
    {
        cout << "Creando conexion" << endl;
        con = getConTest();
        cout << "Estado de la conexion : " << con->isConnected() << endl;

        msgError = "No se pudo insertar el reigstro";

        cout << "Antes de llamar a save" << endl;
        con->save(&cara,true);

        cout << "Registro creado con ID: " << cara.idclustercaras << endl;
    }
    catch( std::exception &e)
    {
        cout << msgError << " : " << e.what() << endl;
    }

    if ( con != NULL )
    {
        delete con;
    }
}

/**
 * Test para borrar un registro
 */
void testBorrarSinSql()
{
    ClusterCarasDMAP cara;
    string msgError;
    int num;

    cout << "Validando Borrado de registro sin SQL" << endl;

    cara.idclustercaras = 2;
    
    GMySqlConnection *con = NULL;

    msgError = "No se pudo crear la conexion con la BD";
    try
    {
        cout << "Creando conexion" << endl;
        con = getConTest();
        cout << "Estado de la conexion : " << con->isConnected() << endl;

        msgError = "No se pudo insertar el reigstro";

        cout << "Antes de llamar a save" << endl;
        num = con->erase(&cara);

        cout << "Registro borrado con ID: " << cara.idclustercaras << " Num Registros: " << num << endl;
    }
    catch( std::exception &e)
    {
        cout << msgError << " : " << e.what() << endl;
    }

    if ( con != NULL )
    {
        delete con;
    }
}


/**
 * Prueba acceso a la bd
 */
void testDb()
{
    GMySqlConnection *con;
    string msg = "Error al crear conexion";

    cout << "Antes de crear la conexion" << endl;
    try    
    {
        con = getConTest();
        cout << "Conexion creada" << endl;

        msg = "Error al insertar dato";
        testInsertaRegistro();

        con->close();        
        cout << "Conexion cerrada" << endl;
    }
    catch(const std::exception& e)
    {
        cout << "Error al acer consulta SQL :" << msg << e.what() << endl;
    }
    
    delete con;    
}

/**
 * Test ejecutando una consulta simple SQL
 */
void testConsultaSQL()
{    
    GVector parametros;
    GDbCursor cursor;
    string msgError;

    cout << "Validando consulta SQL simple " << endl;

    GMySqlConnection *con  = NULL;

    try
    {
        con = getConTest();
        cout << "Estado de la conexion : " << con->isConnected() << endl;

        parametros.add(new GDbInt(4));
        
        string sql = "SELECT * FROM clustercaras ";
        sql.append("WHERE idclustercaras = ?");
        
        msgError = "Error al hacer consulta SQL";
        cursor = con->executeSqlQuery(sql, parametros); 

        cout << "QUery ejecutado " << endl;
    }
    catch(const std::exception& e)
    {
        cout << msgError << " : " << e.what() << endl;
    }

    if ( con != NULL )
    {        
        delete con;
    }
    cout << "Fin de test de query SQL" << endl;
}