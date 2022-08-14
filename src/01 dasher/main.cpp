#include "raylib.h"
#include <tuple>
#include <iostream>
#include <string> 

const int gravity = 1000;
const int collisionPadding = 22;
struct window
{
    const int width;
    const int height;
    const char * title;
};
struct texture
{
    Texture2D data;
    const int nRows;
    const int nColumns;
};
struct object
{
    int currentTextureFrame = 0;
    texture texture;
    Color color;
    Rectangle rectangle;
    Vector2 position;
    const float movementSpeed = 200.0;
    const float jumpSpeed = 650;
    float verticalMovement = 0;
    float frameUpdateFrequency;
    float timeSinceLastFrameUpdate = 0;
};
void setupWindow(window window)
{
    InitWindow(window.width, window.height, window.title);
    SetTargetFPS(60);    
}
void drawObject(object object)
{
    DrawTextureRec(object.texture.data, object.rectangle, object.position, object.color);
}
bool updateObject(object& object, float timeSinceLastFrame, window& window)
{
    bool positionReset = false;

    if (object.timeSinceLastFrameUpdate > object.frameUpdateFrequency) {
        object.timeSinceLastFrameUpdate = 0;
        object.currentTextureFrame++;
        object.rectangle.x = object.currentTextureFrame * object.rectangle.width;
    }
    if (object.currentTextureFrame >= object.texture.nColumns) {
        object.currentTextureFrame = 0;
    }
    object.timeSinceLastFrameUpdate += timeSinceLastFrame;    
    object.position.x -= object.movementSpeed * timeSinceLastFrame;
    if (object.position.x + object.rectangle.width < 0) {
        object.position.x = window.width * 2;
        positionReset = true;
    }
    return positionReset;
}
void handleJumpInput(object& object, float timeSinceLastFrame, window& window, bool objectOnGround) {
    if (objectOnGround) {
        object.verticalMovement = 0;
    } else {
        object.verticalMovement += gravity * timeSinceLastFrame;
    }
    if (IsKeyPressed(KEY_SPACE) && objectOnGround) {
        object.verticalMovement -= object.jumpSpeed;
    }
    object.position.y += object.verticalMovement * timeSinceLastFrame;
}
bool evaluateCollision(object& player, object (&nebulae)[2]) {

    bool isCollision;

    for (auto nebula : nebulae) {
        if (
                nebula.position.x > player.position.x + collisionPadding && 
                nebula.position.x < player.position.x + player.rectangle.width - collisionPadding &&
                nebula.position.y < player.position.y + player.rectangle.height - collisionPadding &&
                nebula.position.y > player.position.y + collisionPadding 
        ) {
            // check if player rectangle collides with nebula
            isCollision = true;
        }
    }

    return isCollision;
}


int main()
{
    window window{512, 380, "Dapper Dasher"};
    setupWindow(window);
    texture textures[] = {
        {
            .data = LoadTexture("./res/scarfy.png"),
            .nRows = 1,
            .nColumns = 6
        },
        {
            .data = LoadTexture("./res/12_nebula_spritesheet.png"),
            .nRows = 8,
            .nColumns = 8
        }
    };
    object player = {
        .texture = textures[0],
        .color = WHITE,
        .rectangle = {0, 0, textures[0].data.width / textures[0].nColumns, textures[0].data.height / textures[0].nRows},
        .position = {
            window.width / 2 - textures[0].data.width / textures[0].nColumns,
            window.height - textures[0].data.height / textures[0].nRows,            
        },
        .movementSpeed = 0,
        .frameUpdateFrequency = 1.0 / 12.0
    };
    object nebulae[] = {
        {
            .texture = textures[1],
            .color = WHITE,
            .rectangle = {0, 0, textures[1].data.width / textures[1].nColumns, textures[1].data.height / textures[1].nRows},
            .position = {
                (window.width - textures[1].data.width / textures[1].nColumns) * 2.2,  // place slightly off screen              
                window.height - textures[1].data.height / textures[1].nRows,
            },
            .frameUpdateFrequency = 1.0 / 12.0            
        },
        {
            .texture = textures[1],
            .color = RED,
            .rectangle = {0, 0, textures[1].data.width / textures[1].nColumns, textures[1].data.height / textures[1].nRows},
            .position = {
                (window.width - textures[1].data.width / textures[1].nColumns) * 1.2,   // place slightly off screen           
                window.height - textures[1].data.height / textures[1].nRows,
            },
            .frameUpdateFrequency = 1.0 / 12.0           
        }
    };
    int nNebulaeAvoided = 0;
    char const *gameOverText = "Game Over";
    bool gameOver = false;

    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);
        BeginDrawing();

            // todo: scroll the background
            // todo: debug collision behaviour wrt padding
            // todo: tidy and finish the game

            float timeSinceLastFrame = GetFrameTime();
            bool playerIsHit = evaluateCollision(player, nebulae);
            if (playerIsHit) {
                gameOver = true;
            }
            bool nebulaIsAvoided = false;
            bool playerOnGround = player.position.y + player.rectangle.height >= window.height;
            if (playerOnGround){
                updateObject(player, timeSinceLastFrame, window);
            }
            handleJumpInput(player, timeSinceLastFrame, window, playerOnGround);
            if (!gameOver){
                drawObject(player);
            }
            for (int i = 0; i < 2; i++) // todo: remove hard-coded array length
            {
                nebulaIsAvoided = updateObject(nebulae[i], timeSinceLastFrame, window);
                if (!gameOver){
                    drawObject(nebulae[i]);
                }
                if (nebulaIsAvoided) {
                    nNebulaeAvoided++;
                }                
            }
            // convert into to char pointer
            std::string avoidedCounter = std::to_string(nNebulaeAvoided);
            char const *pchar = avoidedCounter.c_str();
            if (!gameOver) {
                DrawText(pchar, 10, 10, 50, RED);  
            } else {
                DrawText(gameOverText, 10, 10, 50, RED);  
            }


        EndDrawing();
    }
    for (auto texture : textures)
    {
        UnloadTexture(texture.data);
    }
}