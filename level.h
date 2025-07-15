#pragma once
#include "sprite.h"
#include "tile.h"
#include "player.h"
#include "settings.h"
#include "support.h"
#include "weapon.h"
#include "enemy.h"
#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <random>
class Level {
public:

    Level(SDL_Renderer* renderer) : renderer(renderer) {
        create_map();
    };

    void create_map() {
    std::unordered_map<std::string, std::vector<std::vector<std::string>>> layouts = {
        { "boundary", import_csv_layout("map/map_FloorBlocks.csv") },
        { "grass",    import_csv_layout("map/map_Grass.csv") },
        { "objects",  import_csv_layout("map/map_Objects.csv") },
        { "entities", import_csv_layout("map/map_Entities.csv") }
    };

    std::unordered_map<std::string, std::vector<SDL_Surface*>> graphics = {
        { "grass", import_folder("graphics/Grass") },
        { "objects", import_folder("graphics/objects") }
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> grass_dist(0, graphics["grass"].size() - 1);

    // Pass 1: Place all static tiles and the player
    for (const auto& [style, layout] : layouts) {
        for (int i = 0; i < layout.size(); i++) {
            for (int j = 0; j < layout[i].size(); j++) {
                const std::string cell = trim(layout[i][j]);
                if (cell == "-1") continue;

                int x = j * TILESIZE;
                int y = i * TILESIZE;

                if (style == "boundary") {
                    createTile(renderer, {x, y}, {&obstacle_sprites}, "invisible");
                } else if (style == "grass") {
                    createTile(renderer, {x, y}, {&visible_sprites, &obstacle_sprites, &attackable_sprites},
                               "grass", graphics["grass"][grass_dist(gen)]);
                } else if (style == "objects") {
                    int obj_idx = stoi(cell);
                    createTile(renderer, {x, y}, {&obstacle_sprites, &visible_sprites},
                               "objects", graphics["objects"][obj_idx]);
                } else if (style == "entities") {
                    int obj_idx = stoi(cell);
                    if (obj_idx == 394) {
						std::cout << "new player created" << std::endl;
                        player = createPlayer(renderer, {x, y}, {&visible_sprites}, &obstacle_sprites,
                                              [this]() { this->create_attack(); },
                                              nullptr,
                                              [this]() { this->create_magic(); });
                    }
                }
            }
        }
    }
    const auto& entity_layout = layouts["entities"];
    for (int i = 0; i < entity_layout.size(); i++) {
        for (int j = 0; j < entity_layout[i].size(); j++) {
            const std::string cell = trim(entity_layout[i][j]);
            if (cell == "-1") continue;

            int obj_idx = stoi(cell);
            if (obj_idx == 394) continue;  // already handled

            int x = j * TILESIZE;
            int y = i * TILESIZE;

            std::string type;
            switch (obj_idx) {
                case 390: type = "bamboo"; break;
                case 391: type = "spirit"; break;
                case 392: type = "raccoon"; break;
                case 393: type = "squid";  break;
                default:
                    type = "bamboo";
                    break;
            }
            auto enemy = createEnemy(
                renderer,
                {x, y},
                {&visible_sprites, &attackable_sprites},
                &obstacle_sprites,
                [this](int damage) {
                    std::cout << "[lambda] Called with damage: " << damage << "\n";
    				if (player) {
        				std::cout << "[lambda] Player address: " << player.get() << "\n";
        				player->takeDamage(damage);
    				}
                },
                type
            );
        }
    }
}
    void create_attack() {
        if (player->currentWeapon) {
            visible_sprites.remove(player->currentWeapon);
            attack_sprites.remove(player->currentWeapon);
            player->currentWeapon.reset();
        }

        player->currentWeapon = createWeapon(
            renderer,
            player,
            {&visible_sprites, &attack_sprites},
            weapon_graphics[weapons[player->weapon_index]]
        );
    }
    void create_magic() {
        std::cout << "create_magic successfully called." << std::endl;
    }

void player_attack_logic() {
    if (!player->attacking || !player->currentWeapon) return;

    SDL_Rect weapon_rect = player->currentWeapon->getHitbox();

    for (const auto& sprite : attackable_sprites.getSprites()) {
        auto enemy = std::dynamic_pointer_cast<Enemy>(sprite);
        if (!enemy || !enemy->isAlive() || !enemy->isVulnerable()) continue;

        SDL_Rect enemy_hitbox = enemy->getHitbox();

        if (SDL_HasIntersection(&weapon_rect, &enemy_hitbox)) {
            enemy->takeDamage(player->stats.attack);

            // Knockback direction
SDL_FPoint dir = enemy->getPlayerDistanceAndDirection(player->getCenter()).second;
dir.x *= -1;
dir.y *= -1;
enemy->applyKnockback(dir);


            std::cout << "[Weapon Hit] " << enemy->getType()
                      << " took " << player->stats.attack << " damage and was knocked back.\n";
        }
    }
}



void enemy_attack_logic() {
    SDL_Rect player_hitbox = player->getHitbox();

    for (const auto& sprite : attackable_sprites.getSprites()) {
        auto enemy = std::dynamic_pointer_cast<Enemy>(sprite);
        if (enemy && enemy->isAttacking()) {
            SDL_Rect enemy_hitbox = enemy->getHitbox();

            if (SDL_HasIntersection(&player_hitbox, &enemy_hitbox)) {
                enemy->triggerAttack();  // this should call the callback which runs player->takeDamage
            }
        }
    }
}


void update() {
    if (player->currentWeapon && !player->attacking) {
        visible_sprites.remove(player->currentWeapon);
        attack_sprites.remove(player->currentWeapon);
        player->currentWeapon.reset();
    }

    obstacle_sprites.update();
    visible_sprites.update();
    attack_sprites.update();

    player_attack_logic();
	enemy_attack_logic();
    attackable_sprites.update();
	for (auto& group : {&attackable_sprites, &visible_sprites}) {
    for (auto sprite : group->getSprites()) {
        auto enemy = std::dynamic_pointer_cast<Enemy>(sprite);
        if (enemy && !enemy->isAlive()) {
            group->remove(sprite);
        }
    }
}

    SDL_Point player_center = player->getCenter();

    for (const auto& sprite : visible_sprites.getSprites()) {
        auto enemy = std::dynamic_pointer_cast<Enemy>(sprite);
        if (enemy) {
            enemy->update(player_center);
        }
    }
}


    void render(SDL_Point offset) {
        for (const auto& sprite : visible_sprites.getSprites()) {
            sprite->draw(renderer, offset);
        }
    }

    // Getters and Setters
    const SpriteGroup& getVisibleSprites() const { return visible_sprites; }
    SpriteGroup* getVisibleSprites() { return &visible_sprites; }
    const SpriteGroup& getObstacleSprites() const { return obstacle_sprites; }
    std::shared_ptr<Player> getPlayer() const { return player; }

private:
    SDL_Renderer* renderer;
    SpriteGroup visible_sprites;
    SpriteGroup obstacle_sprites;
    SpriteGroup attackable_sprites;
    SpriteGroup attack_sprites;

    std::shared_ptr<Player> player;
};
