#pragma once

#include "tablero.h"
#include <fstream>
#include <iostream>

using namespace std;

class tReglasSudoku {
private:
  typedef struct {
    int fila;
    int columna;
  } tPosicion;

  typedef struct {
    bool posible;
    int celdas_que_afectan;
  } tValor;

  typedef struct {
    int nFilas;
    int nColumnas;
    tValor valores[MAX][MAX][MAX];
  } tValores;

  typedef struct {
    tPosicion *lista[MAX * MAX];
    int cont;
  } tListaBloqueos;

  typedef int arrValores[MAX];
  arrValores cuantas_celdas;

  tListaBloqueos lista;
  tValores valores_celda;
  tTablero tablero;
  int cont;

  int celdas_afectadas(int f, int c, int afectadas[][2]) const;
  void actualizar_al_poner(int f, int c, int v);
  void inicializar_valores();
  void actualizar_al_quitar(int f, int c, int v);

  bool coord_valid(const int &f, const int &c) const;
  void actualizar_bloqueos();

  void recalcular_cuantas_celdas();

public:
  tReglasSudoku();
  tReglasSudoku(const tReglasSudoku &sudoku); // Constructora por copia
  ~tReglasSudoku();                           // Destructora

  // Operadores
  tReglasSudoku &operator=(const tReglasSudoku &reglas);
  bool operator<(const tReglasSudoku &s2) const;
  bool operator==(const tReglasSudoku &s2) const;

  int dame_dimension() const;         // devuelve la dimensión del tablero
  int dame_celda(int f, int c) const; // devuelve la celda en la posición (f,c)
  bool terminado() const; // true si y sólo si el Sudoku está resuelto
  bool bloqueo() const;   // true si el Sudoku tiene celdas bloqueadas
  int dame_num_celdas_bloqueadas()
      const;                          // devuelve el número de celdas bloqueadas
  int dame_num_celdas_vacias() const; // devuelve el número de celdas vacías
  void dame_celda_bloqueada(int p, int &f, int &c)
      const; // devuelve en (f,c) la celda bloqueada en la posición p
  bool es_valor_posible(int f, int c, int v)
      const; // true si y sólo si v se puede colocar en (f,c)
  int posibles_valores(int f, int c, int valores[] = nullptr)
      const; // devuelve el número de posibles valores para (f,c) y
             // opcionalmente los escribe en valores[]
  // modificadoras
  bool pon_valor(int f, int c, int v); // pone v en (f,c)
  bool quita_valor(int f, int c);      // pone la celda (f,c) a VACIA
  void reset();                        // recupera el Sudoku original
  void autocompletar(); // rellena todas las celdas con un único valor posible
  // inicializar sudoku
  bool carga_sudoku(ifstream &); // carga un Sudoku original de un archiv
  int cuantas_celdas_pueden_tener(int n_valores) const;
};