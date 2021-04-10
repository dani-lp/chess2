#include "pieceSprite.h"
#include <cmath>

static int selectedPiece = 0;
int getPieceBaseHp(int id);
int getPieceBaseDmg(int id);

PieceSprite::PieceSprite(int x, int y, int id, const String& texturePath) {
    this->state = IDLE;
    this->setX(x);
    this->setY(y);
    this->id = id;
    this->hp = getPieceBaseHp(this->getPieceCode());
    this->dmg = getPieceBaseDmg(this->getPieceCode());
    this->dmg = getDamage(this->id);
    this->active = true;
    this->texture = Texture::create(texturePath);
    this->touched = false;
    this->focus = Vec2(0,0);
}

void PieceSprite::update(Game *game) {
    if (this->touched && this->overlapsMouse() && Input::released(MouseButton::Left)) this->touched = false;
    if (this->hp <= 0) {
        this->active = false;
        return;
    }
    this->hp = game->pieces[this->id - 1].hp;
    switch (this->state) {
        case IDLE:
            if (this->overlapsMouse() && Input::pressed(MouseButton::Left) && !this->touched) {
                this->touched = true;
                this->state = CHOOSING;
                selectedPiece = this->id;
            }
            break;
        case CHOOSING:
            if (this->overlapsMouse() && Input::pressed(MouseButton::Left) && !this->touched) {
                this->touched = true;
                this->state = IDLE;
                selectedPiece = 0;
            }
            if (selectedPiece != this->id) this->state = IDLE;
            for (auto & pos:this->getMovePositions(game->data)) {
                if (mouseOverlapsPoint((int)pos.x, (int) pos.y) && Input::pressed(MouseButton::Left) && !this->touched){
                    int originY = this->getX()/64 - 6;
                    int originX = this->getY()/64 - 1;
                    int destinyY = (int)pos.x/64 - 6;
                    int destinyX = (int)pos.y/64 - 1;
                    if (updatePiece(game,originX,originY,destinyX,destinyY) == 1){
                        game->turn++;
                        this->focus = Vec2((int) pos.x, (int) pos.y);
                        this->state = MOVING;
                    } else {
                        this->state = IDLE;
                        selectedPiece = 0;
                    }
                }
            }
            for (auto & pos:this->getAttackPositions(game->data)) {
                if (mouseOverlapsPoint((int)pos.x, (int) pos.y) && Input::pressed(MouseButton::Left) && !this->touched){
                    int originY = this->getX()/64 - 6;
                    int originX = this->getY()/64 - 1;
                    int destinyY = (int)pos.x/64 - 6;
                    int destinyX = (int)pos.y/64 - 1;
                    if (game->data[destinyX][destinyY] == 0) break;
                    int result = updatePiece(game,originX,originY,destinyX,destinyY);
                    if (result == 2) { // Ataque sin destruir
                        game->turn++;
                        this->state = IDLE;
                        selectedPiece = 0;
                        std::cout << "2" << std::endl;
                    } else if (result == 3) { // Ataque destruyendo
                        game->turn++;
                        this->focus = Vec2((int) pos.x, (int) pos.y);
                        this->state = MOVING;
                        std::cout << "3" << std::endl;
                        std::cout << this->id << std::endl;
                        std::cout << this->focus.x << ":" << this->focus.y << std::endl;
                    } else if (result == 0 || result == 1) { // Ataque no realizado/pieza movida
                        this->state = IDLE;
                        selectedPiece = 0;
                        std::cout << "0/1" << std::endl;
                    }
                }
            }
            break;
        case MOVING:
            // Animación de movimiento
            if (!(this->getX() == (int)this->focus.x && this->getY() == (int)this->focus.y)) {
                int addX = (int) std::ceil((float) ((int) this->focus.x - this->getX()) * Time::delta * 6);
                int addY = (int) std::ceil((float) ((int) this->focus.y - this->getY()) * Time::delta * 6);
                this->setX(this->getX() + addX);
                this->setY(this->getY() + addY);
                if (addX == 0 && this->getX() != (int)this->focus.x) this->setX((int) this->focus.x);
                if (addY == 0 && this->getY() != (int)this->focus.y) this->setY((int) this->focus.y);
            } else {
                std::cout<<*this<<std::endl;
                this->state = IDLE;
            }
            break;
        case ATTACKING:
            // Animación de ataque
            break;
    }
}

void PieceSprite::draw(Batch *batch) {
    if (this->active) batch->tex(this->texture, Vec2(this->getX(), this->getY()));
}

bool PieceSprite::overlapsMouse() {
    Vec2 mouse = Input::mouse_draw();
    int x = (int) mouse.x;
    int y = (int) mouse.y;
    return this->getX() <= x && this->getX() + 64 >= x && this->getY() <= y && this->getY() + 64 >= y;
}

bool PieceSprite::overlapsPoint(int x, int y){
    return this->getX() <= x && this->getX() + 64 >= x && this->getY() <= y && this->getY() + 64 >= y;
}

bool PieceSprite::mouseOverlapsPoint(int x, int y) {
    Vec2 mouse = Input::mouse_draw();
    int mx = (int) mouse.x;
    int my = (int) mouse.y;
    return x <= mx && x + 64 >= mx && y <= my && y + 64 >= my;
}

int PieceSprite::getPieceCode()
{
    int piece = this->id;
    if ((piece >= 1 && piece <= 4) || (piece >= 13 && piece <= 16)) {
        return 0;
    } else if ((piece >= 5 && piece <= 7) || (piece >= 17 && piece <= 19)) {
        return 1;
    } else if ((piece >= 8 && piece <= 10) || (piece >= 20 && piece <= 22)) {
        return 2;
    } else {
        return 3;
    }
}

std::vector<Vec2> PieceSprite::getMovePositions(int data[7][11])
{
    int x = this->getX();
    int y = this->getY();
    int relX = x/64 - 6;
    int relY = y/64 - 1;
    std::vector<Vec2> positions;
    switch (this->getPieceCode()) {
        case 0: // Lancero
            if (id <= 12) {
                if (relX + 1 <= 10)                              positions.emplace_back(Vec2(x + 64, y));
                if (relX + 2 <= 10 && data[relY][relX + 1] == 0) positions.emplace_back(Vec2(x + 128, y));
            } else {
                if (relX - 1 >= 0)                               positions.emplace_back(Vec2(x - 64, y));
                if (relX - 2 >= 0 && data[relY][relX - 1] == 0)  positions.emplace_back(Vec2(x - 128, y));
            }
            break;
        case 2: // Asesino
            for (int i = -3; i <= 3; i++) {
                if (i == 0) continue;
                if (relX + i <= 10 && relX + i >= 0) positions.emplace_back(Vec2(x + i*64, y));
                if (relY + i <= 6  && relY + i >= 0) positions.emplace_back(Vec2(x, y + i*64));
            }
            break;
        case 1: // Mago
        case 3: // Golem
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) continue;
                    else if (relX + i <= 10 && relX + i >= 0 && relY + j <= 6 && relY + j >= 0)
                        positions.emplace_back(x + i*64, y + j*64);
                }
            }
            break;
    }
    for (auto it = positions.begin(); it != positions.end();) {
        if (data[(int) (it->y/64 - 1)][(int) (it->x/64 - 6)] == 0) it++;
        else positions.erase(it);
    }
    return positions;
}

std::vector<Vec2> PieceSprite::getAttackPositions(int data[7][11])
{
    int x = this->getX();
    int y = this->getY();
    int relX = x/64 - 6;
    int relY = y/64 - 1;
    std::vector<Vec2> positions;
    switch (this->getPieceCode()) {
        case 0: // Lancero
            if (this->id <= 12) {
                if (relX + 1 <= 10) {
                    if (relY     <= 6 && relY     >= 0) positions.emplace_back(Vec2(x + 64, y));
                    if (relY + 1 <= 6 && relY + 1 >= 0) positions.emplace_back(Vec2(x + 64, y + 64));
                    if (relY - 1 <= 6 && relY - 1 >= 0) positions.emplace_back(Vec2(x + 64, y - 64));
                }
            } else {
                if (relX - 1 >= 0) {
                    if (relY     <= 6 && relY     >= 0) positions.emplace_back(Vec2(x - 64, y));
                    if (relY + 1 <= 6 && relY + 1 >= 0) positions.emplace_back(Vec2(x - 64, y + 64));
                    if (relY - 1 <= 6 && relY - 1 >= 0) positions.emplace_back(Vec2(x - 64, y - 64));
                }
            }
            break;
        case 2: // Asesino
            if (relX + 1 <= 10)  positions.emplace_back(Vec2(x + 64, y));
            if (relX - 1 >= 0)   positions.emplace_back(Vec2(x - 64, y));
            if (relY + 1 <= 6)   positions.emplace_back(Vec2(x, y + 64));
            if (relY - 1 >= 0)   positions.emplace_back(Vec2(x, y - 64));
            break;
        case 1: // Mago
            // Arriba y abajo
            for (int i = -2; i <= 2; i++) {
                if (relX + i <= 10 && relX + i >= 0) {
                    if (relY + 2 <= 6) positions.emplace_back(Vec2(x + i*64, y + 128));
                    if (relY - 2 >= 0) positions.emplace_back(Vec2(x + i*64, y - 128));
                }
            }
            // Lados
            for (int i = -1; i <= 1; i++) {
                if (relY + i <= 6 && relY + i >= 0) {
                    if (relX + 2 <= 10) positions.emplace_back(Vec2(x + 128, y + i*64));
                    if (relX - 2 >= 0)  positions.emplace_back(Vec2(x - 128, y + i*64));
                }
            }
            break;
        case 3: // Golem
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) continue;
                    else if (relX + i <= 10 && relX + i >= 0 && relY + j <= 6 && relY + j >= 0) positions.emplace_back(x + i*64, y + j*64);
                }
            }
            break;
    }

    for (auto it = positions.begin(); it != positions.end();) {
        int team = id <= 12 ? 0 : 1;
        int posCode = data[(int)(it->y/64 - 1)][(int)(it->x/64 - 6)];
        if (team == 0) {
            if (posCode >= 13 && posCode != 25) it++;
            else positions.erase(it);
        } else if (team == 1) {
            if ((posCode <= 12 && posCode >= 1) || posCode == 25) it++;
            else positions.erase(it);
        }
    }
    return positions;
}


int getPieceBaseHp(int id) // TODO valores iniciales de vida
{
    switch (id) {
        case 0:
            return 40;
        case 1:
            return 50;
        case 2:
            return 60;
        case 3:
            return 70;
        default:
            return 10;
    }
}

int getPieceBaseDmg(int id) // TODO valores iniciales de daño
{
    switch (id) {
        case 0:
            return 10;
        case 1:
            return 10;
        case 2:
            return 20;
        case 3:
            return 30;
        default:
            return 5;
    }
}

int PieceSprite::getDmg() {
    return this->dmg;
}
