//*****************************************************************
//ESCUELA SUPERIOR DE CÓMPUTO - IPN
//Curso: Análisis y Diseño de algoritmos
//Autores:
            //Ojeda Navarro Guillermo

//Compilación: "gcc codificacion.c tiempo.c -o codificar"
//Ejecución: "./codificar <nombre del archivo con extensión>"
//*****************************************************************

//*****************************************************************
//LIBRERIAS INCLUIDAS
//*****************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include "definiciones.h"
#include <unistd.h>
#include "tiempo.h"

//*****************************************************************
//    PROGRAMA PRINCIPAL
//*****************************************************************
int main(int argc, char *argv[])
{
    //*************************************************************
    // Declaración de variables para el PROGRAMA PRINCIPAL
    //*************************************************************
    double utime0, stime0, wtime0,utime1, stime1, wtime1; //Para la medición de tiempos
    FILE *arch = NULL; //Apuntador al archivo para leer
    unsigned long tam;            //Almacena el tamaño del archivo a leer
    int i = 0;                    //Para los loops
    lista *l = NULL;              //Para la lista
    arbol *a;                     //Para el arbol
    int bit = 7;                  //Para hacer el corrimiento de bits
    unsigned char auxCara = 0;    //Auxiliar para almacenar los caracteres codificados

    printf("\nNombre del archivo: ");
    scanf("%s", archivo);                   
    arch = fopen(archivo, "rb"); //Abrimos el archivo en modo lectura binaria
    if (arch == NULL) // Si el archivo es nulo
    {
        printf("\nNombre incorrecto o no existe el archivo"); 
        exit(1); 
    }
    
    //******************************************************************    
    //Iniciar el conteo del tiempo para las evaluaciones de rendimiento
    //******************************************************************    
    uswtime(&utime0, &stime0, &wtime0);
    //******************************************************************
    tam = detallesArchivo(arch); 

    // Se pasan los datos leidos del archivo al arreglo
    unsigned char *datos = (char *)malloc(sizeof(char) * tam); 
    fread(datos, sizeof(unsigned char), tam, arch); // Leemos el contenido del archivo


    //********************************************************************************
    //CREAR LA LISTA DE FRECUENCIAS 
    //********************************************************************************
    unsigned long frecuencias[256] = {0}; //Arreglo de 256 posiciones, cada posicion representa un caracter
    while (i < tam)
    {
        frecuencias[datos[i]]++; //sumamos uno a la frecuencia en la posicion del caracter
        i++;
    }

    // Vamos agregando cada caracter a la lista de frecuencias (las frecuencias deben ser distinto de cero)
    for (int k = 0; k < 256; k++)
    {
        if (frecuencias[k] != 0)
        {
            a = (arbol *)malloc(sizeof(arbol)); // Crear un nuevo nodo para elarbol 
            a->dato = k;                        // Asignación del byte
            a->frec = frecuencias[k];           // Asignación de la frecuencia del byte

            agregarLista(&l, a); //Agregacioón al principio de la lista
        }
    }
    // Ordenar la lista de bytes ascendentemente respecto a sus frecuencias
    mergeSort(&l);


    //************************************************************************
    // ESCRITURA DE LOS BYTES Y LAS FRECUENCIAS EN ARCHIVO 
    //************************************************************************
    escribirArchivoFrecuencias(l);

    //************************************************************************
    // CREAMOS EL ARBOL Y OBTENER LA ALTURA
    //************************************************************************
    l = crearArbol(l);           //Unión los arboles de la lista en uno solo
    altura = alturaArbol(l->ar); //Obtención de la altura del arbol

    //***********************************************************************
    //CODIFICACIÓN DE CADA CARACTER Y LO GUARDAMOS CADA CÓDIGO EN UN ARREGLO -------------
    // Inicializamos el arreglo temporal que guardarÃ¡ los códigos
    arregloBitsTemp = (unsigned char *)malloc(sizeof(unsigned char) * (altura + 1)); 
    codificarHojas(l->ar, 0, arregloBitsTemp); // Mandamos a almacenar todos los códidos de las hojas


    // -------- CODIFICAMOS LOS ELEMENTOS DEL ARCHIVO RESPECTO A EL ARREGLO DE CODIGOS ----------
    int indice = 0; // Indice acumulador para detectar la cantidad de bits que se van codificando
    // Abrimos el archivo binario
    FILE *codificado = NULL;           //variable para el archivo codificado
    strcat(archivo, ".dat");           //concatenamos .dat para la salida del archivo
    codificado = fopen(archivo, "wb"); // Abrimos el archivo para escritura binaria

    // Recorremos cada caracter a codificar y una vez codificados lo escribirmos en el archivo
    for (i = 0; i < tam; i++)
    {
        caracter = datos[i]; // Almacenamos temporalmente cada caracter

        // Escribimos los bits del arreglo de codigos (por caracter), verificando cuando se complete cada byte
        for (int j = 0; j < nivelHojas[caracter]; j++)
        {   // Si ya se hicieron todos los corrimientos (se completó el byte)
            if (bit < 0)
            {
                bit = 7;                                       //Reiniciamos los corrimientos
                fwrite(&auxCara, sizeof(char), 1, codificado); //escribimos el caracter codificado en el arch.
                auxCara = 0;                                   //Ponemos en cero los bits para el nuevo byte
            }

            /*  Hacemos la compresion haciendo corrimientos ayudandonos de la variable bit
                AsÃ­ llevamos la cuenta de los bits procesados para ir encendiendo cada uno de los bytes del 
                archivo (acomodamos y encendemos), cuando se procesan los 8 bits escribimos en el archivo
            */
            auxCara = auxCara | (codigosHojas[caracter][j] << bit);

            bit--; //vamos reduciendo la cantidad de bits que faltan por procesar
        }
        indice = (indice + nivelHojas[caracter]); // Aumentamos el indice correspondiente a la cantidad de 
        // bits que se escribieron en el archivo
    }

    // En caso de que hayan quedado un byte incompleto, lo escribimos en el archivo
    if (bit >= 0)
    {
        fwrite(&auxCara, sizeof(char), 1, codificado); //imprimimos el caracter codificado en el archivo final
    }

    fclose(codificado);
    
    
    //******************************************************************    
    //Evaluar los tiempos de ejecución 
    //******************************************************************
    uswtime(&utime1, &stime1, &wtime1);
    
    //Cálculo del tiempo de ejecución del programa
    printf("\n");
    printf("real (Tiempo total)  %.10f s\n",  wtime1 - wtime0);
    printf("\n");
    
    return 0; //salimos
   }

/*****************************************************************
    IMPLEMENTACIóN DE FUNCIONES
*****************************************************************/
/*
    Función para obtener los detalles del archivo, en este caso nos interesa el tamaÃ±o.
    Se hizo uso de la libreria sys/stat.h.
    REcibe el archivo del que deseamos saber su tamaÃ±o
    Retorna el tamaÃ±o del archivo
*/
unsigned long detallesArchivo(FILE * arch)
{
    struct stat buffer; //estructura para recuperar atributos del archivo

    unsigned long descriptor = fileno(arch); //regresa el descriptor de archivo
    //con la llamada a la funcion fstat obtenemos detalles del archivo
    if (fstat(descriptor, &buffer) == -1)
    { //si regresa -1 hubo un error
        perror("Error en fstat");
        exit(1); //Salimos del programa
    }

    return buffer.st_size; //Retornamos el tamaÃ±o del archivo
}

/*
    Función que agrega un elemento al inicio de la lista
    Recibe la lista donde se insertarÃ¡ el elemento y el elemento a insertar
    en este caso el arbol con el dato y la frecuencia
    Retorna el nuevo elemento 
*/
void agregarLista(lista * *l, arbol * a)
{
    lista *nuevoArbol;                           //creamos una variable para alamacenar el dato
    nuevoArbol = (lista *)malloc(sizeof(lista)); //le asignamos espacio al dato
    nuevoArbol->ar = a;                          //guardamos el arbol en el nuevo nodo
    nuevoArbol->siguiente = *l;                  //el siguiente nodo serÃ¡ igual a la lista que teniamos
    (*l) = nuevoArbol;
}

/*
    La siguiente función es para ordenar nuestra lista, se uso mergeSort teniendo una complejidad O(nLogn)
    Recibe la lista que serÃ¡ ordenada bajo este mÃ©todo
    No regresa nada ya que se esta modificando directamente la lista
*/
void mergeSort(lista * *l)
{
    lista *inicio = *l; //establecemos la cabeza de la lista
    lista *a, *b;       //Establecemos los nodos

    //Si la lista es nula o solo hay un arbol no ordenamos
    if ((inicio == NULL) || (inicio->siguiente == NULL))
        return; //salimos de la funcion

    //Función para hacer sublistas de la lista
    sublistas(inicio, &a, &b);

    //Llamada recursiva a merge para las sublistas creadas
    mergeSort(&a); //merge para la sublista a
    mergeSort(&b); //merge para la sublista b

    //Juntamos las dos sublistas ordenadas
    *l = mezcla(a, b);
}

/*
    Función que realiza el merge para tener la lista completamente ordenada
    Recibe la sublista a y sublista b a mezclar
    Regresa la lista mezclada
*/
lista *mezcla(lista * a, lista * b)
{
    lista *resultado = NULL; //variable que almacenara la lista completa

    //Si a es nula regresamos la lista b solamente y viceversa, serÃ¡n los casos base
    if (a == NULL)
        return b; //regresamos la sublista b
    else if (b == NULL)
        return a; //regresamos la sublista a

    /*
    Comparamos las frecuencias de los arboles dentro de las dos sublistas
    Si la frecuencia del arbol de la sublista a es menor o igual que el del arbol de la sublista b
    se le asigna a al resultado, posteriomente se avanza un nodo y se llama recursivamente 
    a la función para hacer la misma comparación, pero con el siguiente nodo
    Si la comparación no se cumple entonces el resultado se asigna a b y se repite lo anterior.
*/
    if (a->ar->frec <= b->ar->frec)
    {
        resultado = a;
        resultado->siguiente = mezcla(a->siguiente, b); //Llamada recursiva a a la mezcla
    }
    else
    {
        resultado = b;
        resultado->siguiente = mezcla(a, b->siguiente); //llamada recursiva a la mezcla
    }

    return resultado; //retornmaos el resultaado
}

/*
    Esta función parte la lista en dos sublistas para comenzar el ordenamiento
    Recibe la lista a partir, la sublista que sera la aprte de atras y la que sera la parte de adelante
    No retorna nada, pues se modifican los nodos por referencia.
*/
void sublistas(lista * l, lista * *delante, lista * *atras)
{
    lista *avanzaDosNodos = l->siguiente; //variable con la que avanzaremos dos nodos de la lista
    lista *avanzaUnNodo = l;              //variable para avanzar un nodo de la lista

    //Mientras se pueda avanzar en la lista avanzamos dos nodos
    while (avanzaDosNodos != NULL)
    {
        avanzaDosNodos = avanzaDosNodos->siguiente; //avnazamos dos nodos de la lista
        if (avanzaDosNodos != NULL)
        {                                               //SI aun no llega al final, avanzamos un nodo
            avanzaUnNodo = avanzaUnNodo->siguiente;     //avanzamos un nodo
            avanzaDosNodos = avanzaDosNodos->siguiente; //Seguimos avanzando dos nodos
        }
    }

    *delante = l; //almacenamos la parte de adelante
                //la parte de atras se asigna a avanzaUnNodo, porque queda antes de la mitad de la lista
    *atras = avanzaUnNodo->siguiente;
    avanzaUnNodo->siguiente = NULL; //el siguiente nodo en NULL
}

/*
    Función que escribe el archivo de frecuencias con los elementros dentro de la lista
    Recibe la lista que contiene los elementos a escribir
    No retorna nada, pero creo un archivo frecuencias.txt con los caracteres y su frecuencia
*/
void escribirArchivoFrecuencias(lista * l)
{
    FILE *frecuencias; //variable para el archivo

    frecuencias = fopen("frecuencias.txt", "wb"); //se abre o crea el archivo en modo escritura

    //Recorremos la lista para impriomir en el archivo
    while (l != NULL)
    {
        //colocamos el caracter junto con su frecuencia
        unsigned char auxDato = l->ar->dato;
        unsigned long auxFrec = l->ar->frec;
        fwrite(&auxDato, sizeof(unsigned char), 1, frecuencias);
        fwrite(&auxFrec, sizeof(unsigned long), 1, frecuencias);
        l = l->siguiente; //avanzamos al siguiente nodo
    }

    fclose(frecuencias); //Cerramos el archivo
}

/*
    Funcion que crea el arbol a partir de ir uniendo dos arboles de la lista 
    y sumando sus frecuencias.
    Recibe la lista que contiene los arboles que se juntarÃ¡n.
    Regresa la lista con un solo arbol que fue unido
*/
lista *crearArbol(lista * l)
{
    //vamos recorriendo  la lista mientras el siguiente nodo no sea nulo
    while (l->siguiente != NULL)
    {
        //Establecemos dos listas, una ira almacenando el arbol unido y otra que servira como auxiliar
        lista *arbolUnido = l;
        lista *aux = l;
        //Como iremos comparando de dos en dos los arboles, recorremos la lista en dos elementos
        l = l->siguiente->siguiente;

        //mandamos a llamar la función que une los arboles para que una los primeros dos
        arbolUnido->ar = unirArboles(arbolUnido->siguiente->ar, arbolUnido->ar);

        /*
        SI la lista es nula o si la frecuencia del arbol unido anteriormente es menor
        a la frecuencia del arbol, haremos que el nuevo elemento de la lista sea el elemento
        desde el cual se recorrio la lista y la lista serÃ¡ el elemento unido
    */
        if (l == NULL || arbolUnido->ar->frec <= l->ar->frec)
        {
            arbolUnido->siguiente = l; //el siguiente elemento serÃ¡ al que apunto al recorrer
            l = arbolUnido;            //la lista tserÃ¡ el arbol unido
        }
        /*
        Si la lista no es nula y la frecuencia del arbol unido es mayor a la del arbol
        haremos uso del auxiliar guardando ahi la lista y avanzando hasta que se encuentre
        la posicion correcta del nuevo nodo que es el arbol unido.
    */
        if (l != NULL && arbolUnido->ar->frec > l->ar->frec)
        {
            aux = l;
            //Recorremos el auxiliar hasta que sea nulo o se encuentre un elemento mayor al arbol unido
            while (aux->siguiente != NULL && arbolUnido->ar->frec > aux->ar->frec)
            {
                aux = aux->siguiente; //avanzamos en el auxiliar
            }
            arbolUnido->siguiente = aux->siguiente; //asigna la posicion donde se guardara el nuevo arbol
            aux->siguiente = arbolUnido;            //almacenamos el nuevo arbol en la posicion
        }
    }

    return l; //retornamos la lista unida
}

/*
    Esta función une los arboles, para hacer esto se crea un nuevo arbol y se almacena en el
    la frecuencia del arbol mayor y menor para asi formar un solo arbol
    Recibe el arbol con el caracter con mayor frecuencia y un arbol con el caracter de menor frecuencia
    Retorna el nuevo arbol que se almacenara en la lista
*/
arbol *unirArboles(arbol * aMayor, arbol * aMenor)
{
    arbol *nuevo = (arbol *)malloc(sizeof(arbol)); //Creamos el nuevo arbol

    nuevo->frec = aMayor->frec + aMenor->frec; //Sumamos las frecuencias del arbol
    nuevo->izq = aMenor;                       //asigna a la izquierda el arbol con menor frecuencia
    nuevo->der = aMayor;                       //asigna a la derecha el arbol con mayor frecuencia

    return nuevo; //Retornamos el nuevo arbol
}

/*
    Función que obtiene los códigos y nivele de cada nodo hoja que contienen los caracteres
    Recibe el arbol y el nivel en el que se encuentra además de un arreglo auxuliar para almacenar 
    los códigos
    No devuelve nada, solo int para deterner ejecución
*/

int codificarHojas(arbol * nodo, int nivel, unsigned char *arregloBitsTemp)
{
    // Si el nodo actual no esta vacio
    if (nodo != NULL)
    {
        // En caso de que ya haya encontrado la hoja que contiene al caracter
        if (nodo->der == NULL && nodo->izq == NULL)
        {
            // Guardamos todo el codigo del caracter
            for (int i = 0; i < nivel; i++)
            {
                // copiamos el arreglo de bits en el arreglo de bits de la hoja
                codigosHojas[nodo->dato][i] = arregloBitsTemp[i];
            }
            // Copiamos la longitud del codigo
            nivelHojas[nodo->dato] = nivel;

            return 0;
        }
        // Recorremos la rama derecha asignando uno a esa rama
        arregloBitsTemp[nivel] = 1;
        codificarHojas(nodo->der, nivel + 1, arregloBitsTemp);

        // Recorremos la rama derecha asignando cero a esa rama
        arregloBitsTemp[nivel] = 0;
        codificarHojas(nodo->izq, nivel + 1, arregloBitsTemp);
    }
}

/*
    FUnción para calcular la altura del arbol.
    Recibe la altura del arbol del que se desea saber la altura
    Devuelve la ltura del arbol
*/
int alturaArbol(arbol * a)
{
    //Si no hay elemntos la altura sera 0
    if (!a)
    {
        return 0;
    }
    int alt = 0; //para almacenar la altura

    //calculamos la altura de la derecha
    int alturaDerecha = alturaArbol(a->der);
    //calculamos la altura de la izquierda
    int alturaIzquierda = alturaArbol(a->izq);

    //la altura mayor se guardarÃ¡ como la altura del arbol
    alt = alturaIzquierda > alturaDerecha ? alturaIzquierda + 1 : alturaDerecha + 1;

    return alt; //Retornamos la altura
}
