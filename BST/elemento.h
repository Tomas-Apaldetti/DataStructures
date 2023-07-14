#ifndef __ELEMENTO_H__
#define __ELEMENTO_H__

typedef struct elemento elemento_t;

/*
 * Crea un elemento con el valor dado.
 * Utilizar valores positivos.
 *
 * Devuelve un puntero a un elemento o NULL en caso de fallar.
 */
elemento_t* crear_elemento(int valor);

/*
 * Compara dos elementos
 * Devuelve 1 si el primer elemento es mayor, -1 si es menor, 0 en casos de ser iguales.
 */
int comparar_elemento(void* elemento1, void* elemento2);

/*
 * Devuelve el contenido de un elemento.
 *
 * Devuelve -1 en caso de error.
 */
int contenido(void* elemento);

/*
 * Cambia el dato del elemento que se pasa
 */
void cambiar_dato(elemento_t** elemento, int nuevo_dato);
/*
 * Destruye un elemento
 */
void destruir_elemento(void* elemento);

#endif /* __ELEMENTO_H__ */