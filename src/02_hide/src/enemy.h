#pragma once
#include "object.h"
#include "raylib.h"
#include <vector>

enum class EnemyState {
    PATROL,
    CHASE,
    RETURN_TO_PATROL
};

class Enemy : public GameObject {
private:
    EnemyState current_state;
    std::vector<Vector2> patrol_points;
    size_t current_patrol_index;
    float vision_angle;  // in radians
    float vision_range;  // how far the enemy can see
    float rotation;      // current facing angle in radians
    float move_speed;
    float rotation_speed;
    Vector2 last_known_player_pos;
    bool can_see_player;

public:
    Enemy(Vector2 starting_position = {0,0});
    void update(float dt, bool allow_movement = true) override;
    void draw() override;
    void draw_indicator(Vector2);
    void set_patrol_points(const std::vector<Vector2>& points);
    bool is_player_in_vision_cone(const Vector2& player_pos, const std::vector<GameObject*>& obstacles) const;
    void update_goap(float dt, const Vector2& player_pos, const std::vector<GameObject*>& obstacles);
    bool has_line_of_sight(const Vector2& target_pos, const std::vector<GameObject*>& obstacles) const;
};
