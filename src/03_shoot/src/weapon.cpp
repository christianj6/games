#include "weapon.h"

Weapon::Weapon(Vector3 pos) : position(pos) {}

void Weapon::draw() { DrawSphere(position, RADIUS, BLUE); }
