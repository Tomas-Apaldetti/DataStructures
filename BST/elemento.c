#include <stdlib.h>

#include "elemento.h"

struct elemento
{
    int valor;
};

elemento_t* crear_elemento(int valor)
{
    elemento_t* elem = calloc(1, sizeof(elemento_t));
    if (elem)
        elem->valor = valor;
    return elem;
}

int comparar_elemento(void* elemento1, void* elemento2)
{
    if (!elemento1 || !elemento2)
        return 0;

    if (contenido(elemento1) > contenido(elemento2))
        return 1;

    if (contenido(elemento1) < contenido(elemento2))
        return -1;

    return 0;
}

int contenido(void* elemento)
{
    if (elemento)
        return (size_t)((elemento_t*)elemento)->valor;
    return -1;
}

void cambiar_dato(elemento_t** elemento, int nuevo_dato)
{
    if (elemento)
        (*elemento)->valor = nuevo_dato;
}

void destruir_elemento(void* elemento)
{
    free(elemento);
}