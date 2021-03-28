#include "GuiButton.h"

GuiButton::GuiButton() {
    this->width = 0;
    this->height = 0;
    this->touched = false;
}

void GuiButton::setWidth(int widthNum) {
    this->width = widthNum;
}

void GuiButton::setHeight(int heigthNum) {
    this->height = heigthNum;
}

void GuiButton::load(int xNum, int yNum, int widthNum, int heightNum, const String& idleTexPath, const String& pressedTexPath) {
    this->setX(xNum);
    this->setY(yNum);
    this->width = widthNum;
    this->height = heightNum;
    this->idleTex = Texture::create(idleTexPath);
    this->pressedTex = Texture::create(pressedTexPath);
}

void GuiButton::update() {
    this->updateTouched();
    if (this->touched) ; // TODO función en otro sitio
}

void GuiButton::draw(Batch *batch) {
    if (this->touched) batch->tex(this->pressedTex, Vec2(this->getX() - App::width()/2, this->getY() - App::height()/2), Color::white);
    else batch->tex(this->idleTex, Vec2(this->getX() - App::width()/2, this->getY() - App::height()/2), Color::white);
}

void GuiButton::updateTouched() {
    Vec2 mouse = Input::mouse_draw();
    int x = (int) mouse.x;
    int y = (int) mouse.y;
    this->touched = this->getX() <= x && this->getX() + this->width >= x && this->getY() <= y && this->getY() + this->height >= y;
}
