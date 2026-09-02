#include "enemy.h"
#include "raylib.h"
#include "raymath.h"
#include "systems/runtime/audio.h"
#include "utils/graphics/renderer.h"
#include "utils/random.h"
#include <cmath>

namespace {
constexpr float kVisionRange = 25.0f;
constexpr float kVisionFovH = 1.5707964f; // 90 deg horizontal, as in 04_teleport
constexpr float kVisionFovV = 1.0471976f; // 60 deg vertical
constexpr float kShootRange = 10.0f;
constexpr float kKillRange = 2.5f;
} // namespace

Enemy::Enemy(Renderer *renderer, Vector3 guard_post)
    : guard_post_(guard_post) {
  model_ = LoadModel("models/enemy.obj");
  if (model_.meshCount > 0) {
    model_loaded_ = true;
    if (renderer != nullptr)
      model_.materials[0].shader = renderer->get_shader();
    BoundingBox box = GetModelBoundingBox(model_);
    float height = box.max.y - box.min.y;
    model_scale_ = height > 0.01f ? 1.8f / height : 1.0f;
  }
  build_tree();
  // Per-enemy hum: each guard owns its buffer so volume/pan can differ.
  buzz_ = LoadSound("audio/buzz.wav");
  buzz_ready_ = IsSoundValid(buzz_);
}

Enemy::~Enemy() {
  if (buzz_ready_) {
    StopSound(buzz_);
    UnloadSound(buzz_);
  }
  if (model_loaded_)
    UnloadModel(model_);
}

void Enemy::build_tree() {
  using namespace BT;

  factory_.registerSimpleCondition(
      "IsFarFromPost", [this](TreeNode &) {
        Vector3 d = Vector3Subtract(current_position, guard_post_);
        return sqrtf(d.x * d.x + d.z * d.z) > leash_radius_
                   ? NodeStatus::SUCCESS
                   : NodeStatus::FAILURE;
      });
  factory_.registerSimpleAction(
      "ReturnToPost", [this](TreeNode &) -> NodeStatus {
        // Hard leash: disengage and walk home so guards stay distributed.
        alert_ = 0.0f;
        move_toward(ctx_.world, guard_post_, 3.0f, ctx_.dt);
        return NodeStatus::SUCCESS;
      });
  factory_.registerSimpleCondition(
      "IsAlerted", [this](TreeNode &) {
        return alert_ >= 1.0f ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
      });
  factory_.registerSimpleCondition(
      "IsSuspicious", [this](TreeNode &) {
        return alert_ >= 0.35f ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
      });
  factory_.registerSimpleAction("ChasePlayer", [this](TreeNode &) -> NodeStatus {
    Vector3 player = ctx_.blackboard->current_player_position;
    move_toward(ctx_.world, player, 3.2f, ctx_.dt);

    shoot_cooldown_ -= ctx_.dt;
    if (ctx_.player_distance < kShootRange && ctx_.sees_player &&
        shoot_cooldown_ <= 0.0f) {
      shoot_cooldown_ = 1.5f;
      tracer_from_ = {current_position.x, current_position.y + 0.8f,
                      current_position.z};
      tracer_to_ = player;
      tracer_timer_ = 0.12f;
      ctx_.blackboard->player_health -= 10.0f;
      ctx_.blackboard->damage_flash = 1.0f;
      Audio::get().play_at(Sfx::Shot, ctx_.blackboard->current_player_position,
                           current_position, ctx_.blackboard->listener_right,
                           60.0f, 0.8f);
      Audio::get().play(Sfx::Hit, 0.7f);
    }
    // SyncActionNode must return SUCCESS/FAILURE; the tree re-ticks every
    // frame from the root, so one frame of chase per tick is the unit of work.
    return NodeStatus::SUCCESS;
  });
  factory_.registerSimpleAction("Investigate", [this](TreeNode &) -> NodeStatus {
    move_toward(ctx_.world, last_seen_, 2.0f, ctx_.dt);
    Vector3 d = {last_seen_.x - current_position.x, 0.0f,
                 last_seen_.z - current_position.z};
    if (sqrtf(d.x * d.x + d.z * d.z) < 1.0f)
      return NodeStatus::SUCCESS; // arrived; alert decays back to patrol
    return NodeStatus::SUCCESS; // not arrived yet; re-tick next frame
  });
  factory_.registerSimpleAction("Patrol", [this](TreeNode &) -> NodeStatus {
    if (!has_target_ && idle_timer_ <= 0.0f)
      pick_patrol_target(ctx_.world);
    if (has_target_) {
      move_toward(ctx_.world, target_, 1.5f, ctx_.dt);
      Vector3 d = {target_.x - current_position.x, 0.0f,
                   target_.z - current_position.z};
      if (sqrtf(d.x * d.x + d.z * d.z) < 0.8f) {
        has_target_ = false;
        RandomNumberGenerator<float> idle(0.5f, 3.0f);
        idle_timer_ = idle();
      }
    } else {
      idle_timer_ -= ctx_.dt;
    }
    return NodeStatus::SUCCESS;
  });

  tree_ = factory_.createTreeFromText(R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="EnemyBrain">
        <Fallback>
          <Sequence>
            <Condition ID="IsFarFromPost"/>
            <Action ID="ReturnToPost"/>
          </Sequence>
          <Sequence>
            <Condition ID="IsAlerted"/>
            <Action ID="ChasePlayer"/>
          </Sequence>
          <Sequence>
            <Condition ID="IsSuspicious"/>
            <Action ID="Investigate"/>
          </Sequence>
          <Action ID="Patrol"/>
        </Fallback>
      </BehaviorTree>
    </root>
  )");
}

void Enemy::lazy_init(World *world) {
  initialized_ = true;
  current_position = {guard_post_.x,
                      world->get_floor_height(guard_post_.x, guard_post_.z) +
                          0.9f,
                      guard_post_.z};
}

void Enemy::pick_patrol_target(World *world) {
  RandomNumberGenerator<float> angle(0.0f, 6.2831853f);
  RandomNumberGenerator<float> dist(4.0f, 12.0f);
  for (int attempt = 0; attempt < 10; ++attempt) {
    float x = guard_post_.x + cosf(angle()) * dist();
    float z = guard_post_.z + sinf(angle()) * dist();
    if (world->get_floor_height(x, z) <= 2.5f) {
      target_ = {x, 0.0f, z};
      has_target_ = true;
      return;
    }
  }
  has_target_ = false;
}
bool Enemy::can_see_player(Blackboard &blackboard) {
  World *world = blackboard.world;
  Vector3 to_player = Vector3Subtract(blackboard.current_player_position,
                                      current_position);
  float dist = Vector3Length(to_player);
  ctx_.player_distance = dist;
  if (dist > kVisionRange)
    return false;
  if (dist < 0.01f)
    return true;

  Vector3 dir = Vector3Scale(to_player, 1.0f / dist);

  // Horizontal cone: 90 degrees around facing (04_teleport convention).
  float yaw = heading_deg_ * DEG2RAD;
  Vector3 facing = {sinf(yaw), 0.0f, cosf(yaw)};
  float h_dot = dir.x * facing.x + dir.z * facing.z;
  if (h_dot > 1.0f) h_dot = 1.0f;
  if (h_dot < -1.0f) h_dot = -1.0f;
  float horizontal_angle = acosf(h_dot);
  if (horizontal_angle > kVisionFovH * 0.5f)
    return false;

  // Vertical cone: 60 degrees — a player perched directly overhead is unseen.
  float v_sin = dir.y;
  if (v_sin > 1.0f) v_sin = 1.0f;
  if (v_sin < -1.0f) v_sin = -1.0f;
  float vertical_angle = asinf(v_sin);
  if (fabsf(vertical_angle) > kVisionFovV * 0.5f)
    return false;

  Vector3 eye = {current_position.x, current_position.y + 0.8f,
                 current_position.z};
  return world->has_line_of_sight(eye, blackboard.current_player_position);
}

void Enemy::draw_vision_cone() {
  Color base = alert_ >= 1.0f ? RED : (alert_ >= 0.35f ? ORANGE : YELLOW);
  float ground_y = current_position.y - 0.9f + 0.06f; // hug the floor top
  Vector3 origin = {current_position.x, ground_y, current_position.z};
  const int segments = 16;
  const float half = kVisionFovH * 0.5f;
  float yaw = heading_deg_ * DEG2RAD;

  // Range rings across the cone (like 04_teleport's wireframe rings).
  const float radii[3] = {kVisionRange * 0.33f, kVisionRange * 0.66f,
                          kVisionRange};
  for (int ri = 0; ri < 3; ++ri) {
    float r = radii[ri];
    Vector3 prev{};
    for (int s = 0; s <= segments; ++s) {
      float a = yaw - half + (kVisionFovH * s / segments);
      Vector3 p = {origin.x + sinf(a) * r, origin.y, origin.z + cosf(a) * r};
      if (s > 0)
        DrawLine3D(prev, p, ColorAlpha(base, 0.35f));
      prev = p;
    }
  }
  // Edge radials + one center line.
  for (int e = 0; e <= 4; ++e) {
    float a = yaw - half + (kVisionFovH * e / 4.0f);
    Vector3 p = {origin.x + sinf(a) * kVisionRange, origin.y,
                 origin.z + cosf(a) * kVisionRange};
    DrawLine3D(origin, p, ColorAlpha(base, 0.25f));
  }
}

void Enemy::move_toward(World *world, Vector3 target, float speed, float dt) {
  Vector3 delta = {target.x - current_position.x, 0.0f,
                   target.z - current_position.z};
  float len = sqrtf(delta.x * delta.x + delta.z * delta.z);
  if (len < 0.05f)
    return;
  delta.x /= len;
  delta.z /= len;

  float step = speed * dt;

  // Try the desired direction, then progressively wider detours — guards
  // skirt around pillars instead of grinding into their faces.
  static const float detours[] = {0.0f,  0.6f,  -0.6f, 1.2f,  -1.2f,
                                  1.8f,  -1.8f, 2.4f,  -2.4f, 2.9f,
                                  -2.9f};
  for (float a : detours) {
    float c = cosf(a);
    float s = sinf(a);
    Vector3 dir = {delta.x * c - delta.z * s, 0.0f, delta.x * s + delta.z * c};
    Vector3 full = {current_position.x + dir.x * step, current_position.y,
                    current_position.z + dir.z * step};
    Vector3 probe = {full.x, world->get_floor_height(full.x, full.z) + 2.0f,
                     full.z};
    if (world->position_is_acceptable(probe)) {
      current_position.x = full.x;
      current_position.z = full.z;
      // Face where we actually walked, so the vision cone follows.
      heading_deg_ = atan2f(dir.x, dir.z) * RAD2DEG;
      return;
    }
  }
  has_target_ = false; // fully boxed in: patrol picks a new waypoint
}

void Enemy::incapacitate() {
  incapacitated_ = true;
  alert_ = 0.0f;
  revive_timer_ = 0.0f;
  has_target_ = false;
}

void Enemy::revive() {
  incapacitated_ = false;
  alert_ = 0.0f;
  revive_timer_ = 0.0f;
  has_target_ = false;
}

void Enemy::set_revive_target(Enemy *ally) { revive_target_ = ally; }

MovementUpdate Enemy::update(float dt, Blackboard &blackboard) {
  World *world = blackboard.world;
  if (world == nullptr)
    return {};
  if (!initialized_)
    lazy_init(world);

  if (incapacitated_) {
    // Downed: no vision, no AI, silent until revived.
    if (buzz_ready_ && IsSoundPlaying(buzz_))
      StopSound(buzz_);
    return {};
  }

  // Contextual takedown: behind an unaware guard = incapacitate;
  // otherwise (spotted / face-to-face) = kill.
  Vector3 to_player = Vector3Subtract(blackboard.current_player_position,
                                      current_position);
  float dist = Vector3Length(to_player);
  if (dist < kKillRange) {
    bool attack = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
                  IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
    if (attack) {
      float yaw = heading_deg_ * DEG2RAD;
      Vector3 facing = {sinf(yaw), 0.0f, cosf(yaw)};
      Vector3 flat = Vector3Normalize({to_player.x, 0.0f, to_player.z});
      bool behind = Vector3DotProduct(facing, flat) < 0.0f;
      if (behind && alert_ < 1.0f) {
        Audio::get().play_at(Sfx::Takedown, blackboard.current_player_position,
                             current_position, blackboard.listener_right,
                             40.0f, 0.9f);
        incapacitate();
        return {};
      }
      Audio::get().play_at(Sfx::Kill, blackboard.current_player_position,
                           current_position, blackboard.listener_right, 50.0f,
                           0.8f);
      dead_ = true;
      return {};
    }
    bool behind = [&] {
      float yaw = heading_deg_ * DEG2RAD;
      Vector3 facing = {sinf(yaw), 0.0f, cosf(yaw)};
      Vector3 flat = Vector3Normalize({to_player.x, 0.0f, to_player.z});
      return Vector3DotProduct(facing, flat) < 0.0f;
    }();
    if (behind && alert_ < 1.0f)
      blackboard.takedown_available = true;
    else
      blackboard.attack_available = true;
  }

  // Detection: fills while visible (faster up close), drains otherwise.
  bool sees = can_see_player(blackboard);
  if (sees) {
    last_seen_ = blackboard.current_player_position;
    float rate = 2.0f * (1.0f - dist / kVisionRange) + 0.5f;
    alert_ += rate * dt;
    if (alert_ > 1.0f)
      alert_ = 1.0f;
  } else {
    alert_ -= 0.5f * dt;
    if (alert_ < 0.0f)
      alert_ = 0.0f;
  }
  if (alert_ >= 1.0f && !alert_announced_) {
    alert_announced_ = true;
    Audio::get().play_at(Sfx::Alert, blackboard.current_player_position,
                         current_position, blackboard.listener_right, 45.0f,
                         0.7f);
  } else if (alert_ < 0.9f) {
    alert_announced_ = false;
  }

  ctx_.blackboard = &blackboard;
  ctx_.world = world;
  ctx_.dt = dt;
  ctx_.sees_player = sees;

  // Stuck detection: active behaviors that stop making progress trigger a
  // short perpendicular escape maneuver (works even when wedged in geometry).
  float moved = Vector3Length(Vector3Subtract(current_position, last_position_));
  bool wants_move = revive_target_ != nullptr || has_target_ || alert_ >= 0.35f;
  if (wants_move && moved < 0.015f)
    stuck_timer_ += dt;
  else
    stuck_timer_ = 0.0f;
  last_position_ = current_position;

  if (unstuck_timer_ > 0.0f) {
    unstuck_timer_ -= dt;
    Vector3 escape = {current_position.x + unstuck_dir_.x * 4.0f,
                      current_position.y,
                      current_position.z + unstuck_dir_.z * 4.0f};
    move_toward(world, escape, 2.8f, dt);
  } else if (stuck_timer_ > 0.6f) {
    RandomNumberGenerator<int> coin(0, 1);
    float yaw = heading_deg_ * DEG2RAD;
    Vector3 fwd = {sinf(yaw), 0.0f, cosf(yaw)};
    float side = coin() == 0 ? 1.0f : -1.0f;
    unstuck_dir_ = {fwd.z * side, 0.0f, -fwd.x * side};
    unstuck_timer_ = 0.6f;
    stuck_timer_ = 0.0f;
  } else if (revive_target_ != nullptr) {
    // Revive duty overrides everything: walk to the ally, stand, lift them.
    Vector3 d = Vector3Subtract(revive_target_->get_position(),
                                current_position);
    float ally_dist = Vector3Length(d);
    if (ally_dist > 2.0f) {
      move_toward(world, revive_target_->get_position(), 3.0f, dt);
    } else {
      revive_timer_ += dt;
      if (revive_timer_ >= 1.5f) {
        Audio::get().play_at(Sfx::Revive, blackboard.current_player_position,
                             revive_target_->get_position(),
                             blackboard.listener_right, 50.0f, 0.8f);
        revive_target_->revive();
        revive_timer_ = 0.0f;
      }
    }
  } else {
    tree_.tickOnce();
  }

  if (tracer_timer_ > 0.0f)
    tracer_timer_ = std::fmax(0.0f, tracer_timer_ - dt);

  current_position.y =
      world->get_floor_height(current_position.x, current_position.z) + 0.9f;

  // Robotic hum: audible from ~70 units, panned with the listener.
  if (buzz_ready_) {
    float d = dist;
    float att = 1.0f - powf(d / 70.0f, 1.5f);
    if (att > 0.0f) {
      Vector3 dir = d > 0.001f ? Vector3Scale(to_player, 1.0f / d)
                               : Vector3{0.0f, 0.0f, 1.0f};
      SetSoundVolume(buzz_, 0.5f * att);
      SetSoundPan(buzz_, Vector3DotProduct(dir, blackboard.listener_right));
      if (!IsSoundPlaying(buzz_))
        PlaySound(buzz_);
    } else if (IsSoundPlaying(buzz_)) {
      StopSound(buzz_);
    }
  }
  return {};
}
void Enemy::draw() {
  float yaw = heading_deg_ * DEG2RAD;
  Vector3 fwd = {sinf(yaw), 0.0f, cosf(yaw)};

  if (incapacitated_) {
    // Lying on the ground, facing side up.
    Vector3 right = {fwd.z, 0.0f, -fwd.x};
    float gy = current_position.y - 0.9f + 0.35f;
    Vector3 c = {current_position.x, gy, current_position.z};
    if (model_loaded_) {
      DrawModelEx(model_, {current_position.x, gy, current_position.z}, fwd,
                  90.0f, {model_scale_, model_scale_, model_scale_}, GRAY);
    } else {
      DrawCylinderEx(Vector3Add(c, Vector3Scale(right, -0.6f)),
                     Vector3Add(c, Vector3Scale(right, 0.6f)), 0.3f, 0.3f, 8,
                     DARKGRAY);
      DrawSphere(Vector3Add(c, Vector3Scale(right, 0.75f)), 0.26f, MAROON);
    }
    return;
  }

  if (model_loaded_) {
    DrawModelEx(model_, current_position, {0.0f, 1.0f, 0.0f}, heading_deg_,
                {model_scale_, model_scale_, model_scale_}, WHITE);
  } else {
    DrawCylinder(current_position, 0.35f, 0.35f, 1.2f, 10, MAROON);
    DrawSphere({current_position.x, current_position.y + 0.95f,
                current_position.z}, 0.3f, RED);
  }
  draw_vision_cone();

  // Detection meter: colored marker above the head (Dishonored-style).
  Color c = alert_ >= 1.0f ? RED : (alert_ >= 0.35f ? ORANGE : LIME);
  Vector3 m = {current_position.x, current_position.y + 2.1f,
               current_position.z};
  DrawSphere(m, 0.09f + 0.08f * alert_, c);
  if (alert_ >= 1.0f)
    DrawCylinder({m.x, m.y + 0.25f, m.z}, 0.04f, 0.04f, 0.25f, 6, RED);

  // Shot tracer
  if (tracer_timer_ > 0.0f) {
    Color tc = {255, 60, 60, (unsigned char)(255 * tracer_timer_ / 0.12f)};
    DrawLine3D(tracer_from_, tracer_to_, tc);
  }
}
