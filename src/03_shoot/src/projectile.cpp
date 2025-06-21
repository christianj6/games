#include "projectile.h"
#include "raymath.h"

Projectile::Projectile(Vector3 start_pos, Vector3 direction) {
  position = start_pos;
  velocity = Vector3Scale(direction, SPEED);
}

void Projectile::update(float dt) {
  lifetime += dt;
  if (lifetime >= MAX_LIFETIME) {
    active = false;
    return;
  }

  position = Vector3Add(position, Vector3Scale(velocity, dt));
}

void Projectile::draw() { DrawSphere(position, RADIUS, BLUE); }
