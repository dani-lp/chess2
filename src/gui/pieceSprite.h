#ifndef CHESS2_PIECESPRITE_H
#define CHESS2_PIECESPRITE_H

#include "sprite.h"

class PieceSprite : public Sprite {
private:
    Vec2 focus;
    bool overlapsMouse();
    bool touched;
public:
    enum State {
        IDLE, CHOOSING, MOVING, ATTACKING
    };
    State state;
    bool active;
    int id;
    PieceSprite(int x, int y, int id, const String& texturePath);
    void update() override;
    void draw(Batch *batch) override;
    void setFocus(Vec2 focus);
    void setCoords(int x, int y);
};


#endif //CHESS2_PIECESPRITE_H
