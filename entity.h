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
                resolveCollision(sprite->getHitbox(), axis);
            }
        }
    }

    void resolveCollision(const SDL_Rect& obstacle, char axis) {
        if (axis == 'x') {
            if (normalizedDirection.x > 0) {
                hitbox.x = obstacle.x - hitbox.w;
            } else if (normalizedDirection.x < 0) {
                hitbox.x = obstacle.x + obstacle.w;
            }
        }

        if (axis == 'y') {
            if (normalizedDirection.y > 0) {
                hitbox.y = obstacle.y - hitbox.h;
            } else if (normalizedDirection.y < 0) {
                hitbox.y = obstacle.y + obstacle.h;
            }
        }
    }

    SDL_Rect hitbox;
    SDL_FPoint normalizedDirection{0, 0};
    SpriteGroup* obstacleGroup = nullptr;
    float frame_index = 0.0f;
    float animation_speed = 0.15f;
};
