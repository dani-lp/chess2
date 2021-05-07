#ifndef CHESS2_PIECESPRITE_H
#define CHESS2_PIECESPRITE_H

#include "sprite.h"
extern "C"{
    #include "../game/game.h"
}

class PieceSprite : public Sprite {
private:
    static int selectedPiece;
    Vec2 focus;
    Game *gameRef; // Referencia a game
    bool overlapsPoint(int x, int y);
    bool overlapsMouse();
    bool mouseOverlapsPoint(int x, int y);
    bool touched;
    int dmg;

    enum AttackDir {N, NW, W, SW, S, SE, E, NE, C}; // Direcciones de ataque
    AttackDir attackDir;
    void setAttackDir(Vec2 origin, Vec2 destiny); // Calcular direción de ataque
    double attack_timer; // Temporizador de animación
    Vec2 positionBuffer; // Posición de retorno
    void animateAttack();
public:
    int id, hp;
    bool active;
    int getDmg();
    int getPieceCode();
    PieceSprite(int x, int y, int id, const String& texturePath, Game *gameRef);

    enum State {
        IDLE, CHOOSING, MOVING, ATTACKING
    };
    State state;

    void update() override;
    void draw(Batch *batch) override;
    std::vector<Vec2> getMovePositions(int data[7][11]);
    std::vector<Vec2> getAttackPositions(int data[7][11]);
};


#endif //CHESS2_PIECESPRITE_H
