#include "hash_iterador.h"
#include "estructuras.h"
#include "hash.h"
#include "lista.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static lista_iterador_t* siguiente_bucket(hash_iterador_t* iterador)
{
    if (!iterador)
        return NULL;
    if (iterador->iter_lista)
        lista_iterador_destruir(iterador->iter_lista);
    for (size_t i = iterador->posic_actual; i < iterador->hash->capacidad; i++)
        if (iterador->hash->vector[i] && !lista_vacia(iterador->hash->vector[i]))
        {
            iterador->posic_actual = i + 1;
            return lista_iterador_crear(iterador->hash->vector[i]);
        }
    return NULL;
}

static lista_iterador_t* siguiente_elemento(hash_iterador_t* iterador)
{
    if (!iterador)
        return NULL;
    if (!iterador->iter_lista)
        return siguiente_bucket(iterador);
    if (lista_iterador_tiene_siguiente(iterador->iter_lista))
        return iterador->iter_lista;
    else
        return siguiente_bucket(iterador);
}
/*
 * Crea un iterador de claves para el hash reservando la memoria
 * necesaria para el mismo. El iterador creado es válido desde su
 * creación hasta que se modifique la tabla de hash (insertando o
 * removiendo elementos).
 *
 * Devuelve el puntero al iterador creado o NULL en caso de error.
 */
hash_iterador_t* hash_iterador_crear(hash_t* hash)
{
    if (!hash)
        return NULL;
    hash_iterador_t* iterador = calloc(1, sizeof(hash_iterador_t));
    if (!iterador)
        return NULL;
    iterador->hash = hash;
    iterador->iter_lista = siguiente_elemento(iterador);
    return iterador;
}

/*
 * Devuelve la próxima clave almacenada en el hash y avanza el iterador.
 * Devuelve la clave o NULL si no habia mas.
 */
const char* hash_iterador_siguiente(hash_iterador_t* iterador)
{
    if (!iterador || !iterador->iter_lista)
        return NULL;
    dato_t* actual = lista_iterador_siguiente(iterador->iter_lista);
    iterador->iter_lista = siguiente_elemento(iterador);
    return actual->clave;
}
/*
 * Devuelve true si quedan claves por recorrer o false en caso
 * contrario o de error.
 */
bool hash_iterador_tiene_siguiente(hash_iterador_t* iterador)
{
    if (!iterador)
        return false;
    if (iterador->iter_lista)
        return true;
    return false;
}

/*
 * Destruye el iterador del hash.
 */
void hash_iterador_destruir(hash_iterador_t* iterador)
{
    if (!iterador)
        return;
    if (iterador->iter_lista)
        lista_iterador_destruir(iterador->iter_lista);
    free(iterador);
}
