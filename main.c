#include <stdio.h>
#include <stdlib.h>

#define TURNO_H 1			/*Turno humano*/
#define TURNO_M 2			/*Turno maquina*/
#define VACIO '.'			/*Casilla vacia en el tablero*/
#define int_MAX 32000		/*Numero suficientemente grande para asignar a LB y D en la funcion AB*/

typedef struct nodo{
	char tablero[8][8];
	short int turno;
	int valor;
	struct nodo *hijo;
	struct nodo *sgte;
}nodo_t;

char tablero[8][8], FICHA_H, FICHA_M;						/*Tablero de juego y fichas del humano y de la maquina*/

int pesos[8][8]={	{120, -40, 20, 5, 5, 20, -40, 120},		/*Tabla con pesos para calcular el valor de fichas estables*/
					{-40, -60, -5, -5, -5, -5, -60, -40},
					{20, -5, 15, 3, 3, 15, -5, 20},
					{5, -5, 3, 3, 3, 3, -5, 5},
					{5, -5, 3, 3, 3, 3, -5, 5},
					{20, -5, 15, 3, 3, 15, -5, 20},
					{-40, -60, -5, -5, -5, -5, -60, -40},
					{120, -40, 20, 5, 5, 20, -40, 120}	};

void inicializar_tablero();
void imprimir_tablero(char tablero[8][8]);
int colocar_ficha(char tab[8][8], int f, int c, short int turno, int colocar, int imprimir);
void juega_humano(short int *turno);
void juega_maquina(short int *turno, short int altura);
void inicio(short int *turno);
int fin_juego(short int *turno, int *no_movio);
void resultado();
nodo_t *construir_arbol(short int altura);
void expandir_arbol(nodo_t *p, int nivel, short int altura);
nodo_t *generar_arbol(char tablero_padre[8][8], short int turno_padre);
void liberar_arbol(nodo_t* p);
nodo_t *crear_nodo(char tablero_padre[8][8], int i, int j, short int turno);
void agregar_nodo(nodo_t **ptr, nodo_t *nuevo_nodo);
void copiar_tablero(char a[8][8], char b[8][8]);
int AB(nodo_t *x, int nivel, int LB, int D);
int max(int a, int b);
int eval(char tablero_actual[8][8]);
int dif_cantidad_de_fichas(char tablero_actual[8][8]);
int dif_fichas_estables(char tablero_actual[8][8]);

int main(void) {

	short int turno = TURNO_M;				/*Variable que indica quien posee el turno para jugar*/
	int no_movio = 0;						/*Variable que indica si el humano, maquina o ambos no pudieron realizar algun movimiento*/
	short int altura = 5;					/*Altura del arbol*/

	inicio(&turno);				/*Desplego mensaje inicial y obtengo del usuario el jugador que empieza el juego y altura*/
	inicializar_tablero();					/*Cargo el tablero de juego*/

	/*ACA SE EJECUTA EL JUEGO, se ejecuta mientras fin_juego permanesca en 0, si se pone a 1 se sale de while, dependiendo del turno
	 *juegan el humano y la maquina mediante las funciones juega_humano y juega_maquina*/
	while (1){
		if ( !fin_juego(&turno, &no_movio )  ){
			if ( no_movio == 1 ) continue;
			if ( turno == TURNO_H ){
				juega_humano(&turno);
			}
			else{
				juega_maquina(&turno,altura);
			}
		}
		else{
			break;
		}
	}

	resultado();
   	system("PAUSE");
	return 0;
}

void inicializar_tablero(){
/*Carga el tablero inicial del juego con los valores correspondientes*/
	int i, j;
	for (i=0;i<8;i++){
		for (j=0;j<8;j++){
			tablero[i][j] = VACIO;
		}
	}
	tablero[3][3] = 'O';
	tablero[3][4] = 'X';
	tablero[4][3] = 'X';
	tablero[4][4] = 'O';
}

void imprimir_tablero(char tab[8][8]){
/*Imprime el tablero de juego*/
	int i, j;
	//printf("\t\t\t\t\t\t\t\t");
	printf("\t\t\t\t");
    for(i=0;i<8;i++) printf("%c ",i+65);
	printf("\n\n\n");
	for (i=0;i<8;i++){
		//printf("\t\t\t\t\t\t\t");
		printf("\t\t\t");
		printf("%d",i);
		printf("\t");
		for (j=0;j<8;j++) printf("%c ",tab[i][j]);
		printf("      %d",i);
		printf("\n");
	}
	//printf("\n\n\t\t\t\t\t\t\t\t");
	printf("\n\n\t\t\t\t");
    for(i=0;i<8;i++) printf("%c ",i+65);
	printf("\n\n\n");
}

int colocar_ficha(char tab[8][8], int f, int c, short int turno, int colocar, int imprimir){
/*Si colocar==1, coloca la ficha en la posicion f, c e invierte las fichas comidas, retorna 1 si pudo colocar la ficha, en caso contrario
 *retorna 0, si colocar==0, analiza si se puede colocar la ficha en la posicion f, c pero no coloca la ficha, si se puede colocar retorna
 *1, en caso contrario retorna 0, imprimir permite que el tablero se imprima o no*/

	int tmp1, tmp2, i, j, cont;
	int ba = 0;

	if ( f<0 || f>7 || c<0 || c>7 ){		/*Si se trata colocar en un lugar fuera del tablero*/
		return 0;
	}

	if ( tab[f][c] != VACIO ){			/*Si se trata colocar en un lugar ocupado en el tablero*/
		return 0;
	}

	if ( turno == TURNO_H){
	/*Dentro de este if, cada vez que el humano intenta colocar una ficha se evalua la linea en donde intenta colocarla, ya sea arriba,
	 * arriba derecha, derecha, abajo derecha, abajo, abajo izquierda, izquierda o arriba izquierda de la ficha, si encuentra una ficha
	 * de humano en algun lugar de la linea que no sea el lugar mas proximo y no encuentra ningun vacio, entonces si colocar==1 se coloca
	 * la ficha, se invierten las que se encuentran en la linea (no se coloca ni invierte nada si colocar==0)y ba se pone 1 para poder
	 * retornar 1 al final de la funcion, si no se encuentra nada ba permanece en 0 y se retorna 0 al final de la funcion*/

		if ( tab[f-1][c] == FICHA_M  ){				/*Ficha de humano arriba*/
			tmp1 = f-2;
			while( tmp1>=0 ){
				if ( tab[tmp1][c] == VACIO )
					break;
				if ( tab[tmp1][c] == FICHA_H ){
					if ( colocar == 1 ){
						j = c;
						for (i=f;i>=tmp1;i--){
							tab[i][j] = FICHA_H;
							if (i==f){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1--;
			}
		}

		if ( tab[f-1][c+1] == FICHA_M  ){			/*Ficha de humano arriba derecha*/
			tmp1 = f-2;
			tmp2 = c+2;
			while( tmp1>=0 && tmp2<=7 ){
				if ( tab[tmp1][tmp2] == VACIO )
					break;
				if ( tab[tmp1][tmp2] == FICHA_H ){
					if ( colocar == 1 ){
						cont = f - tmp1;
						for (i=0;i<=cont;i++){
							tab[f-i][c+i] = FICHA_H;
							if (i==0){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1--;
				tmp2++;
			}
		}

		if ( tab[f][c+1] == FICHA_M  ){				/*Ficha de humano derecha*/
			tmp1 = c+2;
			while( tmp1<=7 ){
				if ( tab[f][tmp1] == VACIO )
					break;
				if ( tab[f][tmp1] == FICHA_H ){
					if ( colocar ==1 ){
						i = f;
						for (j=c;j<=tmp1;j++){
							tab[i][j] = FICHA_H;
							if (j==c){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
					}
				tmp1++;
			}
		}

		if ( tab[f+1][c+1] == FICHA_M  ){			/*Ficha de humano abajo derecha*/
			tmp1 = f+2;
			tmp2 = c+2;
			while( tmp1<=7 && tmp2<=7 ){
				if ( tab[tmp1][tmp2] == VACIO )
					break;
				if ( tab[tmp1][tmp2] == FICHA_H ){
					if ( colocar == 1 ){
						cont = tmp1 - f;
						for (i=0;i<=cont;i++){
							tab[f+i][c+i]=FICHA_H;
							if (i==0) {
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1++;
				tmp2++;
			}
		}

		if ( tab[f+1][c] == FICHA_M  ){				/*Ficha de humano abajo*/
			tmp1 = f+2;
			while( tmp1<=7 ){
				if ( tab[tmp1][c] == VACIO )
					break;
				if ( tab[tmp1][c] == FICHA_H ){
					if ( colocar == 1){
						j = c;
						for (i=f;i<=tmp1;i++){
							tab[i][j] = FICHA_H;
							if (i==f){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1++;
			}
		}

		if ( tab[f+1][c-1] == FICHA_M  ){			/*Ficha de humano abajo izquierda*/
			tmp1 = f+2;
			tmp2 = c-2;
			while( tmp1<=7 && tmp2>=0 ){
				if ( tab[tmp1][tmp2] == VACIO )
					break;
				if ( tab[tmp1][tmp2] == FICHA_H ){
					if ( colocar == 1 ){
						cont = tmp1 - f;
						for (i=0;i<=cont;i++){
							tab[f+i][c-i]=FICHA_H;
							if (i==0) {
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1++;
				tmp2--;
			}
		}

		if ( tab[f][c-1] == FICHA_M  ){				/*Ficha de humano izquierda*/
			tmp1 = c-2;
			while( tmp1>=0 ){
				if ( tab[f][tmp1] == VACIO )
					break;
				if ( tab[f][tmp1] == FICHA_H ){
					if ( colocar == 1 ){
						i = f;
						for (j=c;j>=tmp1;j--){
							tab[i][j] = FICHA_H;
							if (j==c){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1--;
			}
		}

		if ( tab[f-1][c-1] == FICHA_M  ){			/*Ficha de humano arriba izquierda*/
			tmp1 = f-2;
			tmp2 = c-2;
			while( tmp1>=0 && tmp2>=0 ){
				if ( tab[tmp1][tmp2] == VACIO )
					break;
				if ( tab[tmp1][tmp2] == FICHA_H ){
					if ( colocar == 1 ){
						cont = f - tmp1;
						for (i=0;i<=cont;i++){
							tab[f-i][c-i] = FICHA_H;
							if (i==0){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1--;
				tmp2--;
			}
		}

	}
	else{
	/*Dentro de este else, cada vez que la maquina intenta colocar una ficha se evalua la linea en donde intenta colocarla, ya sea arriba,
	 * arriba derecha, derecha, abajo derecha, abajo, abajo izquierda, izquierda o arriba izquierda de la ficha, si encuentra una ficha
	 * de maquina en algun lugar de la linea que no sea el lugar mas proximo y no encuentra ningun vacio, entonces, si colocar==1, se
	 * coloca la ficha, se invierten las que se encuentran en la linea (no se coloca ni invierte nada si colocar==0) y ba se pone 1 para
	 * poder retornar 1 al final de la funcion, si no se encuentra nada ba permanece en 0 y se retorna 0 al final de la funcion*/

		if ( tab[f-1][c] == FICHA_H  ){				/*Ficha de maquina arriba*/
			tmp1 = f-2;
			while( tmp1>=0 ){
				if ( tab[tmp1][c] == VACIO )
					break;
				if ( tab[tmp1][c] == FICHA_M ){
					if ( colocar == 1 ){
						j = c;
						for (i=f;i>=tmp1;i--){
							tab[i][j] = FICHA_M;
							if (i==f){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1--;
			}
		}

		if ( tab[f-1][c+1] == FICHA_H  ){			/*Ficha de maquina arriba derecha*/
			tmp1 = f-2;
			tmp2 = c+2;
			while( tmp1>=0 && tmp2<=7 ){
				if ( tab[tmp1][tmp2] == VACIO )
					break;
				if ( tab[tmp1][tmp2] == FICHA_M ){
					if ( colocar == 1 ){
						cont = f - tmp1;
						for (i=0;i<=cont;i++){
							tab[f-i][c+i] = FICHA_M;
							if (i==0){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1--;
				tmp2++;
			}
		}

		if ( tab[f][c+1] == FICHA_H  ){				/*Ficha de maquina derecha*/
			tmp1 = c+2;
			while( tmp1<=7 ){
				if ( tab[f][tmp1] == VACIO )
					break;
				if ( tab[f][tmp1] == FICHA_M ){
					if ( colocar == 1 ){
						i = f;
						for (j=c;j<=tmp1;j++){
							tab[i][j] = FICHA_M;
							if (j==c){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
					}
				tmp1++;
			}
		}

		if ( tab[f+1][c+1] == FICHA_H  ){			/*Ficha de maquina abajo derecha*/
			tmp1 = f+2;
			tmp2 = c+2;
			while( tmp1<=7 && tmp2<=7 ){
				if ( tab[tmp1][tmp2] == VACIO )
					break;
				if ( tab[tmp1][tmp2] == FICHA_M ){
					if ( colocar == 1 ){
						cont = tmp1 - f;
						for (i=0;i<=cont;i++){
							tab[f+i][c+i]=FICHA_M;
							if (i==0){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1++;
				tmp2++;
			}
		}

		if ( tab[f+1][c] == FICHA_H  ){				/*Ficha de maquina abajo*/
			tmp1 = f+2;
			while( tmp1<=7 ){
				if ( tab[tmp1][c] == VACIO )
					break;
				if ( tab[tmp1][c] == FICHA_M ){
					if ( colocar == 1 ){
						j = c;
						for (i=f;i<=tmp1;i++){
							tab[i][j] = FICHA_M;
							if (i==f){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1++;
			}
		}

		if ( tab[f+1][c-1] == FICHA_H  ){			/*Ficha de maquina abajo izquierda*/
			tmp1 = f+2;
			tmp2 = c-2;
			while( tmp1<=7 && tmp2>=0 ){
				if ( tab[tmp1][tmp2] == VACIO )
					break;
				if ( tab[tmp1][tmp2] == FICHA_M ){
					if ( colocar == 1 ){
						cont = tmp1 - f;
						for (i=0;i<=cont;i++){
							tab[f+i][c-i]=FICHA_M;
							if (i==0){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1++;
				tmp2--;
			}
		}

		if ( tab[f][c-1] == FICHA_H  ){				/*Ficha de maquina izquierda*/
			tmp1 = c-2;
			while( tmp1>=0 ){
				if ( tab[f][tmp1] == VACIO )
					break;
				if ( tab[f][tmp1] == FICHA_M ){
					if ( colocar == 1 ){
						i = f;
						for (j=c;j>=tmp1;j--){
							tab[i][j] = FICHA_M;
							if (j==c){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1--;
			}
		}

		if ( tab[f-1][c-1] == FICHA_H  ){			/*Ficha de maquina arriba izquierda*/
			tmp1 = f-2;
			tmp2 = c-2;
			while( tmp1>=0 && tmp2>=0 ){
				if ( tab[tmp1][tmp2] == VACIO )
					break;
				if ( tab[tmp1][tmp2] == FICHA_M ){
					if ( colocar == 1 ){
						cont = f - tmp1;
						for (i=0;i<=cont;i++){
							tab[f-i][c-i] = FICHA_M;
							if (i==0){
								if (imprimir){
									printf("\n\n");
									imprimir_tablero(tab);
								}
							}
						}
					}
					ba=1;
					break;
				}
				tmp1--;
				tmp2--;
			}
		}

	}

	if (ba!=0)
		return 1;								/*Si se encontro algun lugar para colocar la ficha se retorna 1*/
	return 0;									/*Si no se encontro algun lugar para colocar la ficha (si se trata de colocar una ficha
												 *del mismo tipo una a lado de otra, o si no se encuentra alguna ficha del mismo tipo
												 *en la linea), se retorna 0*/
}

void juega_humano(short int *turno){
/*Recibe del humano los valores de fila y columna a donde se desea colocar la ficha, y coloca la ficha llamando a la funcion
 * colocar_fichas, tambien proporciona una interfaz simple para el usuario*/

	char f_aux[2], c_aux[2], ba=0;
	int f, c;

	//printf("*************************************************************************");
	printf("********************************************************************************\n\n");
	printf("\t\t\t      <--JUEGA HUMANO(%c)-->\n\n\n", FICHA_H);

	imprimir_tablero(tablero);

	do{
		if (ba==1){
			printf("\t    -> Posicion de tablero no valida\n");
			f_aux[1]='\0';
			c_aux[1]='\0';
		}
		printf("\t    -> Inserte la fila y columna en donde desea colocar la ficha:\n");
		printf("\t    -> ");
		scanf("%s",f_aux);
		scanf("%s",c_aux);
		f = f_aux[0] - '0';
		c = c_aux[0] - 65;
	} while ( ( ba = f_aux[1]!='\0' || c_aux[1]!='\0' || !colocar_ficha(tablero, f, c, *turno, 1, 1) ) );

	*turno = TURNO_M;							/*Asigno el turno a la maquina para la siguiente jugada*/

}

void juega_maquina(short int *turno, short int altura){
/*Recibe de la maquina los valores de fila y columna a donde colocara la ficha, esto hace creando el arbol de juego, llamando a la funcion
 *AB, y seleccionando la siguiente jugada, tambien proporciona una interfaz simple para el usuario*/

	nodo_t *ptr_arbol, *p;
	int f, c, ba=0;

	//printf("*************************************************************************");
	printf("********************************************************************************\n\n");
    printf("\t\t\t      <--JUEGA MAQUINA(%c)-->\n\n\n", FICHA_M);
	imprimir_tablero(tablero);

	ptr_arbol = construir_arbol(altura);					/*Construyo el arbol*/
	AB(ptr_arbol, altura, -int_MAX, int_MAX);				/*Ejecuto la funcion alfa beta*/

	p = ptr_arbol->hijo;
	while (p) {												/*Elijo la siguiente jugada, eligiendo el 1er hijo de la raiz con valor*/
		if ( p->valor == -(ptr_arbol->valor) ) break;		/*opuesto al de la raiz*/
		p = p->sgte;
	}

	/*Comparo el nuevo tablero con el tablero actual para pasar la fila y la columna a la funcion colocar_fichas*/
	for (f=0;f<8;f++){
		for (c=0;c<8;c++){
			if ( tablero[f][c]==VACIO && p->tablero[f][c]!=VACIO ){
				colocar_ficha(tablero, f, c, *turno, 1, 1);
				ba=1;
				break;
			}
		}
		if (ba==1) break;
	}

	liberar_arbol(ptr_arbol);								/*Libero la memoria ocupada por el arbol*/
	*turno = TURNO_H;										/*Asigno el turno a el humano para la siguiente jugada*/
	
	printf("\nFICHA PUESTA EN: Fila=%d, Columna=%c\n",f,c+65);

}

void inicio(short int *turno){
/*Desplega un mensaje inicial, y recibe el modo de juego de parte del usuario para asignar el turno inicial, tambien asigna las fichas
 * correspondientes al humano y a la maquina*/

	char opcion[2];
	int ba=0;

	printf("********************************************************************************\n\n\n");
	printf("\t\t\tBIENVENIDO AL JUEGO DE OTHELLO!!!\n\n");
	printf("\n\t\t\t  Desarrollado por Guido Nunez\n\n\n");

	do{
		if (ba==1){
			printf("\t    -> Valor no valido\n");
			opcion[1]='\0';
		}
		printf("\t    -> Elija el modo de juego:\n");
		printf("\t       a: Humano contra maquina\n");
		printf("\t       b: Maquina contra humano\n");
		printf("\t    -> ");
		scanf("%s",opcion);
	} while( (ba = ( opcion[1]!='\0' || (opcion[0]!='a' && opcion[0]!='b') ) ) );

	if ( opcion[0] == 'a' ){
		*turno = TURNO_H;
	}
	else {
		*turno = TURNO_M;
	}

	if ( *turno == TURNO_H ){
		FICHA_H = 'X';
		FICHA_M = 'O';
	}
	else{
		FICHA_H = 'O';
		FICHA_M = 'X';
	}

	printf("\n\n");
}

int fin_juego(short int *turno, int *no_movio ){
/*Analiza el tablero luego de una jugada, si el humano o la maquina pueden colocar fichas retorna 0, si el humano no puede colocar fichas
 * pasa el turno a la maquina y retorna 0, si la maquina no puede colocar fichas pasa el turno al humano y retorna 0, si tanto el humano
 * como la maquina no pueden colocar fichas no_movio=2 y retorna 1*/

	int i, j, ba_colocar=0;

	/*Analizo el tablero para ver si se puede hacer movimientos llamando a la funcion colocar_fichas con el parametro colocar=0 , si
	 * hay movimientos ba_colocar se pone en 1, si no hay movimientos se mantiene en 0*/
	for (i=0;i<8;i++){
		for (j=0;j<8;j++){
			if ( colocar_ficha(tablero, i, j, *turno, 0, 1) ){
				ba_colocar = 1;
				break;
			}
		}
		if ( ba_colocar == 1 ) break;
	}

	/*Si no se puede hacer movimientos (ba_colocar==0), el turno pasa al contrario y se incrementa no_movio, si hay movimientos
	 *(ba_colocar==1) el juego continua normalmente y no_movio=0*/
	if ( ba_colocar == 0 ){
		if ( *turno == TURNO_H )
			*turno = TURNO_M;
		else
			*turno = TURNO_H;
		*no_movio = *no_movio + 1;
	}
	else{
		*no_movio = 0;
	}

	/*Si no_movio==2 tanto el humano como la maquina no pueden mover y se retorna 1 para finalizar el juego*/
	if ( *no_movio == 2 ) return 1;
	return 0;

}

void resultado(){
/*Analiza el tablero para ver quien es el ganador de la partida, imprime un mensaje indicando el ganador o si se ha producido un empate*/

	int i, j;
	int CONT_H=0, CONT_M=0;		/*Contadores de fichas del humano y de la maquina*/

	for (i=0;i<8;i++){
		for (j=0;j<8;j++){
			if ( tablero[i][j] == FICHA_H ){
				CONT_H++;
			}
			else if ( tablero[i][j] == FICHA_M ){
				CONT_M++;
			}
		}
	}

    printf("********************************************************************************\n\n\n");
	imprimir_tablero(tablero);

	if (CONT_H > CONT_M){
        printf("********************************************************************************");
		printf("\t\t\tHUMANO HA GANADO EL JUEGO!!!\n");
        printf("********************************************************************************\n");
	} else if ( CONT_M > CONT_H ) {
        printf("********************************************************************************");
		printf("\t\t\tMAQUINA HA GANADO EL JUEGO!!!\n");
        printf("********************************************************************************\n");
	} else{
        printf("********************************************************************************");
		printf("\t\t\tSE HA PRODUCIDO UN EMPATE!!!\n");
        printf("********************************************************************************\n");
	}

}

nodo_t *construir_arbol(short int altura){
/*Crea el nodo raiz del arbol y llamo a la funcion expandir_arbol para crear un sub-arbol cuya raiz es la raiz creada en esta funcion*/

	nodo_t *ptr_arbol;
	ptr_arbol = (nodo_t*)malloc(sizeof(nodo_t));

	copiar_tablero(tablero, ptr_arbol->tablero);

	ptr_arbol->turno = TURNO_M;
	ptr_arbol->hijo = NULL;
	ptr_arbol->sgte = NULL;

	expandir_arbol(ptr_arbol, 0, altura);

	return (ptr_arbol);

}

void expandir_arbol(nodo_t *p, int nivel, short int altura){
/*Crea todos los nodos del arbol, yendo hacia abajo mediante los hijos de los nodos y creando una lista enlazada de nodos para cada padre
 * llamando a la funcion generar_arbol, para cada nodo de las listas enlazadas se repite el proceso*/

	nodo_t *q;
	char tablero_padre[8][8];
	short int turno_padre;

	copiar_tablero(p->tablero, tablero_padre);
	turno_padre = p->turno;

	if ( nivel < altura ){
		q = generar_arbol(tablero_padre, turno_padre);
		p->hijo = q;
		while ( q != NULL ){
			if (p->turno == TURNO_M)
				q->turno = TURNO_H;
			else
				q->turno = TURNO_M;
			q->hijo = NULL;
			expandir_arbol(q, nivel+1,altura);
			q = q->sgte;
		}

	}


}

nodo_t *generar_arbol(char tablero_padre[8][8], short int turno_padre){
/*Devuelve un puntero a una lista enlazada que contiene los nodos que se pueden obtener de un nodo padre, los nodos se crean cuando se
 * encuentra una posicion en el tablero donde se puede colocar una ficha, para crear y enlazar los nodos se usa dos funciones auxiliares
 * crear_nodo y agregar_nodo*/

	nodo_t *ptr = NULL;
	nodo_t *nuevo_nodo;
	int i, j;

	for (i=0;i<8;i++){
		for (j=0;j<8;j++){
			if( colocar_ficha(tablero_padre, i, j, turno_padre, 0, 1) ){
				nuevo_nodo = crear_nodo(tablero_padre, i, j, turno_padre);
				agregar_nodo(&ptr, nuevo_nodo);
			}
		}
	}

	return (ptr);
}

void liberar_arbol(nodo_t* p){
/*Libera la memoria de todos los nodos del arbol*/
	nodo_t * actual = p;
	if ( p->hijo != NULL)
		liberar_arbol(p->hijo);
	if ( actual->sgte != NULL)
		liberar_arbol( actual->sgte );
	free(p);
}

nodo_t *crear_nodo(char tablero_padre[8][8], int i, int j, short int turno){
/*Crea un nuevo nodo con la nueva disposicion del tablero y apuntando a null*/
	nodo_t *p;
	p = (nodo_t*)malloc(sizeof(nodo_t));
	copiar_tablero(tablero_padre, p->tablero);
	colocar_ficha(p->tablero, i, j, turno, 1, 0);
	p->sgte = NULL;
	return (p);
}

void agregar_nodo(nodo_t **ptr, nodo_t *nuevo_nodo){
/*Enlaza el nuevo nodo en la lista enlazada, si la lista esta vacia lo coloca al inicio, si no esta vacia, lo coloca el final de la lista*/
	nodo_t *t;
	if (*ptr == NULL ){
		(*ptr) = nuevo_nodo;
	}
	else{
		t = *ptr;
		while(t->sgte){
			t = t->sgte;
		}
		t->sgte = nuevo_nodo;
	}
}

void copiar_tablero(char a[8][8], char b[8][8]){
/*copia el tablero a en b*/
	int i, j;
	for (i=0;i<8;i++){
		for (j=0;j<8;j++){
			b[i][j] = a[i][j];
		}
	}
}

int AB(nodo_t *x, int nivel, int LB, int D){
/*Funcion alfa beta. Recorre el arbol y asigna valores a los nodos a partir de los valores dados por la funcion eval, realiza los cortes
 *correspondientes*/
	nodo_t *hijo_i = x->hijo;
	if ( x->hijo == NULL || nivel == 1) {
		x->valor = eval(x->tablero);
		return x->valor;
	}
	x->valor = LB;
	while (hijo_i != NULL) {
		x->valor = max( x->valor, -(AB(hijo_i, nivel-1, -D, -(x->valor))) );
		if (x->valor >= D)
			return x->valor;
		hijo_i = hijo_i->sgte;
	}
	return x->valor;
}

int max(int a, int b){
/*Retorna el mayor entre a y b, si son iguales retorna a*/
	if (a >= b)
		return a;
	else
		return b;
}

int eval(char tablero_actual[8][8]){
/*Funcion de evaluacion. Dada una configuracion de tablero calcula el puntaje. Hace uso de las funciones dif_cantidad_de_fichas y
 *dif_fichas_estables */
	int puntos;
	/*El puntaje es la diferencia entre las fichas de la maquina y las fichas del humano sumado a la diferencia entre la estabilidad de la
	 *maquina y la estabilidad del humano, este ultimo termino multiplicamos por 10 debido a que tiene mayor importancia en el momento de
	 *mejorar una jugada*/
	puntos = dif_cantidad_de_fichas(tablero_actual) + (dif_fichas_estables(tablero_actual) * 10) ;
	return puntos;
}


int dif_cantidad_de_fichas(char tablero_actual[8][8]){
/*Retorna la diferencia entre la cantidad de fichas de la maquina y la cantidad de fichas del humano*/
	int i, j, diferencia;
	int cont_M=0, cont_H=0;
	for (i=0;i<8;i++){
		for (j=0;j<8;j++){
			if ( tablero_actual[i][j] == FICHA_M )
				cont_M++;
			else if ( tablero_actual[i][j] == FICHA_H)
				cont_H++;
		}
	}
	diferencia = cont_M - cont_H;
	if (diferencia==0) diferencia=1;
	return (diferencia);
}

int dif_fichas_estables(char tablero_actual[8][8]){
/*Retorna la diferencia entre el puntaje de estabilidad(fichas estables) de la maquina y del humano, hace uso de la tabla pesos*/
	int i, j, diferencia;
	int estabilidad_M=0, estabilidad_H=0;
	for (i=0;i<8;i++){
		for (j=0;j<8;j++){
			if ( tablero_actual[i][j] == FICHA_M )
				estabilidad_M = estabilidad_M + pesos[i][j];
			else if ( tablero_actual[i][j] == FICHA_H)
				estabilidad_H = estabilidad_H + pesos[i][j];
		}
	}
	diferencia = estabilidad_M - estabilidad_H;
	return (diferencia);
}


