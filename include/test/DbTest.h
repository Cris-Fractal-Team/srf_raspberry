
#include "lib/database/GDbConnection.h"
#include "lib/database/GDbObject.h"
#include "lib/database/GDbTypes.h"

class ClusterCarasDMAP : public GDbObject
{
    public:

        GDbLong idclustercaras;

        GDbLong codusucrea;

        GDbString feccrea;

        GDbLong codusumod;

        GDbString fecmod;

        GDbString nombre;

        GDbString tipo;

        /**
         * Constructor
         */
        ClusterCarasDMAP();
        
        /**
         * Retorna el valor string de una columa dado su nombre
         *  columnName: nombre de la columna
         */
        virtual GDbType* get( string columnName );

};

/**
 * Crea 20 conexiones a la BD
 * se emplea para validar si abrir o cerrar conexiones y borrar el objeto
 * no deja bloques de memoria sin liberar
 */
void testMultiplesConexiones();

/**
 * Prueba si se puede configurar una clase asociada a una tabla
 */
void testConfiguraTabla();

/**
 * Prueba insertar un registro
 */
void testInsertaRegistro();

/**
 * Prueba insertar un registro
 */
void testActualizaRegistro();


/**
 * Prueba insertar un registro sin generar codigo SQL
 * solo en base al analisis de sus datos
 */
void testInsertaRegSinSql() ;

/**
 * Test para borrar un registro
 */
void testBorrarSinSql();

/**
 * Test ejecutando una consulta simple SQL
 */
void testConsultaSQL();


void testDb();
