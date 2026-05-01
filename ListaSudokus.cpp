#include "h/ListaSudokus.h"

ListaSudokus::ListaSudokus() : cont(0), size(2) {
  arraySudokus = new tReglasSudoku *[size];

  for (int i = 0; i < size; i++) {
    arraySudokus[i] = nullptr;
  }
}

ListaSudokus::~ListaSudokus() {
  for (int i = 0; i < cont; i++) {
    delete arraySudokus[i];
  }
  delete[] arraySudokus;
  arraySudokus = nullptr;
  cont = 0;
  size = 0;
}

ostream& operator<<(ostream& out, ListaSudokus& lista) {
  for (int k = 0; k < lista.dame_num_elems(); k++) {
    int dim = lista.dame_sudoku(k).dame_dimension();
    
    out << k + 1 << ": Sudoku con "
         << lista.dame_sudoku(k).dame_num_celdas_vacias() << " casillas vacias."
         << endl;
         
    for (int c = 1; c <= dim; c++) {
      int contCeldas = lista.dame_sudoku(k).cuantas_celdas_pueden_tener(c);
      out << "   celdas con " << c << " valores posibles: " << contCeldas << endl;
    }
  }
  return out;
}

tReglasSudoku &ListaSudokus::operator[](int indice) {
  return *arraySudokus[indice];
}

void ListaSudokus::resize() {
  size *= 2;
  // Creamos un nuevo array del doble d edimensio nqu el original
  tReglasSudoku **nuevoArray = new tReglasSudoku *[size];
  // Copiamos los elementos del aarray viejo a lnuevo
  for (int i = 0; i < cont; i++) {
    nuevoArray[i] = arraySudokus[i];
  }
  delete[] arraySudokus;
  arraySudokus = nuevoArray;
}

int ListaSudokus::dame_num_elems() { return cont; }

const tReglasSudoku &ListaSudokus::dame_sudoku(int i) {
  return *arraySudokus[i];
}

void ListaSudokus::colocar(int i) {
  // Guardamos el PUNTERO del sudoku que queremos ordenar, no una copia
  tReglasSudoku *elemento = arraySudokus[i];

  int j = i;
  // Usamos el operador < desreferenciando los punteros
  while (j > 0 && *elemento < *arraySudokus[j - 1]) {
    arraySudokus[j] = arraySudokus[j - 1];
    j--;
  }

  // ¡FALTABA ESTO! Colocamos el puntero en su posición final definitiva
  arraySudokus[j] = elemento;
}

void ListaSudokus::insertar(const tReglasSudoku &sudoku) {
  // Si no hay espacio usamos resize
  if (cont == size) {
    resize();
  }
  // E insertamos
  arraySudokus[cont] = new tReglasSudoku(sudoku);
  cont++;

  // Y justo despues de insertar un elemento en la ultima posicion disponible
  // ordenamos Si es que es necesario ordenar
    colocar(cont - 1);
  
}

void ListaSudokus::eliminar(int pos) {
  delete arraySudokus[pos];

  for (int i = pos; i < cont - 1; i++) {
    arraySudokus[i] = arraySudokus[i + 1];
  }

  arraySudokus[cont - 1] = nullptr;
  cont--;
} // elimina el elemento de la posición pos


