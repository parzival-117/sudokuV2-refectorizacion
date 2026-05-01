#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "h/ListaSudokus.h"
#include "h/colors.h"

using namespace std;

enum tError { ninguno, opciones, valor, bloqueada, original, vacia, ocupada };

int mostrar_sudoku(const tReglasSudoku &reglas, const tError error);
void poner_valor(tReglasSudoku &reglas, tError &error);
void quitar_valor(tReglasSudoku &reglas, tError &error);
int posibles_valores(tReglasSudoku &reglas, tError &error);
void cargar_listaSudokus(ListaSudokus &sudokus_nuevos,
                         ListaSudokus &partidas_guardadas,
                         tReglasSudoku &reglas);

void pausar();
char mayus(char op);
void guardar_partidas(const ListaSudokus &guardar_partidas);

bool resolver_sudoku(tReglasSudoku &sudoku, int fila, int columna);

int main() {

  tReglasSudoku reglas;

  ListaSudokus sudokus_nuevos;
  ListaSudokus partidas_guardadas;

  cargar_listaSudokus(sudokus_nuevos, partidas_guardadas, reglas);

  tError error = ninguno;

  char op = ' ';
  int aux = 0;
  bool jugar = false;

  cout << "Partida nueva (N), continuar partida (C) o abandonar la aplicacion "
          "(A)?"
       << endl;
  cin >> op;
  op = mayus(op);
  if (op == 'A' || op == 'C' || op == 'N') {

    while (op != 'A') {
      tReglasSudoku partida;
      aux = 0;
      jugar = false;

      bool partida_nueva = false;
      int indice = -1;
      switch (op) {
      case 'N':
        cout << sudokus_nuevos;
        cout << "Elige un sudoku: ";
        cin >> aux;

        if (cin.fail()) {
          cin.clear();
          cin.ignore(10000, '\n');
          aux = -1;
        }

        if (aux <= sudokus_nuevos.dame_num_elems() && aux >= 1) {
          jugar = true;
          partida_nueva = true;
          partida = sudokus_nuevos.dame_sudoku(aux - 1);
        } else {
          // Asignamos el error de la enumeración
          error = valor;
          // Lo imprimimos directamente porque no vamos a entrar a
          // mostrar_sudoku()
          cout << BG_RED << "VALOR INCORRECTO: Ese sudoku no existe." << RESET
               << endl;
        }

        break;
      case 'C':
        if (partidas_guardadas.dame_num_elems() > 0) {
          cout << partidas_guardadas;
          cout << "Elige un sudoku: ";
          cin >> aux;

          if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            aux = -1;
          }
          if (aux <= partidas_guardadas.dame_num_elems() && aux >= 1) {
            jugar = true;
            partida_nueva = false;
            indice = aux - 1;
            partida = partidas_guardadas.dame_sudoku(indice);
          } else {
            // Asignamos el error de la enumeración
            error = valor;
            cout << BG_RED << "VALOR INCORRECTO: Esa partida no existe."
                 << RESET << endl;
          }
        } else {
          cout << "No hay partidas guardadas.\n";
        }
        break;
      case 'A': {

        break;
      }
      default:
        break;
      }
      if (jugar) {

        int opcion = mostrar_sudoku(partida, error);
        while (opcion != 7) {
          error = ninguno;
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
          case 5: // celdas con unico valor
            partida.autocompletar();
            break;
          case 6: // Resolver el sudoku
            resolver_sudoku(partida, 0, 0);
            break;
          default:
            error = opciones;
            break;
          }
          opcion = mostrar_sudoku(partida, error);
        }
        // GUARDADO
        if (partida.terminado()) {
          // Si venía de las partidas guardadas, hay que borrarlo porque ya se
          // lo pasó
          if (!partida_nueva) {
            partidas_guardadas.eliminar(indice);
          }
          // Si era nuevo, no hacemos nada (simplemente no lo guardamos)
        }
        // El usuario lo deja a medias (Sale con el 7)
        else {
          if (partida_nueva) {
            // Si era nuevo, simplemente lo añadimos a la lista de guardadas
            partidas_guardadas.insertar(partida);
            cout << "\n[Info] Partida nueva guardada en progreso.\n";
          } else {
            // Si ya era una partida guardada, la eliminamos y la volvemos a
            // insertar (Se hace esto para que el insertar() la reordene
            // automáticamente según su nueva dificultad)
            partidas_guardadas.eliminar(indice);
            partidas_guardadas.insertar(partida);
            cout << "\n[Info] Partida guardada actualizada.\n";
          }
        }
      }
      cout << "Partida nueva (N), continuar partida (C) o abandonar la "
              "aplicacion "
              "(A)?"
           << endl;
      cin >> op;
      op = mayus(op);
    }
  }
  // Si se elige salir, guardamos las partidas
  ofstream archivo_salida("saves/lista_partidas.txt");

  if (archivo_salida.is_open()) {
    // 1. Escribimos el número total de partidas
    int total_partidas = partidas_guardadas.dame_num_elems();
    archivo_salida << total_partidas << endl;

    for (int p = 0; p < total_partidas; p++) {
      // Obtenemos el sudoku en su estado actual (con jugadas)
      tReglasSudoku actual = partidas_guardadas.dame_sudoku(p);

      // Hacemos una copia y la reseteamos para tener el tablero original
      // base
      tReglasSudoku original = actual;
      original.reset();

      int dim = actual.dame_dimension();

      // Guardamos la dimensión y el tablero original
      archivo_salida << dim << endl;
      for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
          archivo_salida << original.dame_celda(i, j) << " ";
        }
        archivo_salida << endl;
      }

      // Guardamos las posiciones ocupadas por el jugador (comparando
      // ambos tableros)
      for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
          int val_orig = original.dame_celda(i, j);
          int val_act = actual.dame_celda(i, j);

          // Si en el original era 0 (vacía) y ahora tiene un número, lo
          // puso el jugador
          if (val_orig == 0 && val_act != 0) {
            archivo_salida << i << " " << j << " " << val_act << endl;
          }
        }
      }

      // 4. Escribimos el centinela de fin de posiciones
      archivo_salida << "-1" << endl;
    }

    archivo_salida.close();
    cout << "Partidas guardadas correctamente. ¡Hasta pronto!\n";
  } else {
    cout << "Error: No se pudo abrir lista_partidas.txt para guardar.\n";
  }

  return 0;
}

char mayus(char op) {
  switch (op) {

  case 'n':
    op = 'N';
    break;
  case 'c':
    op = 'C';
    break;
  case 'a':
    op = 'A';
    break;
  default:
    break;
  }
  return op;
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

int mostrar_sudoku(const tReglasSudoku &reglas, const tError error) {
  const int LINEA_HORIZONTAL = 196;
  const int LINEA_VERTICAL = 179;
  const int CRUCE = 197;
  const int ANCHO_CELDA = 3;

  int opcion = -1;
  int dim = reglas.dame_dimension();
  int dim_submatriz = (int)sqrt((double)dim);
  if (dim_submatriz <= 0)
    dim_submatriz = 1;
  string error_texto;

  cout << endl
       << CYAN << "================ SUDOKU ================" << RESET << '\n';

  // Cabecera de columnas
  cout << "    ";
  for (int c = 0; c < dim; c++) {
    cout << setw(3) << c;
    if ((c + 1) % dim_submatriz == 0 && c != dim - 1)
      cout << "  ";
  }

  cout << '\n';

  for (int i = 0; i < dim; i++) {
    cout << setw(3) << i << " ";
    for (int j = 0; j < dim; j++) {
      int v = reglas.dame_celda(i, j);
      if (v == 0)
        cout << setw(3) << '.';
      else {
        cout << CYAN << setw(3) << v << RESET;
      }

      // Separador de subcuadros de tamano variable
      if ((j + 1) % dim_submatriz == 0 && j != dim - 1)
        cout << " " << char(LINEA_VERTICAL);
    }
    cout << '\n';

    if ((i + 1) % dim_submatriz == 0 && i != dim - 1) {
      cout << "    ";
      cout << char(LINEA_HORIZONTAL);
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
    cout << '\n';
    cout << YELLOW << "------------- MENU -------------" << RESET << '\n';
    cout << "1. Poner valor\n";
    cout << "2. Quitar valor\n";
    cout << "3. Reset\n";
    cout << "4. Posibles valores de una celda vacia\n";
    cout << "5. Autocompletar celdas con valor unico\n";
    cout << "6. Resolver el sudoku\n";
    cout << "7. Salir\n";
    cout << YELLOW << "--------------------------------" << RESET << '\n';
    switch (error) {
    case opciones:
      error_texto = "OPCION INCORRECTA";
      break;
    case valor:
      error_texto = "VALOR INCORRECTO";
      break;
    case bloqueada:
      error_texto = "CELDA BLOQUEADA!";
      break;
    case original:
      error_texto = "CELDA ORIGINAL";
      break;
    case vacia:
      error_texto = "CELDA VACIA";
      break;
    case ocupada:
      error_texto = "CELDA OCUPADA";
      break;
    default:
      error_texto = "";
      break;
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
    cout << "Opcion: ";

    cin >> opcion;

    if (cin.fail()) {
      cin.clear();
      cin.ignore(10000, '\n');
      opcion = -1;
    }
  } else {
    cout << HK_PINK_PASTEL
         << "=========== SUDOKU TERMINADO ===========" << RESET << endl;
    opcion = 7;
  }

  return opcion;
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