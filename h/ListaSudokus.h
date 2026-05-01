#pragma once

#include "reglas_sudoku.h"

class ListaSudokus {
private:
  tReglasSudoku **arraySudokus;
  int cont;
  int size;

  void resize();
  // Añadimos metodos privado sde ordenacion
  void colocar(int i);

public:
  // CoNntrutoras
  ListaSudokus();
  ~ListaSudokus();

  // Operadores
  tReglasSudoku &operator[](int indice);

  int dame_num_elems();
  const tReglasSudoku &dame_sudoku(int i);
  void insertar(const tReglasSudoku &sudoku);
  void eliminar(int pos); // elimina el elemento de la posición pos


};
ostream& operator<<(ostream& out, ListaSudokus& lista);