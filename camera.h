#pragma once
#include "sprite.h"
#include "tile.h"
#include "player.h"
#include "weapon.h"
#include "enemy.h"
#include "settings.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

class Camera {
public:
    Camera(SDL_Renderer* renderer, SpriteGroup* group)
        : renderer(renderer), visibleGroup(group) {

        SDL_Surface* floor_surface = IMG_Load("./graphics/tilemap/ground.png");
        if (!floor_surface) {
            std::cerr << "Failed to load image 'tilemap.png': " << IMG_GetError() << std::endl;
            exit(1);
        }

        texture.reset(SDL_CreateTextureFromSurface(renderer, floor_surface), SDL_DestroyTexture);
        if (!texture) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            SDL_FreeSurface(floor_surface);
            exit(1);
        }

        floor_rect = {
            0, 0,
            floor_surface->w,
            floor_surface->h
        };

        SDL_FreeSurface(floor_surface);
    }

    void centerOn(const SDL_Rect& target) {
        offset.x = target.x + target.w / 2 - WIDTH / 2;
        offset.y = target.y + target.h / 2 - HEIGHT / 2;
    }

    SDL_Point getOffset() const {
        return offset;
    }

    void draw() {
        SDL_Rect view = { offset.x, offset.y, WIDTH, HEIGHT };

        // --- Floor ---
        SDL_Rect shifted_floor = {
            floor_rect.x - offset.x,
            floor_rect.y - offset.y,
            floor_rect.w,
            floor_rect.h
        };
        SDL_RenderCopy(renderer, texture.get(), nullptr, &shifted_floor);

        // --- Visible Sprites (Sorted by Y for depth) ---
        std::vector<std::shared_ptr<Sprite>> visible;
        for (const auto& sprite : visibleGroup->getSprites()) {
            SDL_Rect rect = sprite->getRect();
            if (SDL_HasIntersection(&rect, &view)) {
                visible.push_back(sprite);
            }
        }

        std::sort(visible.begin(), visible.end(), [](const auto& a, const auto& b) {
            return (a->getRect().y + a->getRect().h) < (b->getRect().y + b->getRect().h);
        });

        for (const auto& sprite : visible) {
            sprite->draw(renderer, offset);  // All Sprite::draw must accept SDL_Point offset
        }
    }

private:
    SDL_Renderer* renderer = nullptr;
    SpriteGroup* visibleGroup = nullptr;
    std::shared_ptr<SDL_Texture> texture;
    SDL_Rect floor_rect;
    SDL_Point offset{0, 0};
};
