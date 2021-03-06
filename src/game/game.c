#include <stdio.h>
#include "game.h"

#define LOGGING 1

/**
 * Comprueba si las casillas introducidas son válidas.
 * @return 1 si son inválidas
 */
int invalidPositions(int x1, int y1, int x2, int y2);

/**
 * Devuelve la vida de la pieza.
 * @param id Id de la pieza
 * @return Vida de la pieza
 */
int getPieceHp(Game game, int id);

/**
 * Comprueba si hay una pieza entre medias para un movimiento largo de lancero.
 */
int spearBlock(int data[B_ROWS][B_COLUMNS], int team, int originX, int originY, int destinyY);

void startGame(Game *game, char *form1, char *form2)
{
    // Inicializar datos base
    game->time = 0;
    game->turn = 0;
    game->nexus1hp = NEXUS_HP;
    game->nexus2hp = NEXUS_HP;
    //char *form1 = loadForm(f1);
    //char *form2 = loadForm(f2);

    // Inicializar datos de piezas
    for (int i = 0; i < 24; i++) game->pieces[i] = (Piece) {i + 1, getBaseHp(i+1), getBaseDmg(i + 1)};
    game->pieces[24] = (Piece) {25, NEXUS_HP};
    game->pieces[25] = (Piece) {26, NEXUS_HP};
    int s = 1;      // Lanceros
    int w = 5;      // Magos
    int a = 8;      // Asesinos
    int g = 11;     // Golems

    for (int i = 0; i < B_ROWS; i++) { // Lado izquierdo
        for (int j = 0; j < 3; j++) {
            char unit = form1[3 * i + j];
            if      (unit == 's') game->data[i][j] = s++;
            else if (unit == 'w') game->data[i][j] = w++;
            else if (unit == 'a') game->data[i][j] = a++;
            else if (unit == 'g') game->data[i][j] = g++;
            else if (unit == 'e') game->data[i][j] = 0;
            else if (unit == 'N') game->data[i][j] = 25;
        }
    }

    s = 13;      // Lanceros
    w = 17;      // Magos
    a = 20;      // Asesinos
    g = 23;     // Golems
    for (int i = 0; i < B_ROWS; i++) { // Lado derecho
        for (int j = 2; j >= 0; j--) {
            char unit = form2[3 * i + 2 - j];
            if      (unit == 's') game->data[i][j+8] = s++;
            else if (unit == 'w') game->data[i][j+8] = w++;
            else if (unit == 'a') game->data[i][j+8] = a++;
            else if (unit == 'g') game->data[i][j+8] = g++;
            else if (unit == 'e') game->data[i][j+8] = 0;
            else if (unit == 'N') game->data[i][j+8] = 26;
        }
    }

    for (int i = 0; i < B_ROWS; i++) { // Zona central
        for (int j = 3; j < 8; j++) {
            game->data[i][j] = 0;
        }
    }
}

int updatePiece(Game *game, int originX, int originY, int destinyX, int destinyY)
{
    // Posiciones fuera del tablero
    if (invalidPositions(originX, originY, destinyX, destinyY)) {
        if (LOGGING) printf("Posicion no valida.\n");
        return 0;
    }

    // Localizar pieza, si la hay
    int id = locateId(*game, originX, originY); // ID de la pieza en la posición de origen
    if (id == 0) { // Si no hay pieza
        if (LOGGING) printf("Casilla vacia.\n");
        return 0;
    } else if (id >= 25) { // Si la pieza es un nexo (IDs 25 y 26)
        if (LOGGING) printf("No se puede mover el nexo.\n");
        return 0;
    }
    Piece *piece = NULL; // Puntero a pieza objetivo
    for (int i = 0; i < 24; i++) { // Encontrar y asignar pieza objetivo
        if (game->pieces[i].id == id) {
            if (game->pieces[i].hp > 0) piece = &game->pieces[i]; // Asignar si la pieza está "viva"
            break;
        }
    }

    // Evaluar equipo y actualizar el estado de las piezas
    int team = piece->id <= 12 ? 0 : 1; // 0 equipo izquierdo, 1 equipo derecho
    int destinyCode = game->data[destinyX][destinyY]; // Comprobar estado de la casilla objetivo

    if (destinyCode == 0 && team == game->turn % 2) { // Casilla vacía y pieza aliada
        if (spearBlock(game->data, team, originX, originY, destinyY) && pieceType(piece) == 0) {
            if (LOGGING) printf("El lancero no puede atravesar piezas enemigas.\n");
            return 0;
        }
        if (movePiece(piece, game, originX, originY, destinyX, destinyY)) {
            if (LOGGING) printf("Pieza movida correctamente.\n");
            return 1;
        }
        else return 0;
    }
    else if (team != game->turn % 2) { // Mover a pieza enemiga
        if (LOGGING) printf("Esa no es una de tus piezas.\n");
        return 0;
    }
    else if (((destinyCode <= 12 || destinyCode == 25) && team == 1) || ((destinyCode >= 12 && destinyCode != 25) && team == 0)) { // Atacar a pieza enemiga
        // Localizar pieza enemiga
        Piece *enemypiece = NULL; // Puntero a pieza enemiga
        int enemyId = locateId(*game, destinyX, destinyY);
        for (int i = 0; i < 26; i++) { // Encontrar y asignar pieza enemiga
            if (game->pieces[i].id == enemyId) {
                if (game->pieces[i].hp > 0) {
                    enemypiece = &game->pieces[i]; // Asignar si la pieza está "viva"
                    break;
                }
            }
        }

        if (!canAttack(piece, enemypiece, originX, originY, destinyX, destinyY)) {
            if (LOGGING) printf("No puedes atacar esa casilla.\n");
            return 0;
        }

        int attackResult = attackPiece(enemypiece, getBaseDmg(piece->id)); // Realizar ataque
        if (attackResult == 0) {  // La pieza enemiga ha sobrevivido
            if (pieceType(piece) == 1 && pieceType(enemypiece) == 1) {  // Si eres un mago, sólo te puede contraatacar otro mago
                attackPiece(piece, getBaseDmg(enemypiece->id));
            }
            else if (pieceType(piece) != 1 && pieceType(enemypiece) != 1){  // Si la pieza atacante NO es un mago:
                if (attackPiece(piece, getBaseDmg(enemypiece->id))){    // Contraataque
                    movePiece(enemypiece, game, originX, originY, destinyX, destinyY); // Tu pieza ha muerto y la enemiga toma su lugar
                }
            }
            if (LOGGING) printf("Ataque realizado.\n");
            return 2;
        } else if (attackResult == 1) { // Has eliminado la pieza enemiga
            if (pieceType(piece) != 1){ // Si no eres un mago tomas tu posicion
                movePiece(piece, game, originX, originY, destinyX, destinyY);
            } else {    // Si eres un mago la pieza es simplemente eliminada
                game->data[destinyX][destinyY] = 0;
            }
            if (LOGGING) printf("Pieza eliminada.\n");
            return 3;
        } else if (attackResult == 2) { // Has atacado a un nexo
            if (enemypiece->id == 25) game->nexus1hp -= piece->dmg;  // Se le resta vida a la variable global de vida nexo
            else if (enemypiece->id == 26) game->nexus2hp -= piece->dmg;
            if (LOGGING) printf("Nexo atacado.\n");
            return 2;
        } else {
            return 0;
        }
    }
    else { // Pieza propia
        if (LOGGING) printf("La casilla esta ocupada por una pieza propia.\n");
        return 0;
    }
}

int movePiece(Piece *piece, Game *game, int originX, int originY, int destinyX, int destinyY)
{
    if (canMove(piece, originX, originY, destinyX, destinyY)) {
        game->data[originX][originY] = 0;   // Tu posicion actual del tablero se libra
        game->data[destinyX][destinyY] = piece->id; // Ocupas la casilla libre
        return 1;
    } else {
        if (LOGGING) printf("Movimiento ilegal, ID: %d\n", piece->id);
        return 0;
    }
}

int attackPiece(Piece *piece, int damage)
{
    piece->hp -= damage;
    if (piece->hp < 1 && piece->id <= 24) return 1; // Pieza eliminada
    else if (piece->id > 24) return 2; // Nexo golpeado
    else if (piece->hp < 1 && piece->id > 24) return 3; // Nexo eliminado
    else return 0; // Pieza golpeada
}

void printBoard(Game game)
{
    for (int i = 0; i < 7; i++) {   // Dibujo de tablero
        printf("%d > |", i + 1);
        for (int j = 0; j < 11; j++) {
            int pieceCode = game.data[i][j];
            if (pieceCode == 0) printf("%c |", pieceLetter(pieceCode));
            else if (pieceCode == 25) printf("%cI|", pieceLetter(pieceCode));
            else printf(pieceCode <= 12 ? "%cI|" : "%cD|", pieceLetter(pieceCode));
        }
        printf("\n"); // Formateo
    }

    printf("      "); // Formateo
    for (int i = 0; i < 11; i++) {
        printf("^  "); // Formateo
    }

    printf("\n      "); // Formateo
    for (int i = 0; i < 11; i++) {
        printf("%c  ", 'a' + i);
    }
    printf("\n");
}

void printLifeBoard(Game game)
{
    for (int i = 0; i < 7; i++) {
        printf("%d > |", i + 1);    // Dibujo de tablero
        for (int j = 0; j < 11; j++) {
            int hp = getPieceHp(game, game.data[i][j]);
            if (hp <= 0) printf("  |");
            else printf(hp < 10 ? "0%d|" : "%d|", hp);
        }
        printf("\n"); // Formateo
    }

    printf("      "); // Formateo
    for (int i = 0; i < 11; i++) {
        printf("^  "); // Formateo
    }

    printf("\n      "); // Formateo
    for (int i = 0; i < 11; i++) {
        printf("%c  ", 'a' + i);
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

int getPieceHp(Game game, int id)
{
    for (int i = 0; i < 26; i++) { // Encontrar y asignar pieza enemiga
        if (game.pieces[i].id == id) {
            return game.pieces[i].hp;
        }
    }
    return 0;
}

int *getPiecePos(Game game, int id) // Encontrar posicion en tablero de la pieza
{
    int *ids;
    for (int i = 0; i < B_ROWS; ++i) {
        for (int j = 0; j < B_COLUMNS; ++j) {
            if (game.data[i][j] == id){
                ids[0] = i;
                ids[1] = j;
                return ids;
            }
        }
    }
}

int spearBlock(int data[B_ROWS][B_COLUMNS], int team, int originX, int originY, int destinyY)
{
    return (team == 0 && (destinyY - originY == 2 && data[originX][originY + 1] != 0)) ||
           (team == 1 && (originY - destinyY == 2 && data[originX][originY - 1] != 0));
}