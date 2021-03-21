#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "filem.h"

/**
 * Comprueba si las casillas introducidas son válidas.
 * @return 1 si son inválidas
 */
int invalidPositions(int x1, int y1, int x2, int y2);

void startGame(Game *game, int f1, int f2)
{
    game->time = 0;
    game->turn = 0;
    char *form1 = loadForm(f1);
    char *form2 = loadForm(f2);

    for (int i = 0; i < 24; i++) game->pieces[i] = (TestPiece) {i+1,10};

    int p = 0;
    int d = 0;
    for (int i = 0; i < B_ROWS; i++) {
        for (int j = 0; j < B_COLUMNS; j++) {
            if (j < 3) {
                if (form1[p] != 'w') game->data[i][j] = 3;
                else game->data[i][j] = 0;
                p++;
            } else if (j > 7) {
                if (form2[d] != 'w') game->data[i][j] = 3;
                else game->data[i][j] = 0;
                d++;
            } else {
                game->data[i][j] = 0;
            }
        }
    }
    free(form1);
    free(form2);
}

void updatePiece(Game *game, const char *origin, const char *destiny)
{
    int originX = origin[1] - '0' - 1;
    int originY = origin[0] - 'a';
    int destinyX = destiny[1] - '0' - 1;
    int destinyY = destiny[0] - 'a';

    if (invalidPositions(originX, originY, destinyX, destinyY)) {
        printf("Posicion no valida.\n");
        return;
    }

    int id = locateId(*game, originX, originY); // ID de la pieza en la posición de origen
    if (id == 0) { // Si no hay pieza
        printf("Casilla vacia.\n");
        return;
    }
    TestPiece *piece = NULL; // Puntero a pieza objetivo
    for (int i = 0; i < 22; i++) { // Encontrar y asignar pieza objetivo
        if (game->pieces[i].id == id) {
            if (game->pieces[i].hp > 0) piece = &game->pieces[i]; // Asignar si la pieza está "viva"
            break;
        }
    }

    int team = piece->id <= 12 ? 0 : 1; // 0 equipo "blanco", 1 equipo "negro"
    int destinyCode = game->data[destinyX][destinyY]; // Comprobar estado de la casilla objetivo
    if (destinyCode == 0) { // Casilla vacía
        movePiece(piece, game, originX, originY, destinyX, destinyY);
    }
    else if ((destinyCode <= 12 && team == 1) || (destinyCode >= 12 && team == 0)) { // Pieza enemiga
        //attackPiece();
    }
    else {
        printf("La casilla está ocupada por una pieza propia.\n");
        return;
    }
}

void movePiece(TestPiece *piece, Game *game, int originX, int originY, int destinyX, int destinyY)
{
    if (canMove(piece, originX, originY, destinyX, destinyY)) {
        game->data[originX][originY] = 0;
        game->data[destinyX][destinyY] = piece->id;
    } else {
        printf("Movimiento ilegal\n");
    }
}

void attackPiece(TestPiece *piece1, TestPiece *piece2, Game *game)
{

}

void printBoard(Game game)
{
    for (int i = 0; i < 7; i++) {
        printf("%d > |", i + 1);
        for (int j = 0; j < 11; j++)
        {
            int piece = game.data[i][j];
            char tempChar;
            if (piece == 0) tempChar = ' ';
            else tempChar = 'p';
            printf("%c|", tempChar);
        }
        printf("\n"); // Formateo
    }

    printf("     "); // Formateo
    for (int i = 0; i < 11; i++) {
        printf("^ "); // Formateo
    }

    printf("\n     "); // Formateo
    for (int i = 0; i < 11; i++) {
        printf("%c ", 'a' + i);
    }
    printf("\n");
}

int locateId(Game game, int x, int y)   //Buscar ID
{
    return game.data[x][y];
}

int invalidPositions(int x1, int y1, int x2, int y2)
{
    return x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 || x1 > 6 || y1 > 10 || x2 > 6 || y2 > 10;
}