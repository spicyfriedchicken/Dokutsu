#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "entity.h"
#include "support.h"
#include "settings.h"

class Weapon;
class Magic;

class Player : public Entity {
public:
    Player(SDL_Renderer* renderer,
       SDL_Point pos,
       std::function<void()> attack_callback,
       std::function<void()> destroy_callback = nullptr,
	   std::function<void()> magic_callback = nullptr)
    : renderer(renderer),
      attack_callback(std::move(attack_callback)),
      magic_callback(std::move(magic_callback)),
      destroy_callback(std::move(destroy_callback)) {



        SDL_Surface* tempSurface = IMG_Load("./graphics/test/player.png");
        if (!tempSurface) {
            std::cerr << "Failed to load image: " << IMG_GetError() << std::endl;
            exit(1);
        }

        texture.reset(SDL_CreateTextureFromSurface(renderer, tempSurface), SDL_DestroyTexture);
        if (!texture) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            SDL_FreeSurface(tempSurface);
            exit(1);
        }

        rect = { pos.x, pos.y, tempSurface->w, tempSurface->h };
        int insetY = 10;
        hitbox = {
            pos.x,
            pos.y + insetY,
            tempSurface->w,
            tempSurface->h - 2 * insetY
        };

        SDL_FreeSurface(tempSurface);

        animations = {
            { "up",   {} }, { "up_idle", {} }, { "up_attack", {} },
            { "down", {} }, { "down_idle", {} }, { "down_attack", {} },
            { "left", {} }, { "left_idle", {} }, { "left_attack", {} },
            { "right", {} }, { "right_idle", {} }, { "right_attack", {} },
        };

        import_player_assets();
    }

void import_player_assets() {
    std::string path = "./graphics/player/";

    for (auto& [k, v] : animations) {
        std::string completePath = path + k;
        std::vector<std::filesystem::directory_entry> entries;
std::error_code ec;
if (!std::filesystem::exists(completePath, ec)) {
    std::cerr << "player::import_player_assets: missing animation folder at: " << completePath << "\n";
    continue;
}

        for (const auto& entry : std::filesystem::directory_iterator(completePath)) {
            if (entry.is_regular_file()) entries.push_back(entry);
        }

        std::sort(entries.begin(), entries.end(),
            [](const auto& a, const auto& b) {
                return a.path().filename() < b.path().filename();
            });

        std::vector<std::shared_ptr<SDL_Surface>> surface_list;
        for (const auto& entry : entries) {
            std::string full_path = entry.path().string();
            std::shared_ptr<SDL_Surface> image(IMG_Load(full_path.c_str()), SDL_FreeSurface);

            if (!image) {
                std::cerr << "Failed to load image: " << full_path << " | " << IMG_GetError() << std::endl;
                continue;
            }

            surface_list.push_back(std::move(image));
        }

        v = std::move(surface_list);
    }
}

    // Animations
    void get_status() {
        std::string old_status = status;
        std::string base_direction;

        if (direction.x < 0)      base_direction = "left";
        else if (direction.x > 0) base_direction = "right";
        else if (direction.y < 0) base_direction = "up";
        else if (direction.y > 0) base_direction = "down";
        else                      base_direction = status.substr(0, status.find('_'));

        if (attacking || casting_magic) {
            status = base_direction + "_attack";
        } else if (direction.x == 0 && direction.y == 0) {
            status = base_direction + "_idle";
        } else {
            status = base_direction;
        }
    }




void animate() {
    if (!vulnerable) {
        int alpha = (SDL_GetTicks() / 100) % 2 ? 128 : 255; // blink every ~100ms
        SDL_SetTextureAlphaMod(texture.get(), alpha);
    } else {
        SDL_SetTextureAlphaMod(texture.get(), 255);
    }

    auto& animation = animations[status];
    int anim_size = static_cast<int>(animation.size());
    if (anim_size == 0) return;

    frame_index += animation_speed;

    if (frame_index >= anim_size) {
        frame_index = 0.0f;
    }

    int new_frame = static_cast<int>(frame_index);
    if (new_frame >= anim_size) new_frame = anim_size - 1;

    if (new_frame != current_frame) {
        current_frame = new_frame;
auto& surface = animation[current_frame];

if (!surface) {
    std::cerr << "[ERROR] Null surface in animation[" << status << "] at frame " << current_frame << "\n";
    return;
}

texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()), SDL_DestroyTexture);
if (!texture) {
    std::cerr << "[ERROR] Failed to create texture from surface at frame " << current_frame << ": " << SDL_GetError() << "\n";
    return;
}

        rect.w = surface->w;
        rect.h = surface->h;
        rect.x = hitbox.x + hitbox.w / 2 - rect.w / 2;
        rect.y = hitbox.y + hitbox.h / 2 - rect.h / 2;
    }
}




    void draw(SDL_Renderer* renderer, SDL_Point offset) override {
        SDL_Rect shifted = {
            rect.x - offset.x,
            rect.y - offset.y,
            rect.w,
            rect.h
        };
        SDL_RenderCopy(renderer, texture.get(), nullptr, &shifted);
    }


// Input Handling

    void handleInput() {
        direction = {0, 0};
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        bool spaceDown = keystate[SDL_SCANCODE_SPACE] || keystate[SDL_SCANCODE_TAB];
        bool magicDown = keystate[SDL_SCANCODE_E];

        // Attack logic
        if (spaceDown && !attack_button_held && !attacking && !casting_magic) {
            attacking = true;
            attackTime = SDL_GetTicks();
            if (attack_callback) attack_callback();
        }

        attack_button_held = spaceDown;

        // Magic logic
        if (magicDown && !magic_button_held && !casting_magic && !attacking) {
            casting_magic = true;
            magic_cast_time = SDL_GetTicks();
            if (magic_callback) magic_callback();
        }

        magic_button_held = magicDown;

        if (!attacking && !casting_magic) {
            // Movement
            if (keystate[SDL_SCANCODE_UP]) direction.y = -1;
            else if (keystate[SDL_SCANCODE_DOWN]) direction.y = 1;
            if (keystate[SDL_SCANCODE_LEFT]) direction.x = -1;
            else if (keystate[SDL_SCANCODE_RIGHT]) direction.x = 1;

            if (direction.x < 0)      status = "left";
            else if (direction.x > 0) status = "right";
            else if (direction.y < 0) status = "up";
            else if (direction.y > 0) status = "down";

            if (direction.x != 0 || direction.y != 0) {
                float len = SDL_sqrtf(direction.x * direction.x + direction.y * direction.y);
                normalizedDirection.x = direction.x / len;
                normalizedDirection.y = direction.y / len;
            } else {
                normalizedDirection = {};
            }
        }

        // Weapon swap
        if (keystate[SDL_SCANCODE_Q] && !weapon_swapping) {
            weapon_swapping = true;
            weaponSwapTime = SDL_GetTicks();
            weapon_index = (1 + weapon_index) % weapons.size();
        }

        // Magic swap
        if (keystate[SDL_SCANCODE_W] && !magic_swapping) {
            magic_swapping = true;
            magicSwapTime = SDL_GetTicks();
            magic_index = (1 + magic_index) % magic.size();
        }
    }



    void cooldowns() {
        Uint32 currentTime = SDL_GetTicks();

        if (currentTime - attackTime >= attack_cooldown) {
            attacking = false;
            if (destroy_callback) destroy_callback();
        }

        if (currentTime - magic_cast_time >= magic_cooldown) {
            casting_magic = false;
        }

        if (currentTime - weaponSwapTime >= swap_cooldown) {
            weapon_swapping = false;
        }

        if (currentTime - magicSwapTime >= swap_cooldown) {
            magic_swapping = false;
        }

        if (!vulnerable && currentTime - hurt_time >= invulnerability_duration) {
            vulnerable = true;
        }
    }


    // Main update loop

    void update() override {
        if (!attacking && !casting_magic) {
            move(normalizedDirection.x * speed, 0);
            handleCollision('x');
            move(0, normalizedDirection.y * speed);
            handleCollision('y');

            int insetY = 10;
            rect.x = hitbox.x;
            rect.y = hitbox.y - insetY;
        }

        cooldowns();
        get_status();
        animate();
    }

void takeDamage(int amount) {
    if (vulnerable) {
        stats.health -= amount;
        if (stats.health < 0) {
			stats.health = 0;
			alive = false;
		}
        vulnerable = false;
        hurt_time = SDL_GetTicks();
    }
}


    bool useMana(int amount) {
        if (mana >= amount) {
            mana -= amount;
            return true;
        }
        return false;
    }


    // Getters/Setters

    SDL_Rect getRect() const override { return rect; }
    std::shared_ptr<SDL_Texture> getTexture() const { return texture; }
    SDL_Rect getHitbox() const override { return hitbox; }
    std::string getStatus() const { return status; }
	bool isAlive() const { return alive; }
	SDL_Point getDirection() const { return direction; }

    SDL_Point getCenter() const {
        return {
            rect.x + rect.w / 2,
            rect.y + rect.h / 2
        };
    }


    // Member Variables for State Management

    float speed = 5.0f;
    SDL_Point direction{0, 0};

    bool attacking = false;
    bool attack_button_held = false;
    Uint32 attackTime;
    Uint32 attack_cooldown = 400;

    bool casting_magic = false;
    bool magic_button_held = false;
    Uint32 magic_cast_time;
    Uint32 magic_cooldown = 600;

    bool weapon_swapping = false;
    bool magic_swapping = false;
    Uint32 weaponSwapTime;
    Uint32 magicSwapTime;
    Uint32 swap_cooldown = 400;

    std::string status = "down";
    int current_frame = -1;

    int weapon_index = 0;
    int magic_index = 0;
    std::shared_ptr<Weapon> currentWeapon;

    // Stats

    PlayerStats stats;
    int exp = 123;
    int maximumHealth = 100;
    int maximumMana = 60;

    bool vulnerable = true;
    Uint32 hurt_time = 0;
    Uint32 invulnerability_duration = 500; // ms

    float mana = stats.mana;
	bool alive = true;


private:
    std::shared_ptr<SDL_Texture> texture;
    std::unordered_map<std::string, std::vector<std::shared_ptr<SDL_Surface>>> animations;
    SDL_Renderer* renderer = nullptr;
    SDL_Rect rect;
    std::function<void()> attack_callback;
    std::function<void()> destroy_callback;
    std::function<void()> magic_callback;
};

std::shared_ptr<Player> createPlayer(
    SDL_Renderer* renderer,
    SDL_Point pos,
    std::initializer_list<SpriteGroup*> groups,
    SpriteGroup* obstacles,
    std::function<void()> attack_callback,
    std::function<void()> destroy_callback,
    std::function<void()> magic_callback
) {
    auto player = std::make_shared<Player>(renderer, pos, attack_callback, destroy_callback, magic_callback);
    player->obstacleGroup = obstacles;

    for (auto* group : groups) {
        group->add(player);
    }
    return player;
}
