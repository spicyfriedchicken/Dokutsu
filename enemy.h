#pragma once
#include "entity.h"
#include "settings.h"
#include "/opt/homebrew/include/SDL2/SDL.h"
#include "/opt/homebrew/include/SDL2/SDL_image.h"
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
        const std::string& enemy_type = "")

        : renderer(renderer), enemy_type(enemy_type), stats(monster_data.at(enemy_type)) {

        status = "idle";
        frame_index = 0.0f;
        animation_speed = 0.15f;
        current_frame = -1;
        import_enemy_assets();

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
        }

    }

    void import_enemy_assets() {
        std::string basePath = "./graphics/monsters/" + enemy_type + "/";
        for (auto& [k, v] : animations = {
                { "idle", {} }, { "move", {} }, { "attack", {} }
            }) {
            std::string completePath = basePath + k;
            std::vector<std::filesystem::directory_entry> entries;

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
            if (can_attack && !is_attacking) {
                status = "attack";
                is_attacking = true;
                frame_index = 0.0f;
            }
        } else if (distance <= stats.notice_radius) {
            status = "move";
        } else {
            status = "idle";
        }
    }

    void update() override {
    }


    void attack() {
        std::cout << "[Enemy::attack] " << enemy_type << " attacked with " << stats.attack_type << "!\n";
        // TODO: damage player, play sound, etc.
    }

    void update(SDL_Point player_center) {
        auto [distance, direction] = getPlayerDistanceAndDirection(player_center);
        Uint32 now = SDL_GetTicks();
        can_attack = (now - last_attack_time >= attack_cooldown);

        update_status(distance);

        if (status == "attack" && is_attacking && can_attack) {
            attack();
            last_attack_time = now;
            can_attack = false;
        } else if (status == "move") {
            move_toward_player(direction);
        }

        animate();
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

            texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()), SDL_DestroyTexture);
            if (!texture) {
                std::cerr << "[Enemy::animate] Failed to create texture from frame.\n";
                return;
            }

            rect.w = surface->w;
            rect.h = surface->h;
            rect.x = hitbox.x + hitbox.w / 2 - rect.w / 2;
            rect.y = hitbox.y + hitbox.h / 2 - rect.h / 2;
        }

        if (status == "attack" && current_frame == anim_size - 1 && is_attacking) {
            status = "idle";
            frame_index = 0.0f;
            is_attacking = false;
        }
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
            std::cerr << "[Enemy::draw] No texture, rendering fallback black rect for: " << enemy_type << "\n";
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
    
    void update_health() {
        
    }

    SDL_Rect getRect() const override { return rect; }
    SDL_Rect getHitbox() const override { return hitbox; }
    std::string getType() const { return enemy_type; }
    std::shared_ptr<SDL_Texture> getTexture() const { return texture; }

    SDL_Point getCenter() const {
        return {
            rect.x + rect.w / 2,
            rect.y + rect.h / 2
        };
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
    bool is_attacking = false;

    Uint32 last_attacked_time = 0;
    Uint32 invuln_cooldown = 600;
    bool is_vulnerable = true;

    EnemyStats stats;
};

std::shared_ptr<Enemy> createEnemy(
    SDL_Renderer* renderer,
    SDL_Point pos,
    std::initializer_list<SpriteGroup*> groups,
    SpriteGroup* obstacles,
    const std::string& enemy_type = "bamboo")
 {

    auto enemy = std::make_shared<Enemy>(renderer, pos, enemy_type);
    enemy->obstacleGroup = obstacles;

    for (auto* group : groups) {
        group->add(std::static_pointer_cast<Sprite>(enemy));
    }

    return enemy;
}