#pragma once
#include "entity.h"
#include "settings.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

class Enemy : public Entity {
public:
    Enemy(SDL_Renderer* renderer,
        SDL_Point pos,
        std::function<void(int)> damage_player_callback,
        const std::string& enemy_type = "")

        : renderer(renderer), enemy_type(enemy_type), stats(monster_data.at(enemy_type)), damage_player_callback(damage_player_callback) {

        auto it = monster_data.find(enemy_type);
        if (it == monster_data.end()) {
            std::cerr << "Error, Unknown enemy type: " << enemy_type << std::endl;
            exit(1);
        }

        stats = it->second;
        status = "idle";
        frame_index = 0.0f;
        animation_speed = 0.15f;
        current_frame = -1;
        import_enemy_assets();
		bool has_any_animation = false;
		for (const auto& [_, frames] : animations) {
  	  		if (!frames.empty()) {
   	   	  		has_any_animation = true;
  		      break;
    		}
		}



	if (!has_any_animation) {
    	std::cerr << "enemy: '" << enemy_type << "' has no valid animation frames at all!\n";
    	throw std::runtime_error("no animation frames for enemy: " + enemy_type);
	}

        if (!animations[status].empty()) {
            SDL_Surface* firstFrame = animations[status][0].get();
            rect = { pos.x, pos.y, firstFrame->w, firstFrame->h };
            int insetY = 10;
            hitbox = {
                pos.x,
                pos.y + insetY,
                firstFrame->w,
                firstFrame->h - 2 * insetY
            };
        } else std::cerr << "no animation frames found for status '" << status << "' and enemy type '" << enemy_type << "'\n";

    }

    void import_enemy_assets() {
        std::string basePath = "./graphics/monsters/" + enemy_type + "/";
        for (auto& [k, v] : animations = {
                { "idle", {} }, { "move", {} }, { "attack", {} }
            }) {
            std::string completePath = basePath + k;
            std::vector<std::filesystem::directory_entry> entries;
	if (!std::filesystem::exists(completePath)) {
    	std::cerr << "[ERROR] Directory does not exist: " << completePath << std::endl;
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
                    std::cerr << "Failed to load enemy image: " << full_path << " | " << IMG_GetError() << std::endl;
                    continue;
                }

                surface_list.push_back(std::move(image));
            }

            v = std::move(surface_list);
        }
    }

    std::pair<float, SDL_FPoint> getPlayerDistanceAndDirection(SDL_Point player_center) const {
        SDL_FPoint enemy_center = {
            rect.x + rect.w / 2.0f,
            rect.y + rect.h / 2.0f
        };

        SDL_FPoint diff = {
            static_cast<float>(player_center.x) - enemy_center.x,
            static_cast<float>(player_center.y) - enemy_center.y
        };

        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        SDL_FPoint direction = {0.0f, 0.0f};
        if (distance > 0.0f) {
            direction.x = diff.x / distance;
            direction.y = diff.y / distance;
        }

        return { distance, direction };
    }

    void update_status(float distance) {
        if (distance <= stats.attack_radius) {
            if (can_attack && !attacking) {
                status = "attack";
                attacking = true;
                frame_index = 0.0f;
            }
        } else if (distance <= stats.notice_radius) {
            status = "move";
        } else {
            status = "idle";
        }
    }

    void update() override {
        if (!vulnerable && SDL_GetTicks() - last_attacked_time >= invuln_cooldown)
            vulnerable = true;
    }


    void attack() {
        std::cout << "in enemy::attack: " << enemy_type << " attacked with " << stats.attack_type << "!\n";
        // damage player
		 triggerAttack();
    }

void update(SDL_Point player_center) {
	if (!alive) return;
    auto [distance, direction] = getPlayerDistanceAndDirection(player_center);
    Uint32 now = SDL_GetTicks();
    can_attack = (now - last_attack_time >= attack_cooldown);

    update_status(distance);

    if (status == "move") {
        move_toward_player(direction);
    }

    animate();  // attack() + triggerAttack() gets called here
}


void animate() {
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
            std::cerr << "within enemy::animate, null surface at frame " << current_frame << " for " << status << "\n";
            return;
        }

        texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()), SDL_DestroyTexture);
        if (!texture) {
            std::cerr << "within enemy::animate, failed to create texture from frame.\n";
            return;
        }

        rect.w = surface->w;
        rect.h = surface->h;
        rect.x = hitbox.x + hitbox.w / 2 - rect.w / 2;
        rect.y = hitbox.y + hitbox.h / 2 - rect.h / 2;
    }

    if (status == "attack" && current_frame == anim_size - 1) {
        if (attacking) {
            attack();  // call once at end
            attacking = false;
            can_attack = false;
            last_attack_time = SDL_GetTicks();
        }
        status = "idle";
        frame_index = 0.0f;
    }
}

void applyKnockback(SDL_FPoint source_dir, int force = 100) {
    float dx = source_dir.x * force;
    float dy = source_dir.y * force;

    hitbox.x += static_cast<int>(dx);
    hitbox.y += static_cast<int>(dy);

    rect.x = hitbox.x + hitbox.w / 2 - rect.w / 2;
    rect.y = hitbox.y + hitbox.h / 2 - rect.h / 2;
}





    void draw(SDL_Renderer* renderer, SDL_Point offset) override {
        SDL_Rect shifted = {
            rect.x - offset.x,
            rect.y - offset.y,
            rect.w,
            rect.h
        };

        if (texture) {
            SDL_RenderCopy(renderer, texture.get(), nullptr, &shifted);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &shifted);
            std::cerr << "no texture detected, rendering fallback black rect for: " << enemy_type << "\n";
        }
    }

    void move_toward_player(const SDL_FPoint& direction) {
        SDL_FPoint delta = {
            direction.x * static_cast<float>(stats.speed),
            direction.y * static_cast<float>(stats.speed)
        };

        hitbox.x += static_cast<int>(delta.x);
        hitbox.y += static_cast<int>(delta.y);

        rect.x = hitbox.x + hitbox.w / 2 - rect.w / 2;
        rect.y = hitbox.y + hitbox.h / 2 - rect.h / 2;

    }

    SDL_Rect getRect() const override { return rect; }
    SDL_Rect getHitbox() const override { return hitbox; }
    std::string getType() const { return enemy_type; }
    std::shared_ptr<SDL_Texture> getTexture() const { return texture; }

    int getHealth() const { return stats.health; }
    int getEXP() const { return stats.exp; }
    int getAttackDamage() const { return stats.attack_damage; }
	bool isAlive() const { return alive; }
	bool isAttacking() const { return attacking; }
	bool isVulnerable() const { return vulnerable; }
    SDL_Point getCenter() const {
        return {
            rect.x + rect.w / 2,
            rect.y + rect.h / 2
        };
    }

void takeDamage(int amount) {
    if (!vulnerable) return;

    stats.health -= amount;
    if (stats.health <= 0) {
        stats.health = 0;
        alive = false;
        std::cout << enemy_type << " has died.\n";
    }

    vulnerable = false;
    last_attacked_time = SDL_GetTicks();
}


void triggerAttack() {
    if (damage_player_callback && can_attack) {
        damage_player_callback(stats.attack_damage);
        can_attack = false;
        last_attack_time = SDL_GetTicks();
    }
}



private:
    SDL_Renderer* renderer = nullptr;
    std::string enemy_type;
    std::string status;

    SDL_Rect rect;
    SDL_Rect hitbox;
    std::shared_ptr<SDL_Texture> texture;

    std::unordered_map<std::string, std::vector<std::shared_ptr<SDL_Surface>>> animations;
    float frame_index;
    float animation_speed;
    int current_frame;

    Uint32 last_attack_time = 0;
    Uint32 attack_cooldown = 600;
    bool can_attack = true;
    bool attacking = false;

    Uint32 last_attacked_time = 0;
    Uint32 invuln_cooldown = 600;
    bool vulnerable = true;
	bool alive = true;

    EnemyStats stats;
	std::function<void(int)> damage_player_callback;
};

std::shared_ptr<Enemy> createEnemy(
    SDL_Renderer* renderer,
    SDL_Point pos,
    std::initializer_list<SpriteGroup*> groups,
    SpriteGroup* obstacles,
    std::function<void(int)> damage_player_callback,
    const std::string& enemy_type)
{

    if (!obstacles) {
        std::cerr << "fatal: obstacleGroup is null!\n";
        throw std::runtime_error("null obstacle group");
    }

    for (auto* group : groups) {
        if (!group) {
            std::cerr << "one of the SpriteGroups is null!\n";
            throw std::runtime_error("null SpriteGroup in initializer_list");
        }
    }

    auto enemy = std::make_shared<Enemy>(renderer, pos, damage_player_callback, enemy_type);
    enemy->obstacleGroup = obstacles;

    for (auto* group : groups) {
        group->add(std::static_pointer_cast<Sprite>(enemy));
    }

    return enemy;
}

