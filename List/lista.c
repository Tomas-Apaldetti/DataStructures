#include "lista.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct nodo
{
    void*        dato;
    struct nodo* siguiente;
} nodo_t;

struct lista
{
    nodo_t* head;
    size_t  largo;
};

struct lista_iterador
{
    nodo_t* actual;
};

lista_t* lista_crear()
{
    return calloc(1, sizeof(lista_t));
}

/*
 * Crea un nuevo nodo, con elemento como dato, apunta a NULL
 * Devuelve un puntero a un nodo, o NULL en caso de fallar
 */
static nodo_t* nuevo_nodo(void* elemento)
{
    nodo_t* nodo = calloc(1, sizeof(nodo_t));
    if (!nodo)
        return NULL;
    nodo->dato = elemento;
    return nodo;
}
/*
 * Inserta un elemento al final de la lista.
 * Devuelve 0 si pudo insertar o -1 si no pudo.
 */
int lista_insertar(lista_t* lista, void* elemento)
{
    if (!lista)
        return -1;
    nodo_t* tracker = lista->head;
    nodo_t* auxiliar = nuevo_nodo(elemento);
    if (!auxiliar)
        return -1;
    if (tracker)
    {
        while (tracker->siguiente)
            tracker = tracker->siguiente;
        tracker->siguiente = auxiliar;
    }
    else
        lista->head = auxiliar;
    lista->largo++;
    return 0;
}

/*
 * Inserta un elemento en la posicion indicada, donde 0 es insertar
 * como primer elemento y 1 es insertar luego del primer elemento.
 * En caso de no existir la posicion indicada, lo inserta al final.
 * Devuelve 0 si pudo insertar o -1 si no pudo.
 */
int lista_insertar_en_posicion(lista_t* lista, void* elemento, size_t posicion)
{
    if (!lista)
        return -1;
    if (posicion >= lista->largo || lista_vacia(lista))
        return lista_insertar(lista, elemento);
    nodo_t* anterior = NULL;
    nodo_t* tracker = lista->head;
    nodo_t* auxiliar = nuevo_nodo(elemento);
    if (!auxiliar)
        return -1;
    for (int i = 0; i < posicion; i++)
    {
        anterior = tracker;
        tracker = tracker->siguiente;
    }
    if (anterior)
    {
        anterior->siguiente = auxiliar;
        auxiliar->siguiente = tracker;
    }
    else
    {
        auxiliar->siguiente = tracker;
        lista->head = auxiliar;
    }
    lista->largo++;
    return 0;
}
/*
 *Libera la memoria reservada para un nodo
 */
static void liberar_nodo(nodo_t* nodo)
{
    free(nodo);
}
/*
 * Quita de la lista el elemento que se encuentra en la ultima posición.
 * Devuelve 0 si pudo eliminar o -1 si no pudo.
 */
int lista_borrar(lista_t* lista)
{
    if (!lista || lista_vacia(lista))
        return -1;
    nodo_t* anterior = NULL;
    nodo_t* tracker = lista->head;
    while (tracker->siguiente)
    {
        anterior = tracker;
        tracker = tracker->siguiente;
    }
    if (anterior)
        anterior->siguiente = NULL;
    else
        lista->head = NULL;
    liberar_nodo(tracker);
    lista->largo--;
    return 0;
}

/*
 * Quita de la lista el elemento que se encuentra en la posición
 * indicada, donde 0 es el primer elemento.
 * En caso de no existir esa posición se intentará borrar el último
 * elemento.
 * Devuelve 0 si pudo eliminar o -1 si no pudo.
 */
int lista_borrar_de_posicion(lista_t* lista, size_t posicion)
{
    if (!lista || lista_vacia(lista))
        return -1;
    if (posicion >= lista->largo)
        return lista_borrar(lista);
    nodo_t* anterior = NULL;
    nodo_t* tracker = lista->head;
    for (int i = 0; i < posicion; i++)
    {
        anterior = tracker;
        tracker = tracker->siguiente;
    }
    if (anterior)
        anterior->siguiente = tracker->siguiente;
    else
        lista->head = tracker->siguiente;
    liberar_nodo(tracker);
    lista->largo--;
    return 0;
}

/*
 * Devuelve el elemento en la posicion indicada, donde 0 es el primer
 * elemento.
 *
 * Si no existe dicha posicion devuelve NULL.
 */
void* lista_elemento_en_posicion(lista_t* lista, size_t posicion)
{
    if (!lista || posicion >= lista_elementos(lista))
        return NULL;
    nodo_t* tracker = lista->head;
    for (int i = 0; i < posicion; i++)
        tracker = tracker->siguiente;
    return tracker->dato;
}

/*
 * Devuelve el último elemento de la lista o NULL si la lista se
 * encuentra vacía.
 */
void* lista_ultimo(lista_t* lista)
{
    if (!lista)
        return NULL;
    if (lista_vacia(lista))
        return NULL;
    nodo_t* tracker = lista->head;
    while (tracker->siguiente)
        tracker = tracker->siguiente;
    return tracker->dato;
}

/*
 * Devuelve true si la lista está vacía o false en caso contrario.
 */
bool lista_vacia(lista_t* lista)
{
    return (lista_elementos(lista) == 0);
}
/*
 * Devuelve la cantidad de elementos almacenados en la lista.
 */
size_t lista_elementos(lista_t* lista)
{
    if (lista)
        return lista->largo;
    return 0;
}
/*
 * Apila un elemento.
 * Devuelve 0 si pudo o -1 en caso contrario.
 */
int lista_apilar(lista_t* lista, void* elemento)
{
    return lista_insertar(lista, elemento);
}

/*
 * Desapila un elemento.
 * Devuelve 0 si pudo desapilar o -1 si no pudo.
 */
int lista_desapilar(lista_t* lista)
{
    return lista_borrar(lista);
}

/*
 * Devuelve el elemento en el tope de la pila o NULL
 * en caso de estar vacía.
 */
void* lista_tope(lista_t* lista)
{
    return lista_elemento_en_posicion(lista, lista_elementos(lista) - 1);
}

/*
 * Encola un elemento.
 * Devuelve 0 si pudo encolar o -1 si no pudo.
 */
int lista_encolar(lista_t* lista, void* elemento)
{
    return lista_insertar_en_posicion(lista, elemento, 0);
}

/*
 * Desencola un elemento.
 * Devuelve 0 si pudo desencolar o -1 si no pudo.
 */
int lista_desencolar(lista_t* lista)
{
    return lista_borrar(lista);
}

/*
 * Devuelve el primer elemento de la cola o NULL en caso de estar
 * vacía.
 */
void* lista_primero(lista_t* lista)
{
    return lista_tope(lista);
}

/*
 * Libera la memoria reservada por la lista.
 */
void lista_destruir(lista_t* lista)
{
    if (!lista)
        return;
    while (!lista_vacia(lista))
        lista_borrar(lista);
    free(lista);
}

/*
 * Crea un iterador para una lista. El iterador creado es válido desde
 * el momento de su creación hasta que no haya mas elementos por
 * recorrer o se modifique la lista iterada (agregando o quitando
 * elementos de la lista).
 *
 * Devuelve el puntero al iterador creado o NULL en caso de error.
 */
lista_iterador_t* lista_iterador_crear(lista_t* lista)
{
    if (!lista)
        return NULL;
    lista_iterador_t* it = calloc(1, sizeof(lista_iterador_t));
    if (!it)
        return NULL;
    it->actual = lista->head;
    return it;
}
/*
 * Devuelve true si hay mas elementos sobre los cuales iterar o false
 * si no hay mas.
 */
bool lista_iterador_tiene_siguiente(lista_iterador_t* iterador)
{
    if (!iterador)
        return false;
    return iterador->actual;
}

/*
 * Devuelve el próximo elemento disponible en la iteración.
 * En caso de error devuelve NULL.
 */
void* lista_iterador_siguiente(lista_iterador_t* iterador)
{
    if (!iterador)
        return NULL;
    if (lista_iterador_tiene_siguiente(iterador))
    {
        nodo_t* actual = iterador->actual->dato;
        iterador->actual = iterador->actual->siguiente;
        return actual;
    }
    return NULL;

    /* if (!iterador->actual)
        return NULL;
    nodo_t* actual = iterador->actual->dato;
    if (lista_iterador_tiene_siguiente(iterador))
        iterador->actual = iterador->actual->siguiente;
    return actual; */
}

/*
 * Libera la memoria reservada por el iterador.
 */
void lista_iterador_destruir(lista_iterador_t* iterador)
{
    free(iterador);
}

/*
 * Iterador interno. Recorre la lista e invoca la funcion con cada
 * elemento de la misma.
 */
void lista_con_cada_elemento(lista_t* lista, void (*funcion)(void*, void*), void* contexto)
{
    if (!lista || !funcion)
        return;
    nodo_t* tracker = lista->head;
    while (tracker)
    {
        funcion(tracker->dato, contexto);
        tracker = tracker->siguiente;
    }
}
