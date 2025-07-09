#pragma once
#include "/opt/homebrew/include/SDL2/SDL.h"
#include "/opt/homebrew/include/SDL2/SDL_image.h"
#include <SDL_ttf.h>

#include "settings.h"
#include "player.h"

#include <memory>
#include <string>
#include <iostream>

class UI {
public:
    UI(SDL_Renderer* renderer, std::shared_ptr<Player> player)
        : renderer(renderer), player(std::move(player)) {

        // Load font from settings
        if (TTF_Init() == -1) {
            std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
            exit(1);
        }
        font = TTF_OpenFont(UI_FONT.c_str(), UI_FONT_SIZE);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            exit(1);
        }

        // Setup bar rects
        health_back = {10, 10, HEALTH_BAR_WIDTH, BAR_HEIGHT};
        health_fill = health_back;

        mana_back = {10, 40, MANA_BAR_WIDTH, BAR_HEIGHT};
        mana_fill = mana_back;
    }

    ~UI() {
        if (font) TTF_CloseFont(font);
        if (exp_texture) SDL_DestroyTexture(exp_texture);
        TTF_Quit();
    }

    void update() {
        // Update health/mana bar fill widths
        float health_ratio = static_cast<float>(player->stats.health) / player->maximumHealth;
        float mana_ratio   = static_cast<float>(player->stats.mana) / player->maximumMana;

        health_fill.w = static_cast<int>(health_back.w * health_ratio);
        mana_fill.w   = static_cast<int>(mana_back.w * mana_ratio);

        updateEXP();
        updateWeapon();
        updateMagic();
    }

void render() {
    // Background bars
    SDL_SetRenderDrawColor(renderer, 34, 34, 34, 255);
    SDL_RenderFillRect(renderer, &health_back);
    SDL_RenderFillRect(renderer, &mana_back);

    // Filled bars
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
    SDL_RenderFillRect(renderer, &health_fill);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue
    SDL_RenderFillRect(renderer, &mana_fill);

    // Borders (set color before drawing!)
    SDL_SetRenderDrawColor(renderer, 17, 17, 17, 255); // Restore grey
    drawThiccRect(health_back, 3);
    drawThiccRect(mana_back, 3);

    // EXP text
    renderEXP();
    // EXP text Border
    SDL_SetRenderDrawColor(renderer, 17, 17, 17, 255); // Restore grey
    SDL_Rect r = { exp_rect.x - 4, exp_rect.y - 4, exp_rect.w + 8, exp_rect.h + 8};
    drawThiccRect(r, 3);

    // Weapon background box
    SDL_SetRenderDrawColor(renderer, 34, 34, 34, 255); // dark grey background
    SDL_RenderFillRect(renderer, &weapon_box);
    SDL_SetRenderDrawColor(renderer, 17, 17, 17, 255); // darker outline
    drawThiccRect(weapon_box, 3);

    // Weapon sprite (centered)
    renderWeapon();

    // Weapon background box
    SDL_SetRenderDrawColor(renderer, 34, 34, 34, 255); // dark grey background
    SDL_RenderFillRect(renderer, &magic_box);
    SDL_SetRenderDrawColor(renderer, 17, 17, 17, 255); // darker outline
    drawThiccRect(magic_box, 3);

    // Magic sprite (centered)
    renderMagic();
}


private:
    SDL_Renderer* renderer;
    std::shared_ptr<Player> player;

    // Font
    TTF_Font* font = nullptr;

    // Bars
    SDL_Rect health_back, health_fill;
    SDL_Rect mana_back, mana_fill;

    // EXP caching
    int last_exp = -1;
    SDL_Texture* exp_texture = nullptr;
    SDL_Texture* weapon_texture = nullptr;
    SDL_Texture* magic_texture = nullptr;

    SDL_Rect exp_rect = {};
    SDL_Rect weapon_rect = {};
    SDL_Rect magic_rect = {};
    SDL_Rect weapon_box = {20, 620, ITEM_BOX_SIZE, ITEM_BOX_SIZE};
    SDL_Rect magic_box = {120, 620, ITEM_BOX_SIZE, ITEM_BOX_SIZE};


    void updateEXP() {
        int current_exp = player->exp;
        if (current_exp == last_exp) return;

        if (exp_texture) {
            SDL_DestroyTexture(exp_texture);
            exp_texture = nullptr;
        }

        last_exp = current_exp;
        std::string exp_text = std::to_string(current_exp);
        SDL_Color color = {32, 32, 32, 255}; 

        SDL_Surface* surface = TTF_RenderText_Solid(font, exp_text.c_str(), color);
        if (!surface) return;

        exp_texture = SDL_CreateTextureFromSurface(renderer, surface);
        exp_rect = {1200, 680, surface->w, surface->h};

        SDL_FreeSurface(surface);
    }

    void updateWeapon() {
        std::string full_path = weapon_graphics[weapons[player->weapon_index]] + "full.png";

        SDL_Surface* surface = IMG_Load(full_path.c_str());
        if (!surface) {
            std::cerr << "Failed to load weapon texture: " << full_path << " | " << IMG_GetError() << std::endl;
            exit(1);
        }

        if (weapon_texture) {
            SDL_DestroyTexture(weapon_texture);
            weapon_texture = nullptr;
        }

        weapon_texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!weapon_texture) {
            std::cerr << "Failed to create weapon texture for UI\n";
            SDL_FreeSurface(surface);
            exit(1);
        }

        int weapon_w = surface->w;
        int weapon_h = surface->h;
        SDL_FreeSurface(surface);

        // Center weapon inside the fixed weapon box
        weapon_rect.w = weapon_w;
        weapon_rect.h = weapon_h;
        weapon_rect.x = weapon_box.x + (weapon_box.w - weapon_w) / 2;
        weapon_rect.y = weapon_box.y + (weapon_box.h - weapon_h) / 2;
    }

    void updateMagic() {
        std::string full_path = magic_graphics[magic[player->magic_index]];
        SDL_Surface* surface = IMG_Load(full_path.c_str());
        if (!surface) {
            std::cerr << "Failed to load magic texture: " << full_path << " | " << IMG_GetError() << std::endl;
            exit(1);
        }

        if (magic_texture) {
            SDL_DestroyTexture(magic_texture);
            magic_texture = nullptr;
        }

        magic_texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!magic_texture) {
            std::cerr << "Failed to create magic texture for UI\n";
            SDL_FreeSurface(surface);
            exit(1);
        }

        int magic_w = surface->w;
        int magic_h = surface->h;
        SDL_FreeSurface(surface);

        magic_rect.w = magic_w;
        magic_rect.h = magic_h;
        magic_rect.x = magic_box.x + (magic_box.w - magic_w) / 2;
        magic_rect.y = magic_box.y + (magic_box.h - magic_h) / 2;
    }


    void renderEXP() {
        if (exp_texture) {
            SDL_RenderCopy(renderer, exp_texture, nullptr, &exp_rect);
        }
    }

    void renderWeapon() {
        if (weapon_texture) {
            SDL_RenderCopy(renderer, weapon_texture, nullptr, &weapon_rect);
        }
    }

    void renderMagic() {
        if (magic_texture) {
            SDL_RenderCopy(renderer, magic_texture, nullptr, &magic_rect);
        }
    }

    void drawThiccRect(const SDL_Rect& rect, int thickness) {
        for (int i = 0; i < thickness; ++i) {
            SDL_Rect r = {
                rect.x - i,
                rect.y - i,
                rect.w + i * 2,
                rect.h + i * 2
            };
            SDL_RenderDrawRect(renderer, &r);
        }
    }
};
