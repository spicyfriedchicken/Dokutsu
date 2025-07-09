#pragma once
#include "/opt/homebrew/include/SDL2/SDL.h"
#include "/opt/homebrew/include/SDL2/SDL_image.h"
#include <iostream>
#include <memory>
#include "sprite.h"
#include "player.h"

class Magic : public Sprite {
public:
    Magic(SDL_Renderer* renderer, std::shared_ptr<Player> player, const std::string& texture_path = "") {
        SDL_Surface* surface = nullptr;
        bool createdInternally = false;
        std::cout << "Magic constructor called" << std::endl;
        if (texture_path.empty()) {
            surface = SDL_CreateRGBSurface(0, 40, 40, 32, 0, 0, 0, 0);
            if (!surface) {
                std::cerr << "Failed to create default magic surface: " << SDL_GetError() << std::endl;
                exit(1);
            }
            SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 0, 0, 0));
            createdInternally = true;
        } else {
            std::string status = player->getStatus();
            status = status.substr(0, status.find('_'));
            std::string full_path = texture_path;

            surface = IMG_Load(full_path.c_str());
            if (!surface) {
                std::cerr << "Failed to load magic texture: " << full_path << " | " << IMG_GetError() << std::endl;
                exit(1);
            }
        }

        texture.reset(SDL_CreateTextureFromSurface(renderer, surface), SDL_DestroyTexture);
        SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND);
        if (!texture) {
            std::cerr << "Faigled to create magic texture\n";
            if (createdInternally && surface) SDL_FreeSurface(surface);
            exit(1);
        }

        const SDL_Rect& player_rect = player->getRect();
        rect.w = surface->w;
        rect.h = surface->h;

        std::string status = player->getStatus();
        status = status.substr(0, status.find('_'));

        if (status == "right") {
            rect.x = player_rect.x + player_rect.w;
            rect.y = player_rect.y + (player_rect.h / 2);
        } else if (status == "left") {
            rect.x = player_rect.x - rect.w;
            rect.y = player_rect.y + (player_rect.h / 2);
        } else if (status == "down") {
            rect.x = player_rect.x + (player_rect.w / 2);
            rect.y = player_rect.y + player_rect.h;
        } else {
            rect.x = player_rect.x;
            rect.y = player_rect.y - rect.h;
        }

        hitbox = rect;
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
    SDL_Rect getHitbox() const override { return hitbox; }
    std::shared_ptr<SDL_Texture> getTexture() const { return texture; }

    ~Magic() {
    }
private:
    std::shared_ptr<SDL_Texture> texture;
    SDL_Rect rect;
    SDL_Rect hitbox;
};



std::shared_ptr<Magic> createMagic(SDL_Renderer* renderer,
                                     std::shared_ptr<Player> player,
                                     std::initializer_list<SpriteGroup*> groups,
                                     const std::string& texture_path = "") {
    auto magic = std::make_shared<Magic>(renderer, player);
    for (auto* group : groups) {
        group->add(magic);
    }
    return magic;
}
