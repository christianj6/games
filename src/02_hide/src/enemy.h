#pragma once
#include "object.h"
#include "raylib.h"
#include <vector>

// enum class means we are enforcing stricter typing on the enum members, as opposed to a c-style class
enum class EnemyState {
    PATROL,
    CHASE,
    RETURN_TO_PATROL
};

class Enemy : public GameObject {
private:
    EnemyState current_state;
    std::vector<Vector2> patrol_points;
    // size_t is unsigned int which is as large as any obj in memory; used typically to represent container sizes
    size_t current_patrol_index;
    float vision_angle;  // in radians
    float vision_range;  // how far the enemy can see
    float rotation;      // current facing angle in radians
    float move_speed;
    float rotation_speed;
    Vector2 last_known_player_pos;
    bool can_see_player;
    float search_timeout;    // How long to search last known position
    float search_timer;      // Current search time
    float patrol_update_timer; // Timer for updating patrol points
    float patrol_update_interval; // How often to update patrol points

public:
    Enemy(Vector2 starting_position = {0,0}, int num_patrol_points = 10);
    void generate_patrol_points(int num_points);
    void update(float dt, bool allow_movement = true) override;
    void draw() override;
    void draw_indicator(Vector2);
    void set_patrol_points(const std::vector<Vector2>& points);
    // syntax here means we are passing a vector of game object pointers by reference, promising not to change them
    bool is_player_in_vision_cone(const Vector2& player_pos, const std::vector<GameObject*>& obstacles) const;
    bool is_touching_player(const Vector2& player_pos) const;
    void update_goap(float dt, const Vector2& player_pos, const std::vector<GameObject*>& obstacles);
    bool has_line_of_sight(const Vector2& target_pos, const std::vector<GameObject*>& obstacles) const;
};
