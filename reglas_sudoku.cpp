#include "h/reglas_sudoku.h"

#include <cmath>
#include <iostream>

//
tReglasSudoku::tReglasSudoku() {
  cont = 0;
  lista.cont = 0;
  valores_celda.nFilas = 0;
  valores_celda.nColumnas = 0;

  for (int i = 0; i < MAX; i++) {
    cuantas_celdas[i] = 0;
  }
}

// [MOdificado] Declaramos destructora
tReglasSudoku::~tReglasSudoku() {

  // Como es un array estatico, la  borra cada puntero
  // individualmente
  for (int i = 0; i < lista.cont; i++) {
    delete lista.lista[i];
    lista.lista[i] = nullptr;
  }
  cont = 0;
  lista.cont = 0;
}

tReglasSudoku::tReglasSudoku(const tReglasSudoku &sudoku) {
  // Copiamos el contenido de uno a otro
  cont = sudoku.cont;
  tablero = sudoku.tablero;
  lista.cont = sudoku.lista.cont;
  valores_celda = sudoku.valores_celda;

  // Creamos nuevos espacio de memoria par cada uno
  for (int i = 0; i < lista.cont; i++) {
    lista.lista[i] = new tPosicion;
    *lista.lista[i] = *sudoku.lista.lista[i];
  }
  //[MODIFICADO] Copiar el array de la refactorizaicion
  for (int i = 0; i < MAX; i++) {
    cuantas_celdas[i] = sudoku.cuantas_celdas[i];
  }

} // Constructora por copia

tReglasSudoku &tReglasSudoku::operator=(const tReglasSudoku &reglas) {
  if (this !=
      &reglas) { // Si los dos sudokus tienen direcciones de memoria distintas
    // Evita autoasignacion
    // Se libera memoria ocupada por this
    for (int i = 0; i < lista.cont; i++) {
      delete lista.lista[i];
      lista.lista[i] = nullptr;
    }
    // Copiamos los datos
    lista.cont = 0;
    cont = reglas.cont;
    tablero = reglas.tablero;
    valores_celda = reglas.valores_celda;

    lista.cont = reglas.lista.cont;
    for (int i = 0; i < lista.cont; i++) {
      lista.lista[i] = new tPosicion;
      *lista.lista[i] = *reglas.lista.lista[i];
    }
    //[MODIFICADO] Copiar el array de la refactorizaicion
    for (int i = 0; i < MAX; i++) {
      cuantas_celdas[i] = reglas.cuantas_celdas[i];
    }
  }
  return *this; // retornamos el sudoku
}

bool tReglasSudoku::operator<(const tReglasSudoku &s2) const {
  bool es_menor = false;
  bool decidido = false;

  // Un Sudoku es menor si tiene menos celdas vacías
  if (dame_num_celdas_vacias() != s2.dame_num_celdas_vacias()) {
    es_menor = dame_num_celdas_vacias() < s2.dame_num_celdas_vacias();
    decidido = true; // Ya tenemos la respuesta, no hace falta mirar más
  } 
  // Si tienen las mismas vacías, miramos el marcador de la refactorización
  else {
    int dim = dame_dimension();
    
    // El bucle se detendrá si llegamos al final O si 'decidido' pasa a ser true
    for (int k = 1; k <= dim && !decidido; k++) {
      
      int cont1 = cuantas_celdas_pueden_tener(k);
      int cont2 = s2.cuantas_celdas_pueden_tener(k);

      // En cuanto hay una diferencia, tomamos la decisión
      if (cont1 != cont2) {
        es_menor = cont1 > cont2;
        decidido = true; // Aborta el bucle de forma limpia
      }
    }
  }

  // Único punto de salida
  return es_menor;
}
bool tReglasSudoku::operator==(const tReglasSudoku &s2) const {
  // *this representa al sudoku actual (el de la izquierda de la comparación)
  return !(*this < s2) && !(s2 < *this);
}

int tReglasSudoku::dame_dimension() const { return tablero.dame_dimension(); }

int tReglasSudoku::dame_celda(int g, int c) const {
  return tablero.dame_elem(g, c).dame_valor();
}

bool tReglasSudoku::terminado() const {
  return dame_dimension() * dame_dimension() == cont;
}

bool tReglasSudoku::bloqueo() const { return lista.cont != 0; }

int tReglasSudoku::dame_num_celdas_bloqueadas() const { return lista.cont; }

int tReglasSudoku::dame_num_celdas_vacias() const {
  return dame_dimension() * dame_dimension() - cont;
}

void tReglasSudoku::dame_celda_bloqueada(int p, int &f, int &c) const {
  if (p >= 0 && p < lista.cont) {
    f = lista.lista[p]->fila;
    c = lista.lista[p]->columna;
  } else {
    f = -1;
    c = -1;
  }
}
// MEJORA DE IMPLEMENTACION
// Rellena en 'afectadas' todas las posiciones que ve (f,c)
// Devuelve el número de posiciones
int tReglasSudoku::celdas_afectadas(int f, int c, int afectadas[][2]) const {
  int dim = dame_dimension();
  int n = 0;
  int dim_sub = (int)sqrt((double)dim);
  int startF = (f / dim_sub) * dim_sub;
  int startC = (c / dim_sub) * dim_sub;

  for (int col = 0; col < dim; col++)
    if (col != c) {
      afectadas[n][0] = f;
      afectadas[n][1] = col;
      n++;
    }

  for (int fila = 0; fila < dim; fila++)
    if (fila != f) {
      afectadas[n][0] = fila;
      afectadas[n][1] = c;
      n++;
    }

  for (int fi = startF; fi < startF + dim_sub; fi++)
    for (int ci = startC; ci < startC + dim_sub; ci++)
      if (fi != f && ci != c) {
        afectadas[n][0] = fi;
        afectadas[n][1] = ci;
        n++;
      }

  return n;
}

void tReglasSudoku::inicializar_valores() {
  int dim = dame_dimension();

  // Todo posible, contadores a 0
  for (int f = 0; f < dim; f++)
    for (int c = 0; c < dim; c++)
      for (int v = 0; v < dim; v++) {
        valores_celda.valores[f][c][v].posible = true;
        valores_celda.valores[f][c][v].celdas_que_afectan = 0;
      }

  // Para cada celda con valor ya puesto, actualizar sus afectadas
  for (int f = 0; f < dim; f++)
    for (int c = 0; c < dim; c++)
      if (!tablero.dame_elem(f, c).es_vacia())
        actualizar_al_poner(f, c, tablero.dame_elem(f, c).dame_valor());
}

void tReglasSudoku::actualizar_al_poner(int f, int c, int v) {
  int afectadas[MAX * MAX][2];
  int n = celdas_afectadas(f, c, afectadas);
  int idx = v - 1; // los valores van de 1..dim, el array de 0..dim-1

  for (int i = 0; i < n; i++) {
    int fi = afectadas[i][0];
    int ci = afectadas[i][1];

    valores_celda.valores[fi][ci][idx].celdas_que_afectan++;
    valores_celda.valores[fi][ci][idx].posible = false;
  }
}

void tReglasSudoku::actualizar_al_quitar(int f, int c, int v) {
  int afectadas[MAX * MAX][2];
  int n = celdas_afectadas(f, c, afectadas);
  int idx = v - 1;

  for (int i = 0; i < n; i++) {
    int fi = afectadas[i][0];
    int ci = afectadas[i][1];
    valores_celda.valores[fi][ci][idx].celdas_que_afectan--;
    if (valores_celda.valores[fi][ci][idx].celdas_que_afectan == 0)
      valores_celda.valores[fi][ci][idx].posible = true;
  }
}

bool tReglasSudoku::coord_valid(const int &f, const int &c) const {
  int dim = dame_dimension();
  return f >= 0 && c >= 0 && f < dim && c < dim;
}

void tReglasSudoku::actualizar_bloqueos() {
  int dim = dame_dimension();

  // Borramos los bloqueos antiguos para no dejar basura en memoria
  for (int c = 0; c < lista.cont; c++) {
    delete lista.lista[c];
    lista.lista[c] = nullptr;
  }
  lista.cont = 0;
  for (int i = 0; i < dim; i++) {
    for (int j = 0; j < dim; j++) {
      if (tablero.dame_elem(i, j).es_vacia() && posibles_valores(i, j) == 0) {
        if (lista.cont < MAX * MAX) {
          lista.lista[lista.cont] = new tPosicion;
          lista.lista[lista.cont]->fila = i;
          lista.lista[lista.cont]->columna = j;
          lista.cont++;
        }
      }
    }
  }
}

bool tReglasSudoku::es_valor_posible(int f, int c, int v) const {
  // Es valor posible pasa de recorrer a consultar
  bool posible;
  int n = dame_dimension();
  if (!coord_valid(f, c)) {
    posible = false;
  } else if (!tablero.dame_elem(f, c).es_vacia()) {
    posible = false;
  } else if (v < 1 || v > n) {
    posible = false;
  } else {
    posible = valores_celda.valores[f][c][v - 1].posible;
  }

  return posible;
}

int tReglasSudoku::posibles_valores(int f, int c, int valores[]) const {
  int num_posibles = 0;
  int dim = dame_dimension();
  if (coord_valid(f, c) && tablero.dame_elem(f, c).es_vacia()) {
    for (int v = 1; v <= dame_dimension(); v++) {
      if (valores_celda.valores[f][c][v - 1].posible) {
        if (valores != nullptr)
          valores[num_posibles] = v;
        num_posibles++;
      }
    }
  }

  return num_posibles;
}

void tReglasSudoku::autocompletar() {
  int dim = dame_dimension();

  for (int i = 0; i < dim; i++) {
    for (int j = 0; j < dim; j++) {
      tCelda c = tablero.dame_elem(i, j);

      if (c.es_vacia()) {

        int numPosibles = 0;
        int valores[MAX] = {0};
        int v = 1;

        numPosibles = posibles_valores(i, j, valores);

        if (numPosibles == 1) {
          pon_valor(i, j, valores[0]);
        }
      }
    }
  }
}

bool tReglasSudoku::pon_valor(int f, int c, int v) {
  bool colocado = false;

  int dim = dame_dimension();
  if (coord_valid(f, c)) {
    tCelda celda = tablero.dame_elem(f, c); // pone v en (f,c)
    if (celda.es_vacia() && es_valor_posible(f, c, v)) {
      celda.set_valor(v);
      celda.set_ocupada();
      tablero.colocar_celda(f, c, celda);
      cont++;
      colocado = true;
    }
  }

  if (colocado) {
    actualizar_al_poner(f, c, v);
    actualizar_bloqueos();
    // [MODIFICADO]
    // Segun la refactorizacion tambien cada vez que usamos pon_valor recalculamos cuantas_celdas
    recalcular_cuantas_celdas();
  }

  return colocado;
}
bool tReglasSudoku::quita_valor(int f, int c) {
  bool quitado = false;

  int dim = dame_dimension();

  if (coord_valid(f, c)) {
    tCelda celda = tablero.dame_elem(f, c); // pone v en (f,c)
    int v = celda.dame_valor();
    if (celda.es_ocupada()) {

      celda.set_valor(0);
      celda.set_vacia();
      tablero.colocar_celda(f, c, celda);
      cont--;
      quitado = true;
    }

    //[MODIFICADO] --> Refactorizacion
    // Llamamos recalcular celdas
    if (quitado) {
      actualizar_al_quitar(f, c, v);
      actualizar_bloqueos();
      recalcular_cuantas_celdas();
    }
  }

  return quitado;

} // pone la celda (f,c) a VACIA
void tReglasSudoku::reset() {
  int dim = dame_dimension();

  for (int i = 0; i < dame_dimension(); i++) {
    for (int j = 0; j < dame_dimension(); j++) {
      if (!tablero.dame_elem(i, j).es_original())
        quita_valor(i, j);
    }
  }
}

bool tReglasSudoku::carga_sudoku(ifstream &input) {
  bool cargado = false;
  tCelda c;

  cont = 0;
  lista.cont = 0;

  int dim;
  input >> dim;

  cargado = (dim == 4 || dim == 9);

  if (cargado) {
    tablero = tTablero(dim);

    for (int i = 0; i < dim; i++) {
      for (int j = 0; j < dim; j++) {
        int v;
        input >> v;
        if (v != 0) {

          c.set_valor(v);
          c.set_original();
          tablero.colocar_celda(i, j, c);
          cont++;
        } else {
          c.set_valor(0);
          c.set_vacia();
          tablero.colocar_celda(i, j, c);
        }
      }
    }
    inicializar_valores();
    actualizar_bloqueos();
    recalcular_cuantas_celdas();  // [MODIFICADO] refectorizacion
  }

  return cargado;
}

int tReglasSudoku::cuantas_celdas_pueden_tener(int n_valores) const {
  int cantidad = 0;

  if (n_valores >= 1 && n_valores <= dame_dimension()) {
    cantidad = cuantas_celdas[n_valores - 1]; // Recuerda el -1 para evitar el crash
  }

  // Único punto de salida
  return cantidad;
}

void tReglasSudoku::recalcular_cuantas_celdas() {
  for (int i = 0; i < MAX; i++) {
    cuantas_celdas[i] = 0;
  }

  // Guardamos la dimensión en una variable local para no llamar a la función 81
  // veces.
  int dim = dame_dimension();

  // 2. Recorremos el tablero UNA sola vez.
  for (int fila = 0; fila < dim; fila++) {
    for (int columna = 0; columna < dim; columna++) {

      if (dame_celda(fila, columna) == 0) {
        // Calculamos sus valores posibles una única vez por celda
        int num_posibles = posibles_valores(fila, columna);

        // Si tiene valores posibles (y está en rango), incrementamos su
        // contador
        if (num_posibles > 0 && num_posibles <= dim) {
          cuantas_celdas[num_posibles - 1]++;
        }
      }
    }
  }
}
