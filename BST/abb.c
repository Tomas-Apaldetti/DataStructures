#include <stdbool.h>
#include <stdlib.h>

#include "abb.h"

/*
 * Crea el arbol y reserva la memoria necesaria de la estructura.
 * Comparador se utiliza para comparar dos elementos.
 * Destructor es invocado sobre cada elemento que sale del arbol,
 * puede ser NULL indicando que no se debe utilizar un destructor.
 *
 * Devuelve un puntero al arbol creado o NULL en caso de error.
 */
abb_t* arbol_crear(abb_comparador comparador, abb_liberar_elemento destructor)
{
    if (!comparador)
        return NULL;
    abb_t* arbol = calloc(1, sizeof(abb_t));
    if (!arbol)
        return NULL;
    arbol->comparador = comparador;
    arbol->destructor = destructor;
    return arbol;
}

/*
 * Crea un nuevo nodo y le asigna el elemento que guarda,
 * asi como NULL a ambas ramas.
 *
 * Devuelve NULL en caso de no poder crear.
 */
static nodo_abb_t* nuevo_nodo(void* elemento)
{
    nodo_abb_t* nodo = calloc(1, sizeof(nodo_abb_t));
    if (!nodo)
        return NULL;
    nodo->elemento = elemento;
    return nodo;
}

/*
 * Inserta un nodo recursivamente.
 * Recibe un nodo,
 * un puntero a un comparador,
 * el elemento a insertar
 *
 * La funcion deja de llamarse recursivamente cuando el nodo sea NULL.
 *
 * Devuelve un arbol con el elemento insertado en el lugar que corresponde.
 * Devuelve NULL en caso de fallar al crear el nodo.
 */
static nodo_abb_t* insertar(nodo_abb_t* nodo, abb_comparador comparador, void* elemento)
{
    if (!comparador) // Sin comparador, no entra a llamarse a si misma.
        return NULL;
    if (!nodo) // Nodo NULL, estoy en donde deberia ir el nodo
    {
        nodo_abb_t* n_nodo = nuevo_nodo(elemento);
        if (n_nodo)
            return n_nodo;
        return NULL;
    }
    // El elemento a insertar es menor al que estoy ahora. Evaluo la rama izquierda del nodo actual.
    if (comparador(elemento, nodo->elemento) == -1)
    {
        nodo_abb_t* aux = insertar(nodo->izquierda, comparador, elemento);
        if (!aux)
            return NULL;
        else
            nodo->izquierda = aux;
    }
    // El elemento a insertar es mayor o igual al que estoy hora. Evaluo la rama derecha del nodo
    // actual
    else
    {
        nodo_abb_t* aux = insertar(nodo->derecha, comparador, elemento);
        if (!aux)
            return NULL;
        else
            nodo->derecha = aux;
    }
    return nodo;
}

/*
 * Inserta un elemento en el arbol.
 * Devuelve 0 si pudo insertar o -1 si no pudo.
 * El arbol admite elementos con valores repetidos.
 */
int arbol_insertar(abb_t* arbol, void* elemento)
{
    if (!arbol)
        return -1;
    nodo_abb_t* auxiliar = insertar(arbol->nodo_raiz, arbol->comparador, elemento);
    if (!auxiliar)
        return -1;
    arbol->nodo_raiz = auxiliar;
    return 0;
}

/*
 * Utiliza el destructor en caso de existir.
 */
static void destruir_elemento(void* elemento, abb_liberar_elemento destructor)
{
    if (destructor)
        destructor(elemento);
}

/*
 * Libera un nodo, utilizando el destructor.
 */
static void liberar_nodo(nodo_abb_t* nodo, abb_liberar_elemento destructor)
{
    destruir_elemento(nodo->elemento, destructor);
    free(nodo);
}

/*
 * Busca y elimina el nodo mas a la derecha de un arbol.
 * Guarda en elemento, el dato del nodo eliminado.
 * Devuelve el hijo izquierdo del nodo eliminado.
 */
static nodo_abb_t* predecesor_inorden(nodo_abb_t* nodo, void** elemento)
{
    if (!nodo->derecha)
    {
        *elemento = nodo->elemento;
        nodo_abb_t* auxiliar = nodo->izquierda;
        free(nodo);
        return auxiliar;
    }
    nodo->derecha = predecesor_inorden(nodo->derecha, elemento);
    return nodo;
}

/*
 * Funcion recursiva para eliminar un elemento del arbol.
 * Recibe un nodo de un arbol valido,
 * una funcion destructura(opcional),
 * una funcion comparadora,
 * el elemento a borrar,
 * y un puntero a un bool, el cual debe ser false en la primera llamada(En caso de ser true en la
 * primera llamada, la funcion devolvera siempre NULL)
 *
 * Devuelve el arbol con el elemento borrado, en caso de encontrarlo en el arbol.
 * En caso de no encontrar el elemento, se cambia la bandera de no_encontre, haciendo que las
 * llamadas devuelvan NULL
 */
static nodo_abb_t* borrar(nodo_abb_t* nodo, abb_liberar_elemento destructor,
                          abb_comparador comparador, void* elemento, bool* no_encontre)
{
    if (!comparador) // No puedo comparar, imposible eliminar. Nunca va a entrar en la recursividad
        return NULL;
    if (!nodo) // Se llego al final del recorrido, no se encontro un elemento a buscar.
    {
        *no_encontre = true;
        return NULL;
    }
    // El elemento a buscar es menor, pongo en evaluacion la rama izquierda
    if (comparador(elemento, nodo->elemento) == -1)
    {
        nodo_abb_t* aux = borrar(nodo->izquierda, destructor, comparador, elemento, no_encontre);
        if (*no_encontre)
            return NULL;
        nodo->izquierda = aux;
    }
    // El elemento a buscar es mayor, pongo en evaluacion la rama derecha
    else if (comparador(elemento, nodo->elemento) == 1)
    {
        nodo_abb_t* aux = borrar(nodo->derecha, destructor, comparador, elemento, no_encontre);
        if (*no_encontre)
            return NULL;
        nodo->derecha = aux;
    }
    // Es el nodo a eliminar, busco cuantos hijos tiene
    else
    {
        // Tiene un solo hijo, libero el nodo, devuelvo el hijo contrario(puede o no ser NULL)
        if (!nodo->izquierda)
        {
            nodo_abb_t* temp = nodo->derecha;
            liberar_nodo(nodo, destructor);
            return temp;
        }
        else if (!nodo->derecha)
        {
            nodo_abb_t* temp = nodo->izquierda;
            liberar_nodo(nodo, destructor);
            return temp;
        }
        // Tiene dos hijos, busco el predecesor, y hago el cambio de hijos y elementos
        // correspondiente.
        void* elemento_predecesor = NULL;
        nodo->izquierda = predecesor_inorden(nodo->izquierda, &elemento_predecesor);
        destruir_elemento(nodo->elemento, destructor);
        nodo->elemento = elemento_predecesor;
    }
    return nodo;
}

/*
 * Busca en el arbol un elemento igual al provisto (utilizando la
 * funcion de comparación) y si lo encuentra lo quita del arbol.
 * Adicionalmente, si encuentra el elemento, invoca el destructor con
 * dicho elemento.
 * Devuelve 0 si pudo eliminar el elemento o -1 en caso contrario.
 */
int arbol_borrar(abb_t* arbol, void* elemento)
{
    if (!arbol)
        return -1;
    bool        flag = false;
    nodo_abb_t* aux =
        borrar(arbol->nodo_raiz, arbol->destructor, arbol->comparador, elemento, &flag);
    if (!aux && flag)
        return -1;
    arbol->nodo_raiz = aux;
    return 0;
}

/*
 * Funcion recursiva para buscar nodo.
 *
 * Devuelve el primer elemento en el arbol que sea igual al elemento
 * a buscar,o NULL en caso de el elemento a buscar no existe en el arbol
 */
static void* buscar(nodo_abb_t* nodo, abb_comparador comparador, void* elemento)
{
    // LLego al final de la rama, no encontro el elemento
    if (!nodo)
        return NULL;
    if (comparador(elemento, nodo->elemento) == 0)
        return nodo->elemento;
    else if (comparador(elemento, nodo->elemento) == -1)
        return buscar(nodo->izquierda, comparador, elemento);
    else
        return buscar(nodo->derecha, comparador, elemento);
}

/*
 * Busca en el arbol un elemento igual al provisto (utilizando la
 * funcion de comparación).
 *
 * Devuelve el elemento encontrado o NULL si no lo encuentra.
 */
void* arbol_buscar(abb_t* arbol, void* elemento)
{
    if (!arbol || !elemento)
        return NULL;
    return buscar(arbol->nodo_raiz, arbol->comparador, elemento);
}

/*
 * Devuelve el elemento almacenado como raiz o NULL si el árbol está
 * vacío o no existe.
 */
void* arbol_raiz(abb_t* arbol)
{
    if (!arbol)
        return NULL;
    if (arbol_vacio(arbol))
        return NULL;
    return arbol->nodo_raiz->elemento;
}

/*
 * Determina si el árbol está vacío.
 * Devuelve true si está vacío o el arbol es NULL, false si el árbol tiene elementos.
 */
bool arbol_vacio(abb_t* arbol)
{
    if (!arbol)
        return true;
    if (arbol->nodo_raiz)
        return false;
    return true;
}

/*
 * Funcion recursiva del recorrido inorden.
 * Llena el array hasta el tamaño indicado o hasta que se quede sin nodos.
 * Cada vez que llena un elemento, aumenta el contador en 1.
 */
void recorrido_inorden(nodo_abb_t* nodo, void** array, int tamanio_array, int* contador)
{

    if (!nodo)
        return;
    recorrido_inorden(nodo->izquierda, array, tamanio_array, contador);
    if (*contador >= tamanio_array)
        return;
    array[(*contador)++] = nodo->elemento;
    recorrido_inorden(nodo->derecha, array, tamanio_array, contador);
}

/*
 * Llena el array del tamaño dado con los elementos de arbol
 * en secuencia inorden.
 * Devuelve la cantidad de elementos del array que pudo llenar (si el
 * espacio en el array no alcanza para almacenar todos los elementos,
 * llena hasta donde puede y devuelve la cantidad de elementos que
 * pudo poner).
 */
int arbol_recorrido_inorden(abb_t* arbol, void** array, int tamanio_array)
{
    int contador = 0;
    if (arbol && array)
        recorrido_inorden(arbol->nodo_raiz, array, tamanio_array, &contador);
    return contador;
}

/*
 * Funcion recursiva del recorrido preorden.
 * Llena el array hasta el tamaño indicado o hasta que se quede sin nodos.
 * Cada vez que llena un elemento, aumenta el contador en 1.
 */
void recorrido_preorden(nodo_abb_t* nodo, void** array, int tamanio_array, int* contador)
{
    if (*contador >= tamanio_array || !nodo)
        return;
    array[(*contador)++] = nodo->elemento;
    recorrido_preorden(nodo->izquierda, array, tamanio_array, contador);
    recorrido_preorden(nodo->derecha, array, tamanio_array, contador);
}

/*
 * Llena el array del tamaño dado con los elementos de arbol
 * en secuencia preorden.
 * Devuelve la cantidad de elementos del array que pudo llenar (si el
 * espacio en el array no alcanza para almacenar todos los elementos,
 * llena hasta donde puede y devuelve la cantidad de elementos que
 * pudo poner).
 */
int arbol_recorrido_preorden(abb_t* arbol, void** array, int tamanio_array)
{
    int contador = 0;
    if (arbol && array)
        recorrido_preorden(arbol->nodo_raiz, array, tamanio_array, &contador);
    return contador;
}

/*
 * Funcion recursiva del recorrido postorden.
 * Llena el array hasta el tamaño indicado o hasta que se quede sin nodos.
 * Cada vez que llena un elemento, aumenta el contador en 1.
 */
void recorrido_postorden(nodo_abb_t* nodo, void** array, int tamanio_array, int* contador)
{
    if (!nodo)
        return;
    recorrido_postorden(nodo->izquierda, array, tamanio_array, contador);
    recorrido_postorden(nodo->derecha, array, tamanio_array, contador);
    if (*contador >= tamanio_array)
        return;
    array[(*contador)++] = nodo->elemento;
}

/*
 * Llena el array del tamaño dado con los elementos de arbol
 * en secuencia postorden.
 * Devuelve la cantidad de elementos del array que pudo llenar (si el
 * espacio en el array no alcanza para almacenar todos los elementos,
 * llena hasta donde puede y devuelve la cantidad de elementos que
 * pudo poner).
 */
int arbol_recorrido_postorden(abb_t* arbol, void** array, int tamanio_array)
{
    int contador = 0;
    if (arbol && array)
        recorrido_postorden(arbol->nodo_raiz, array, tamanio_array, &contador);
    return contador;
}

/*
 * Destruccion recursiva de los nodos.
 *
 * Se recorren los nodos en postorden, eliminandolos uno por uno
 * si es necesario utilizando el destructor(si esta disponible).
 */
static void destruir_nodos(nodo_abb_t* nodo, abb_liberar_elemento destructor)
{
    if (!nodo)
        return;
    destruir_nodos(nodo->izquierda, destructor);
    destruir_nodos(nodo->derecha, destructor);
    liberar_nodo(nodo, destructor);
}

/*
 * Destruye el arbol liberando la memoria reservada por el mismo.
 * Adicionalmente invoca el destructor con cada elemento presente en
 * el arbol.
 */
void arbol_destruir(abb_t* arbol)
{
    if (!arbol)
        return;
    destruir_nodos(arbol->nodo_raiz, arbol->destructor);
    free(arbol);
}

/*
 * Recorrido inorden para iterador interno.
 * Deja de recorrer cuando la funcion devuelva true
 */
bool inorden(nodo_abb_t* nodo, bool (*funcion)(void*, void*), void* extra)
{
    if (!nodo)
        return false;
    if (inorden(nodo->izquierda, funcion, extra))
        return true;
    if (funcion(nodo->elemento, extra))
        return true;
    if (inorden(nodo->derecha, funcion, extra))
        return true;
    return false;
}

/*
 * Recorrido preorden para iterador interno.
 * Deja de recorrer cuando la funcion devuelva true
 */
bool preorden(nodo_abb_t* nodo, bool (*funcion)(void*, void*), void* extra)
{
    if (!nodo)
        return false;
    if (funcion(nodo->elemento, extra))
        return true;
    if (preorden(nodo->izquierda, funcion, extra))
        return true;
    if (preorden(nodo->derecha, funcion, extra))
        return true;
    return false;
}

/*
 * Recorrido postorden para iterador interno.
 * Deja de recorrer cuando la funcion devuelva true
 */
bool postorden(nodo_abb_t* nodo, bool (*funcion)(void*, void*), void* extra)
{
    if (!nodo)
        return false;
    if (postorden(nodo->izquierda, funcion, extra))
        return true;
    if (postorden(nodo->derecha, funcion, extra))
        return true;
    if (funcion(nodo->elemento, extra))
        return true;
    return false;
}

/*
 * Iterador interno. Recorre el arbol e invoca la funcion con cada
 * elemento del mismo. El puntero 'extra' se pasa como segundo
 * parámetro a la función. Si la función devuelve true, se finaliza el
 * recorrido aun si quedan elementos por recorrer. Si devuelve false
 * se sigue recorriendo mientras queden elementos.
 * El recorrido se realiza de acuerdo al recorrido solicitado.  Los
 * recorridos válidos son: ABB_RECORRER_INORDEN, ABB_RECORRER_PREORDEN
 * y ABB_RECORRER_POSTORDEN.
 */
void abb_con_cada_elemento(abb_t* arbol, int recorrido, bool (*funcion)(void*, void*), void* extra)
{
    if (!arbol || !funcion)
        return;
    if (recorrido == ABB_RECORRER_INORDEN)
        inorden(arbol->nodo_raiz, funcion, extra);
    else if (recorrido == ABB_RECORRER_PREORDEN)
        preorden(arbol->nodo_raiz, funcion, extra);
    else if (recorrido == ABB_RECORRER_POSTORDEN)
        postorden(arbol->nodo_raiz, funcion, extra);
    return;
}
