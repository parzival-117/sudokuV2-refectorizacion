#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "h/ListaSudokus.h"
#include "h/colors.h"

using namespace std;

enum tError { ninguno, opciones, valor, bloqueada, original, vacia, ocupada };

void mostrar_sudoku(const tReglasSudoku &reglas, const tError error);
void poner_valor(tReglasSudoku &reglas, tError &error);
void quitar_valor(tReglasSudoku &reglas, tError &error);
int posibles_valores(tReglasSudoku &reglas, tError &error);
void cargar_listaSudokus(ListaSudokus &sudokus_nuevos,
                         ListaSudokus &partidas_guardadas,
                         tReglasSudoku &reglas);

void pausar();
char mayus(char op);

// Funciones main

int menu(); // Muestra el menu del juego NUeva partida Continuar partida y A salir

bool seleccionar_partida(char tipo, ListaSudokus &nuevos,
                         ListaSudokus &guardados, tReglasSudoku &partida,
                         int &indice);
void ejecutar_partida(tReglasSudoku &partida, tError &error);

void guardar_partidas(const ListaSudokus &partidas_guardadas);
void serializar_sudoku(ofstream &archivo, const tReglasSudoku &reglas);
void gestionar_final_partida(tReglasSudoku &partida, bool es_nueva, int indice,
                             ListaSudokus &guardados);

bool resolver_sudoku(tReglasSudoku &sudoku, int fila, int columna);

int main() {
  tReglasSudoku reglas;
  ListaSudokus sudokus_nuevos;
  ListaSudokus partidas_guardadas;

  // Inicialización
  cargar_listaSudokus(sudokus_nuevos, partidas_guardadas, reglas);

  tError error = ninguno;
  char op = ' ';

  // Bucle Principal de la Aplicación
  do {
    cout << "\nPartida nueva (N), continuar partida (C) o abandonar (A)? ";
    cin >> op;
    op = mayus(op);

    if (op == 'N' || op == 'C') {
      tReglasSudoku partida;
      int indice = -1;
      bool partida_nueva = (op == 'N');

      // Selección de la partida
      bool listo_para_jugar = seleccionar_partida(
          op, sudokus_nuevos, partidas_guardadas, partida, indice);

      if (listo_para_jugar) {
        // Bucle de juego
        ejecutar_partida(partida, error);

        // Gestión post-partida (Guardar o Eliminar si terminó)
        gestionar_final_partida(partida, partida_nueva, indice,
                                partidas_guardadas);
      }
    }
  } while (op != 'A');

  // 3. Finalización y Guardado
  guardar_partidas(partidas_guardadas);
  cout << "¡Hasta pronto!" << endl;

  return 0;
}

char mayus(char op) {
  switch (op) {
  case 'n': op = 'N'; break;
  case 'c': op = 'C'; break;
  case 'a': op = 'A'; break;
  default: break;
  }
  return op;
}

int menu() {
  int opcion = -1;

  cout << "\n";
  cout << YELLOW << "------------- MENU -------------" << RESET << "\n";
  cout << "1. Poner valor\n";
  cout << "2. Quitar valor\n";
  cout << "3. Reset\n";
  cout << "4. Posibles valores de una celda vacia\n";
  cout << "5. Autocompletar celdas con valor unico\n";
  cout << "6. Resolver el sudoku\n";
  cout << "7. Salir\n";
  cout << YELLOW << "--------------------------------" << RESET << "\n";
  cout << "Opcion: ";

  cin >> opcion;

  // Por si se introducen letras, para que no se produzcan bucles infinitos
  if (cin.fail()) {
    cin.clear();
    cin.ignore(10000, '\n');
    opcion = -1; // Marcamos como opción inválida
  }

  return opcion;
}

bool seleccionar_partida(char tipo, ListaSudokus &nuevos,
                         ListaSudokus &guardados, tReglasSudoku &partida,
                         int &indice) {
  bool exito = false;
  int aux;

  if (tipo == 'N') {
    cout << nuevos;
    cout << "Elige un sudoku: ";
  } else {
    if (guardados.dame_num_elems() == 0) {
      cout << "No hay partidas guardadas." << endl;
      return false;
    }
    cout << guardados;
    cout << "Elige una partida: ";
  }

  cin >> aux;
  if (cin.fail()) {
    cin.clear();
    cin.ignore(10000, '\n');
    aux = -1;
  }

  ListaSudokus &lista_actual = (tipo == 'N') ? nuevos : guardados;

  if (aux >= 1 && aux <= lista_actual.dame_num_elems()) {
    indice = aux - 1;
    partida = lista_actual.dame_sudoku(indice);
    exito = true;
  } else {
    cout << BG_RED << "VALOR INCORRECTO: No existe esa seleccion." << RESET
         << endl;
  }

  return exito;
}

void ejecutar_partida(tReglasSudoku &partida, tError &error) {
  int opcion = 0;

  // Mientras el usuario no quiera salir y el sudoku no esté terminado
  while (opcion != 7 && !partida.terminado()) {

    // Mostramos cómo está el tablero y si hubo errores antes
    mostrar_sudoku(partida, error);

    // Limpiamos el error para que no se repita en la siguiente vuelta
    error = ninguno;

    // Pedimos la opción al usuario
    opcion = menu();

    // Ejecutamos la acción
    switch (opcion) {
    case 1:
      poner_valor(partida, error);
      break;
    case 2:
      quitar_valor(partida, error);
      break;
    case 3:
      partida.reset();
      break;
    case 4:
      posibles_valores(partida, error);
      pausar();
      break;
    case 5:
      partida.autocompletar();
      break;
    case 6:
      resolver_sudoku(partida, 0, 0);
      break;
    case 7:
      break;
    default:
      error = opciones;
      break; // Aquí se genera el error de opción inválida
    }
  }
}

void gestionar_final_partida(tReglasSudoku &partida, bool es_nueva, int indice,
                             ListaSudokus &guardados) {
  if (partida.terminado()) {
    mostrar_sudoku(partida, ninguno);
    if (!es_nueva) {
      guardados.eliminar(indice);
    }
  } else {
    // Si no terminó, la insertamos/actualizamos en guardados
    if (!es_nueva) {
      guardados.eliminar(indice);
    }
    guardados.insertar(partida);
    cout << "\n[Info] Partida sincronizada en lista de guardadas.\n";
  }
}

bool resolver_sudoku(tReglasSudoku &sudoku, int fila, int columna) {
  int dim = sudoku.dame_dimension();

  // Si llegamos al fina lde las filas está resuelto
  if (fila == dim) {
    return true;
  }
  // Si llegamos al final de las columnas saltamos de fila
  if (columna == dim) {
    return resolver_sudoku(sudoku, fila + 1, 0);
  }
  // Si la celda ya contiene un numero, pasamos a la siguiente celda ->
  // columna
  // + 1
  if (sudoku.dame_celda(fila, columna) != 0) {
    return resolver_sudoku(sudoku, fila, columna + 1);
  }
  // Probamos numeros 1-9
  bool resuelto = false;
  int v = 1;

  while (v <= dim && !resuelto) {

    if (sudoku.pon_valor(fila, columna, v)) {

      resuelto = resolver_sudoku(sudoku, fila, columna + 1);

      if (!resuelto) {
        sudoku.quita_valor(fila, columna);
      }
    }

    v++; // Pasamos al siguiente valor posible
  }

  // A la salida del bucle devulve resuelto
  return resuelto;
}

void cargar_listaSudokus(ListaSudokus &sudokus_nuevos,
                         ListaSudokus &partidas_guardadas,
                         tReglasSudoku &reglas) {
  // Cargamos los sudokus nuevos
  int aux = 0;
  string nombre = " ";
  ifstream file("sudokus/lista_sudokus.txt");
  if (file.is_open()) {
    file >> aux;

    for (int i = 0; i < aux; i++) {
      file >> nombre;
      ifstream sudoku("sudokus/" + nombre);
      if (reglas.carga_sudoku(sudoku)) {
        // Si lo cargo bien lo insertamos en listaSudokus
        sudokus_nuevos.insertar(reglas);
      }
      sudoku.close();
    }
    file.close();
  } else {
    cout << "NO SE PUDO ABRIR EL FICHERO sudokus/lista_sudous.txt" << endl;
  }

  // Ahora cargamos las partidas guardadas
  int num_partidas = 0;
  ifstream file_partidas("saves/lista_partidas.txt");

  if (file_partidas.is_open()) {
    file_partidas >> num_partidas;

    for (int i = 0; i < num_partidas; i++) {
      // 1. Carga la descripción original del sudoku (como en la versión 1)
      if (reglas.carga_sudoku(file_partidas)) {

        int f, c, v;

        // Lee la primera fila, o el -1 si no hay posiciones ocupadas
        file_partidas >> f;

        // Mientras no encontremos el centinela -1, seguimos leyendo
        while (f != -1) {
          file_partidas >> c >> v;
          reglas.pon_valor(f, c, v);
          // Leemos la siguiente fila (o el -1 para salir del bucle)
          file_partidas >> f;
        }

        // Insertamos la partida
        partidas_guardadas.insertar(reglas);
      }
    }
    file_partidas.close();
  } else {
    cout << "NO SE PUDO ABRIR EL FICHERO saves/lista_partidas.txt" << endl;
  }
}

// Esta función gestiona el ARCHIVO
void guardar_partidas(const ListaSudokus &partidas_guardadas) {
    ofstream archivo_salida("saves/lista_partidas.txt");

    if (archivo_salida.is_open()) {
        int total = partidas_guardadas.dame_num_elems();
        
        // Escribimos el número total de partidas al principio
        archivo_salida << total << endl;

        for (int i = 0; i < total; i++) {
            // Llamamos a la función especialista en escribir UN sudoku
            serializar_sudoku(archivo_salida, partidas_guardadas.dame_sudoku(i));
        }

        archivo_salida.close();
        cout << "[Sistema] Todas las partidas han sido exportadas." << endl;
    } else {
        cout << "[Error] No se pudo abrir saves/lista_partidas.txt" << endl;
    }
}

// Esta función gestiona el FORMATO del Sudoku
void serializar_sudoku(ofstream &archivo, const tReglasSudoku &reglas) {
    // Obtenemos el tablero base (el original sin jugadas)
    tReglasSudoku original = reglas;
    original.reset();

    int dim = reglas.dame_dimension();
    
    // Guardamos la dimensión y el tablero original
    archivo << dim << endl;
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            archivo << original.dame_celda(i, j) << " ";
        }
        archivo << endl;
    }

    // Comparamos con el tablero actual para guardar las jugadas del usuario
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            int val_act = reglas.dame_celda(i, j);
            int val_orig = original.dame_celda(i, j);

            // Si el original era 0 y ahora hay algo, es una jugada
            if (val_orig == 0 && val_act != 0) {
                archivo << i << " " << j << " " << val_act << endl;
            }
        }
    }

    // 4. Centinela para indicar que no hay más jugadas en este Sudoku
    archivo << "-1" << endl;
}
void mostrar_sudoku(const tReglasSudoku &reglas, const tError error) {
  const int LINEA_HORIZONTAL = 196;
  const int LINEA_VERTICAL = 179;
  const int CRUCE = 197;
  const int ANCHO_CELDA = 3;

  int dim = reglas.dame_dimension();
  int dim_submatriz = (int)sqrt((double)dim);
  if (dim_submatriz <= 0) dim_submatriz = 1;

  cout << endl << CYAN << "================ SUDOKU ================" << RESET << '\n';

  cout << "    ";
  for (int c = 0; c < dim; c++) {
    cout << setw(3) << c;
    if ((c + 1) % dim_submatriz == 0 && c != dim - 1)
      cout << "  ";
  }
  cout << '\n';

  for (int i = 0; i < dim; i++) {
    // Número de fila
    cout << setw(3) << i << " ";
    
    for (int j = 0; j < dim; j++) {
      int v = reglas.dame_celda(i, j);
      
      if (v == 0) {
        cout << setw(3) << '.';
      } else {
        // Los números se imprimen en color para resaltar
        cout << CYAN << setw(3) << v << RESET;
      }

      // Separador vertical de submatrices
      if ((j + 1) % dim_submatriz == 0 && j != dim - 1)
        cout << " " << char(LINEA_VERTICAL);
    }
    cout << '\n';

    if ((i + 1) % dim_submatriz == 0 && i != dim - 1) {
      cout << "    ";
      for (int j = 0; j < dim; j++) {
        for (int k = 0; k < ANCHO_CELDA; k++)
          cout << char(LINEA_HORIZONTAL);

        if ((j + 1) % dim_submatriz == 0 && j != dim - 1) {
          cout << char(CRUCE);
          cout << char(LINEA_HORIZONTAL);
        }
      }
      cout << '\n';
    }
  }

  if (!reglas.terminado()) {
    if (error != ninguno) {
      string error_texto;
      switch (error) {
        case opciones:  error_texto = "OPCION INCORRECTA"; break;
        case valor:     error_texto = "VALOR INCORRECTO"; break;
        case bloqueada:  error_texto = "CELDA BLOQUEADA!"; break;
        case original:  error_texto = "CELDA ORIGINAL"; break;
        case vacia:     error_texto = "CELDA VACIA"; break;
        case ocupada:   error_texto = "CELDA OCUPADA"; break;
        default:        error_texto = ""; break;
      }
      cout << BG_RED << error_texto << RESET << endl;

      if (error == bloqueada) {
        int f, c;
        cout << "Celdas bloqueadas: ";
        for (int i = 0; i < reglas.dame_num_celdas_bloqueadas(); i++) {
          reglas.dame_celda_bloqueada(i, f, c);
          cout << "(" << f << ", " << c << ")" << ", ";
        }
        cout << endl;
        pausar();
      }
    }
  } else {
    cout << HK_PINK_PASTEL << "=========== SUDOKU TERMINADO ===========" << RESET << endl;
  }
}

void poner_valor(tReglasSudoku &reglas, tError &error) {
  if (error != bloqueada) {
    int fila, columna, v;
    cout << "Introduce fila y columna: ";
    cin >> fila >> columna;

    int dim = reglas.dame_dimension();

    // Si la posición es inválida, marcamos error.
    if (fila < 0 || fila >= dim || columna < 0 || columna >= dim) {
      error = valor;
    }
    // Si la posición ES válida, entonces pedimos el valor e intentamos ponerlo.
    else {
      cout << "Introduce el valor: ";
      cin >> v;

      if (!reglas.pon_valor(fila, columna, v)) {
        error = valor;
      } else {
        if (reglas.bloqueo()) {
          error = bloqueada;
        }
      }
    }
  }
}

void quitar_valor(tReglasSudoku &reglas, tError &error) {
  int fila, columna;
  cout << "Introduce fila y columna: " << endl;
  cin >> fila >> columna;

  int dim = reglas.dame_dimension();

  if (fila < 0 || fila >= dim || columna < 0 || columna >= dim) {
    error = valor;
  }
  // Si las coordenadas están bien
  else if (reglas.dame_celda(fila, columna) == 0) {
    error = vacia;
  }
  // 3º filtro: Todo está perfecto, procedemos a quitar el valor.
  else {
    if (!reglas.quita_valor(fila, columna)) {
      error = original;
    }
  }
}

int posibles_valores(tReglasSudoku &reglas, tError &error) {
  int cantidad = -1;
  int fila, columna;
  int dim = reglas.dame_dimension();

  cout << "Introduce fila y columna: " << endl;
  cin >> fila >> columna;

  // Comprueba coordenadas
  if (fila < 0 || fila >= dim || columna < 0 || columna >= dim) {
    error = valor;
  } else if (reglas.dame_celda(fila, columna) !=
             0) { // Comprubea si la celda esta vacia
    error = ocupada;
  } else {
    int lista[MAX];
    cantidad = reglas.posibles_valores(fila, columna, lista);

    cout << "POSIBLES VALORES (" << cantidad << "): ";
    if (cantidad == 0) {
      cout << "ninguno";
    } else {
      for (int i = 0; i < cantidad; i++) {
        cout << lista[i];
        if (i < cantidad - 1)
          cout << ", ";
      }
    }
    cout << endl;
  }

  return cantidad;
}

void pausar() {
  cout << "\nPresiona Enter para continuar...";
  cin.ignore();
  cin.get();
}