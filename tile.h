#pragma once
#include "/opt/homebrew/include/SDL2/SDL.h"
#include "/opt/homebrew/include/SDL2/SDL_image.h"
#include <iostream>
#include "sprite.h"

class Tile : public Sprite {
public:
        Tile(SDL_Renderer* renderer, SDL_Point pos,
         const std::string& sprite_type = "",
         SDL_Surface* surface = nullptr)
        : sprite_type(sprite_type)
    {
        bool createdInternally = false;
        if (!surface) {
            surface = SDL_CreateRGBSurface(0, TILESIZE, TILESIZE, 32, 0, 0, 0, 0);
            if (!surface) {
                std::cerr << "Failed to create fallback RGB surface for sprite_type: " << sprite_type << std::endl;
                exit(1);
            }
            createdInternally = true;
        }

        texture.reset(SDL_CreateTextureFromSurface(renderer, surface), SDL_DestroyTexture);
        if (!texture) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            if (createdInternally) SDL_FreeSurface(surface);
            exit(1);
        }
        if (sprite_type == "objects" && surface->h > TILESIZE) {
            int dy = surface->h - TILESIZE;
            rect = { pos.x, pos.y - dy, surface->w, surface->h };

            hitbox = {
                rect.x,
                rect.y + (rect.h - TILESIZE),
                rect.w,
                TILESIZE
            };
        } else {
            rect = { pos.x, pos.y, surface->w, surface->h };

            int insetY = 3;
            hitbox = {
                rect.x,
                rect.y + insetY,
                rect.w,
                rect.h - 2 * insetY
            };
        }


        if (createdInternally) SDL_FreeSurface(surface);
    }

    void update() override {} 

    void draw(SDL_Renderer* renderer, SDL_Point offset) override {
        SDL_Rect shifted = {
            rect.x - offset.x,
            rect.y - offset.y,
            rect.w,
            rect.h
        };

        SDL_RenderCopy(renderer, texture.get(), nullptr, &shifted);
    }


    SDL_Rect getRect() const override { return rect; }
    std::string getType() const { return sprite_type; }
    SDL_Rect getHitbox() const override { return hitbox; }
    std::shared_ptr<SDL_Texture> getTexture() const { return texture; }
    
private:
    std::string sprite_type;
    std::shared_ptr<SDL_Texture> texture;
    SDL_Rect rect;
    SDL_Rect hitbox;

};

std::shared_ptr<Tile> createTile(SDL_Renderer* renderer, SDL_Point pos, std::initializer_list<SpriteGroup*> groups, const std::string& sprite_type = "", SDL_Surface* surface = nullptr) {
    auto tile = std::make_shared<Tile>(renderer, pos, sprite_type, surface);
    for (auto* group : groups) {
        group->add(tile);
    }
    return tile;
}
