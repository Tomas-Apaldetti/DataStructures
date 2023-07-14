
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "estructuras.h"
#include "hash.h"
#include "lista.h"

#define BASE_HASHING 4079 // Numero Primo de base hash
#define PRIMO_UNO    439  // Primer primo para hashing
#define PRIMO_DOS    829  // Segundo primo para hashing

#define FACTOR_CARGA 0.75
#define RATIO        2

#define ERROR -1
#define OK    0

/*
 * Dado un numero, devuelve true si es primo, false si no lo es.
 */
static bool es_primo(size_t numero)
{
    if (numero == 1)
        return false;
    if (numero == 2 || numero == 3)
        return true;
    if (numero % 2 == 0 || numero % 3 == 0)
        return false;
    for (size_t i = 5; i * i <= numero; i += 6)
        if (numero % i == 0 || numero % (i + 2) == 0)
            return false;
    return true;
}
/*
 * Devuelve el siguiente numero primero al numero dado
 */
static size_t siguiente_primo(size_t numero)
{
    if (es_primo(numero))
        return numero;
    return siguiente_primo(++numero);
}

/*
 * Dada una clave, devuelve la posicion en el hash segun el modulo pasado.
 */
static size_t obtener_hash(const char* clave)
{
    if (!clave)
        return 0;
    size_t codigo_hash = BASE_HASHING;
    while (*clave)
    {
        codigo_hash = (codigo_hash * PRIMO_UNO) ^ ((size_t)clave[0] * PRIMO_DOS);
        clave++;
    }
    return codigo_hash;
}

/*
 * Duplica una string y la guarda en el heap.
 *
 * Devuelve un puntero al string o NULL en caso de error.
 */
static char* duplicar_string(const char* string)
{
    if (!string)
        return NULL;
    char* copia = malloc(strlen(string) + 1);
    if (!copia)
        return NULL;
    strcpy(copia, string);
    return copia;
}

/*
 * Crea un dato con el elemento a guardar y su clave.
 *
 * Devuelve un puntero a un dato, o NULL si no se pudo crear.
 */
static dato_t* crear_dato(const char* clave, void* elemento)
{
    if (!clave)
        return NULL;
    dato_t* dato = malloc(sizeof(dato_t));
    if (!dato)
        return NULL;
    char* copia_clave = duplicar_string(clave);
    if (!copia_clave)
    {
        free(dato);
        return NULL;
    }
    dato->clave = copia_clave;
    dato->elemento = elemento;
    return dato;
}
/*
 * Compara un dato con una clave.
 *
 * Devuelve 1/-1 si son diferentes, 0 si son iguales.
 */
static int comparar_dato(void* dato, void* clave)
{
    if (!dato || !clave)
        return ERROR;
    return strcmp(((dato_t*)dato)->clave, (const char*)clave);
}

/*
 * Destruye un dato
 */
static void destruir_dato(void* dato)
{
    if (dato)
        free(((dato_t*)dato)->clave);
    free(dato);
}

/*
 * Crea el hash reservando la memoria necesaria para el.
 * Destruir_elemento es un destructor que se utilizará para liberar
 * los elementos que se eliminen del hash.  Capacidad indica la
 * capacidad inicial con la que se crea el hash. La capacidad inicial
 * no puede ser menor a 3. Si se solicita una capacidad menor, el hash
 * se creará con una capacidad de 3.
 *
 * Devuelve un puntero al hash creado o NULL en caso de no poder
 * crearlo.
 */
hash_t* hash_crear(hash_destruir_dato_t destruir_elemento, size_t capacidad)
{
    hash_t* hash = calloc(1, sizeof(hash_t));
    if (!hash)
        return NULL;
    hash->capacidad = siguiente_primo(capacidad < 3 ? 3 : capacidad);
    hash->destructor = destruir_elemento;
    hash->vector = calloc(hash->capacidad, sizeof(lista_t*));
    if (!hash->vector)
    {
        free(hash);
        return NULL;
    }
    return hash;
}

/*
 * Busca en el vector del hash, en la lista de la posicion dada, la clave.
 *
 * Devuelve un puntero al dato si existia en la lista con el valor de la clave, o NULL en caso de no
 * existir o en caso de error.
 */
static dato_t* hash_buscar(hash_t* hash, size_t valor_hash, const char* clave)
{
    if (!hash || !hash->vector[valor_hash])
        return NULL;
    return lista_buscar(hash->vector[valor_hash], (void*)clave);
}

/*
 * Destruye un vector de hash que contenga listas, si se le pasa un destructor, se utiliza para
 * eliminar los elementos guardados.
 */
static void destruir_vector_hash(lista_t** vector, size_t capacidad,
                                 hash_destruir_dato_t destructor)
{
    if (!vector)
        return;
    dato_t* dato = NULL;
    for (size_t i = 0; i < capacidad; i++)
        if (vector[i])
        {
            while (!lista_vacia(vector[i]))
            {
                dato = lista_elemento_en_posicion(vector[i], 0);
                if (dato && destructor)
                    destructor(dato->elemento);
                lista_borrar_de_posicion(vector[i], 0);
            }
            lista_destruir(vector[i]);
        }
    free(vector);
}

/*
 * Deshace el proceso de rehash, asignandole al hash pasado el vector, la capacidad, y la cantidad
 * pasada, y elimina el vector actual.
 *
 */
static void deshacer_rehash(hash_t* hash, lista_t** vector, size_t capacidad, size_t cantidad)
{
    if (!hash)
        return;
    destruir_vector_hash(hash->vector, hash->capacidad, hash->destructor);
    hash->vector = vector;
    hash->capacidad = capacidad;
    hash->cantidad = cantidad;
}

/*
 * Rehashea una lista(una posicion) del hash, reinsertando cada elemento en el hash, en su nueva
 * posicion.
 *
 * Devuelve 0 si se pudo rehashear la lista correctamente, -1 si ocurrio algun error en el proceso.
 */
static int rehash_lista(hash_t* hash, lista_t* a_rehash)
{
    if (!hash || !a_rehash)
        return ERROR;

    lista_iterador_t* iterador = lista_iterador_crear(a_rehash);
    if (!iterador)
        return ERROR;

    dato_t* dato = NULL;
    while (lista_iterador_tiene_siguiente(iterador))
    {
        dato = lista_iterador_siguiente(iterador);
        if (dato && hash_insertar(hash, dato->clave, dato->elemento) == ERROR)
        {
            lista_iterador_destruir(iterador);
            return ERROR;
        }
    }
    lista_iterador_destruir(iterador);
    return OK;
}

/*
 * Realiza la operacion de rehash.
 * La nueva capacidad sera el siguiente primo a (vieja capacidad*RATIO).
 *
 * En caso de poder rehashear, devuelve 0 y elimina el viejo vector junto con las listas.
 * En caso de algun error en el proceso de rehasheo, devuelve -1 dejando el hash como antes del
 * proceso de rehash.
 */
static int rehash(hash_t* hash)
{
    size_t    capacidad_vieja = hash->capacidad;
    lista_t** viejo = hash->vector;
    size_t    cantidad_vieja = hash->cantidad;

    size_t    nueva_capacidad = siguiente_primo(hash->capacidad * RATIO);
    lista_t** nuevo = calloc(nueva_capacidad, sizeof(lista_t*));
    if (!nuevo)
        return ERROR;

    hash->vector = nuevo;
    hash->capacidad = nueva_capacidad;
    hash->cantidad = 0;

    for (size_t i = 0; i < capacidad_vieja; i++)
        if (viejo[i] && rehash_lista(hash, viejo[i]) == ERROR)
        {
            deshacer_rehash(hash, viejo, capacidad_vieja, cantidad_vieja);
            return ERROR;
        }
    destruir_vector_hash(viejo, capacidad_vieja, NULL);
    return OK;
}

/*
 * Inserta un dato en la posicion del hash dada.
 * Crea una lista con el comparador y el destructor de datos, si es necesario.
 *
 * Devuelve 0 si se pudo insertar, -1 en caso de error.
 */
static int insertar(hash_t* hash, dato_t* a_guardar, size_t posicion)
{
    if (!hash || !a_guardar || !hash->vector)
        return ERROR;
    if (!(hash->vector[posicion]))
    {
        lista_t* lista = lista_crear(comparar_dato, destruir_dato);
        if (!lista)
            return ERROR;
        hash->vector[posicion] = lista;
    }
    return lista_insertar(hash->vector[posicion], a_guardar);
}

/*
 * Reemplaza, si existe un dato en el hash con la clave pasada, el elemento que este almacena.
 * En caso de reemplazar destruye el elemento que reemplaza.
 *
 * Devuelve 0 si se reemplazo el dato, -1 si no se pudo reemplazar.
 */
static int reemplazar(hash_t* hash, const char* clave, void* elemento, size_t valor_hash)
{
    if (!hash || !clave)
        return ERROR;
    dato_t* dato = hash_buscar(hash, valor_hash, clave);
    if (!dato)
        return ERROR;
    if (hash->destructor)
        hash->destructor(dato->elemento);
    dato->elemento = elemento;
    return OK;
}

/*
 * Inserta un elemento en el hash asociado a la clave dada.
 *
 * Nota para los alumnos: Recordar que si insertar un elemento provoca
 * que el factor de carga exceda cierto umbral, se debe ajustar el
 * tamaño de la tabla para evitar futuras colisiones.
 *
 * Devuelve 0 si pudo guardarlo o -1 si no pudo.
 */
int hash_insertar(hash_t* hash, const char* clave, void* elemento)
{
    if (!hash || !clave || !hash->vector) // Un hash valido deberia tener siempre un vector.
        return ERROR;
    size_t valor_hash = obtener_hash(clave) % hash->capacidad;

    if (reemplazar(hash, clave, elemento, valor_hash) == OK)
        return OK;

    dato_t* a_guardar = crear_dato(clave, elemento);
    if (!a_guardar)
        return ERROR;

    if (insertar(hash, a_guardar, valor_hash) == ERROR)
    {
        destruir_dato(a_guardar);
        return ERROR;
    }
    hash->cantidad++;
    double carga = (double)hash->cantidad / (double)hash->capacidad;
    if (carga > FACTOR_CARGA)
        return rehash(hash);
    return OK;
}

/*
 * Quita un elemento del hash e invoca la funcion destructora
 * pasandole dicho elemento.
 * Devuelve 0 si pudo eliminar el elemento o -1 si no pudo.
 */
int hash_quitar(hash_t* hash, const char* clave)
{
    if (!hash || !clave)
        return ERROR;
    size_t valor_hash = obtener_hash(clave) % hash->capacidad;

    if (!(hash->vector[valor_hash]))
        return ERROR;

    lista_iterador_t* iterador = lista_iterador_crear(hash->vector[valor_hash]);
    if (!iterador)
        return ERROR;
    dato_t* dato = NULL;
    bool    continuar = true;
    for (size_t i = 0; lista_iterador_tiene_siguiente(iterador) && continuar; i++)
    {
        dato = lista_iterador_siguiente(iterador);
        if (dato && comparar_dato(dato, (void*)clave) == 0)
        {
            if (hash->destructor)
                hash->destructor(dato->elemento);
            lista_borrar_de_posicion(hash->vector[valor_hash], i);
            continuar = false;
        }
    }
    lista_iterador_destruir(iterador);

    if (lista_vacia(hash->vector[valor_hash]))
    {
        lista_destruir(hash->vector[valor_hash]);
        hash->vector[valor_hash] = NULL;
    }

    hash->cantidad--;
    return OK;
}

/*
 * Devuelve un elemento del hash con la clave dada o NULL si dicho
 * elemento no existe (o en caso de error).
 */
void* hash_obtener(hash_t* hash, const char* clave)
{
    if (!hash || !clave)
        return NULL;
    size_t  valor_hash = obtener_hash(clave) % hash->capacidad;
    dato_t* dato = hash_buscar(hash, valor_hash, clave);
    if (!dato)
        return NULL;
    return dato->elemento;
}

/*
 * Devuelve true si el hash contiene un elemento almacenado con la
 * clave dada o false en caso contrario (o en caso de error).
 */
bool hash_contiene(hash_t* hash, const char* clave)
{
    if (hash_obtener(hash, clave) == NULL)
        return false;
    return true;
}

/*
 * Devuelve la cantidad de elementos almacenados en el hash o 0 en
 * caso de error.
 */
size_t hash_cantidad(hash_t* hash)
{
    if (hash)
        return hash->cantidad;
    return 0;
}

/*
 * Destruye el hash liberando la memoria reservada y asegurandose de
 * invocar la funcion destructora con cada elemento almacenado en el
 * hash.
 */
void hash_destruir(hash_t* hash)
{
    if (!hash)
        return;
    destruir_vector_hash(hash->vector, hash->capacidad, hash->destructor);
    free(hash);
}

/*
 * Recorre cada una de las claves almacenadas en la tabla de hash e
 * invoca a la función funcion, pasandole como parámetros el hash, la
 * clave en cuestión y el puntero auxiliar.
 *
 * Mientras que queden mas claves o la funcion retorne false, la
 * iteración continúa. Cuando no quedan mas claves o la función
 * devuelve true, la iteración se corta y la función principal
 * retorna.
 *
 * Devuelve la cantidad de claves totales iteradas (la cantidad de
 * veces que fue invocada la función) o 0 en caso de error.
 *
 */
size_t hash_con_cada_clave(hash_t* hash,
                           bool (*funcion)(hash_t* hash, const char* clave, void* aux), void* aux)
{
    size_t iterados = 0;
    if (!hash || !funcion)
        return iterados;
    for (size_t i = 0; i < hash->capacidad; i++)
    {
        if (hash->vector[i])
        {
            lista_iterador_t* iterador = lista_iterador_crear(hash->vector[i]);
            if (!iterador)
                return 0;
            dato_t* dato = NULL;
            while (lista_iterador_tiene_siguiente(iterador))
            {
                dato = lista_iterador_siguiente(iterador);
                iterados++;
                if (dato && funcion(hash, dato->clave, aux))
                {
                    lista_iterador_destruir(iterador);
                    return iterados;
                }
            }
            lista_iterador_destruir(iterador);
        }
    }
    return iterados;
}
