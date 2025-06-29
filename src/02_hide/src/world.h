#pragma once
#include <vector>
#include <memory>
#include "object.h"
#include "player.h"

class World {
private:
    // >> closes the type declaration when we have nested, templated objects
    std::vector<std::unique_ptr<GameObject>> objects;
    std::unique_ptr<Player> player_ptr = nullptr;
    Camera2D camera;

    bool is_on_screen(const Vector2& position) const;

public:
    World() {
        // TODO: shift to implementation file
        camera.target = {0, 0};
        camera.offset = {GetScreenWidth()/2.0f, GetScreenHeight()/2.0f};
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
    }
    ~World() = default;

    void add_object(std::unique_ptr<GameObject> object);
    bool update(float dt);
    void draw();         
    
    // returns raw pointer to the player without transferring ownership
    Player* get_player_ptr() const { return player_ptr.get(); }
    // we move the incoming pointer to the class attr, transferring ownership
    void set_player(std::unique_ptr<Player> p) { player_ptr = std::move(p); }
    std::vector<GameObject*> get_obstacles() const;
};
