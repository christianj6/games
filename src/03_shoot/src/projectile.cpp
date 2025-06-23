#include "projectile.h"
#include "raymath.h"

Projectile::Projectile(Vector3 start_pos, Vector3 direction) {
  // just a starting position (player position) and direction (camera direction)
  position = start_pos;
  velocity = Vector3Scale(direction, SPEED);
}

void Projectile::update(float dt) {
  // lifetime prevents from being too many objects to render
  lifetime += dt;
  if (lifetime >= MAX_LIFETIME) {
    active = false;
    return;
  }
  // raylib seems strangely performant; even if i remove the above and spawn a
  // bunch of balls the framerate does not dip; i wonder why

  position = Vector3Add(position, Vector3Scale(velocity, dt));
}

void Projectile::draw() { DrawSphere(position, RADIUS, BLUE); }
