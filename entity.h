#pragma once
#include "sprite.h"
#include "support.h"

class Entity : public Sprite {
public:

    virtual ~Entity() = default;

    void move(float dx, float dy) {
        hitbox.x += static_cast<int>(dx);
        hitbox.y += static_cast<int>(dy);
    }

    bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
        return SDL_HasIntersection(&a, &b);
    }

    void handleCollision(char axis) {
        for (const auto& sprite : obstacleGroup->getSprites()) {
            if (checkCollision(hitbox, sprite->getHitbox())) {
                std::cout << "[Collision Detected] Axis: " << axis
                          << " | Player Hitbox: " << hitbox.x << "," << hitbox.y
                          << " | Obstacle Hitbox: " << sprite->getHitbox().x << "," << sprite->getHitbox().y << "\n";

                resolveCollision(sprite->getHitbox(), axis);
            }
        }
    }


    void resolveCollision(const SDL_Rect& obstacle, char axis) {
        if (axis == 'x') {
            int overlap;
            if (hitbox.x < obstacle.x) {
                overlap = (hitbox.x + hitbox.w) - obstacle.x;
                hitbox.x -= overlap;
            } else {
                overlap = (obstacle.x + obstacle.w) - hitbox.x;
                hitbox.x += overlap;
            }
            std::cout << "[Resolved X] New hitbox.x = " << hitbox.x << ", overlap = " << overlap << "\n";
        }

        if (axis == 'y') {
            int overlap;
            if (hitbox.y < obstacle.y) {
                overlap = (hitbox.y + hitbox.h) - obstacle.y;
                hitbox.y -= overlap;
            } else {
                overlap = (obstacle.y + obstacle.h) - hitbox.y;
                hitbox.y += overlap;
            }
            std::cout << "[Resolved Y] New hitbox.y = " << hitbox.y << ", overlap = " << overlap << "\n";
        }
    }



    SDL_Rect hitbox;
    SDL_FPoint normalizedDirection{0, 0};
    SpriteGroup* obstacleGroup = nullptr;
    float frame_index = 0.0f;
    float animation_speed = 0.15f;
};
