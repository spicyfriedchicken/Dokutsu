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

enum class PlayerActionState {
	Idle,
	Moving,
	Attacking,
	Casting
};

enum class Direction {
	Up,
	Down,
	Left,
	Right
};

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
void updateAnimationStatus() {
    std::string dirStr;
    switch (facingDirection) {
        case Direction::Up:    dirStr = "up"; break;
        case Direction::Down:  dirStr = "down"; break;
        case Direction::Left:  dirStr = "left"; break;
        case Direction::Right: dirStr = "right"; break;
    }

    std::string newStatus;
    switch (actionState) {
        case PlayerActionState::Idle:
            newStatus = dirStr + "_idle";
            break;
        case PlayerActionState::Moving:
            newStatus = dirStr;
            break;
        case PlayerActionState::Attacking:
            newStatus = dirStr + "_attack";
            break;
        case PlayerActionState::Casting:
            newStatus = dirStr + "_attack"; // same for now
            break;
    }

    if (newStatus != status) {
        status = newStatus;
        frame_index = 0.0f;
        current_frame = -1;
    }
}




void animate() {
    if (!vulnerable) {
        int alpha = (SDL_GetTicks() / 100) % 2 ? 128 : 255;
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
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    bool spaceDown = keystate[SDL_SCANCODE_SPACE];
    bool magicDown = keystate[SDL_SCANCODE_E];

    direction = {0, 0};
    SDL_Point rawDir = {0, 0};

    if (keystate[SDL_SCANCODE_UP])    rawDir.y = -1;
    if (keystate[SDL_SCANCODE_DOWN])  rawDir.y =  1;
    if (keystate[SDL_SCANCODE_LEFT])  rawDir.x = -1;
    if (keystate[SDL_SCANCODE_RIGHT]) rawDir.x =  1;

    // Normalize movement direction and update facing
    if (rawDir.x != 0 || rawDir.y != 0) {
        updateFacingDirection(rawDir);
        if (!attacking && !casting) {
            float len = SDL_sqrtf(rawDir.x * rawDir.x + rawDir.y * rawDir.y);
            direction = { static_cast<float>(rawDir.x), static_cast<float>(rawDir.y) };

if (len > 0.0f) {
    normalizedDirection = {
        direction.x / len,
        direction.y / len
    };
} else {
    normalizedDirection = {0.0f, 0.0f};
}

            actionState = PlayerActionState::Moving;
        }
    } else if (!attacking && !casting) {
        direction = {0, 0};
        actionState = PlayerActionState::Idle;
    }

    // Handle attack state
    if (spaceDown && !attack_button_held && !attacking && !casting) {
        attacking = true;
        attackTime = SDL_GetTicks();
        actionState = PlayerActionState::Attacking;
        if (attack_callback) attack_callback();
    }

    attack_button_held = spaceDown;

    // Handle magic state
    if (magicDown && !magic_button_held && !casting && !attacking) {
        casting = true;
        magicCastTime = SDL_GetTicks();
        actionState = PlayerActionState::Casting;
        if (magic_callback) magic_callback();
    }

    magic_button_held = magicDown;

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

    if (attacking && currentTime - attackTime >= attack_cooldown) {
        attacking = false;
        if (actionState == PlayerActionState::Attacking)
            actionState = PlayerActionState::Idle;

        if (destroy_callback) destroy_callback();
    }

    if (casting && currentTime - magicCastTime >= magic_cooldown) {
        casting = false;
        if (actionState == PlayerActionState::Casting)
            actionState = PlayerActionState::Idle;
    }

    if (weapon_swapping && currentTime - weaponSwapTime >= swap_cooldown) {
        weapon_swapping = false;
    }

    if (magic_swapping && currentTime - magicSwapTime >= swap_cooldown) {
        magic_swapping = false;
    }

    if (!vulnerable && currentTime - hurt_time >= invulnerability_duration) {
        vulnerable = true;
    }
}



void updateFacingDirection(const SDL_Point& dir) {
	if (attacking) return;
    if (dir.x > 0)      facingDirection = Direction::Right;
    else if (dir.x < 0) facingDirection = Direction::Left;
    else if (dir.y > 0) facingDirection = Direction::Down;
    else if (dir.y < 0) facingDirection = Direction::Up;
}

void update() override {
    if (!attacking && !casting) {
        move(normalizedDirection.x * speed, 0);
        handleCollision('x');
        move(0, normalizedDirection.y * speed);
        handleCollision('y');

        int insetY = 10;
        rect.x = hitbox.x;
        rect.y = hitbox.y - insetY;
    }

    cooldowns();
    updateAnimationStatus();
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
        if (stats.mana >= amount) {
            stats.mana -= amount;
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
	SDL_FPoint getDirection() const { return direction; }

    SDL_Point getCenter() const {
        return {
            rect.x + rect.w / 2,
            rect.y + rect.h / 2
        };
    }


    // Member Variables for State Management


    float speed = 5.0f;
    SDL_FPoint direction{0, 0};
	SDL_FPoint normalizedDirection = {0, 0};
	std::string status = "down";

    bool attacking = false;
    bool attack_button_held = false;
    Uint32 attackTime;
    Uint32 attack_cooldown = 400;

    bool casting = false;
    bool magic_button_held = false;
    Uint32 magicCastTime;;
    Uint32 magic_cooldown = 400;


    bool weapon_swapping = false;
    bool magic_swapping = false;
	Uint32 swap_cooldown = 400;
    Uint32 weaponSwapTime;
    Uint32 magicSwapTime;

	PlayerActionState actionState = PlayerActionState::Idle;
	Direction facingDirection = Direction::Down;

	int current_frame = -1;
    int weapon_index = 0;
    int magic_index = 0;

    std::shared_ptr<Weapon> currentWeapon;
    PlayerStats stats;

    int exp = 0;
    int maximumHealth = 100;
    int maximumMana = 60;

    bool vulnerable = true;
    Uint32 hurt_time = 0;
    Uint32 invulnerability_duration = 400;

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
