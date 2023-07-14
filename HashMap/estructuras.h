#ifndef __ESTRUCTURAS_H__
#define __ESTRUCTURAS_H__
#include "hash.h"
#include "hash_iterador.h"
#include "lista.h"

struct hash
{
    size_t               capacidad;
    size_t               cantidad;
    lista_t**            vector;
    hash_destruir_dato_t destructor;
};

struct hash_iter
{
    hash_t*           hash;
    size_t            posic_actual;
    lista_iterador_t* iter_lista;
};

typedef struct elemento
{
    void* elemento;
    char* clave;
} dato_t;

#endif /*__ESTRUCTURAS_H__*/