#pragma once
#include "sprite.h"
#include "tile.h"
#include "player.h"
#include "settings.h"
#include "support.h"
#include "weapon.h"
#include "enemy.h"
#include "/opt/homebrew/include/SDL2/SDL.h"
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
            { "entities",  import_csv_layout("map/map_Entities.csv") }
        };

        std::unordered_map<std::string, std::vector<SDL_Surface*>> graphics = {
            { "grass", import_folder("graphics/Grass") },
            { "objects", import_folder("graphics/objects") }
        };

        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_int_distribution<> grass_dist(0, graphics["grass"].size() - 1);

        for (const auto& [style, layout] : layouts) {
            for (int i = 0; i < layout.size(); i++) {
                for (int j = 0; j < layout[i].size(); j++) {
                    const std::string cell = trim(layout[i][j]);
                    if (cell == "-1") continue;
                    int x = j * TILESIZE;
                    int y = i * TILESIZE;
                    if (style == "boundary") {
                        createTile(renderer, {x,y}, {&obstacle_sprites}, "invisible");
                    }
                    if (style == "grass") {
                        createTile(renderer, {x, y}, {&visible_sprites, &obstacle_sprites, &attackable_sprites}, "grass", graphics["grass"][grass_dist(gen)]);
                    }
                    if (style == "objects") {
                        int obj_idx = stoi(cell);
                        createTile(renderer, {x,y}, {&obstacle_sprites, &visible_sprites}, "objects", graphics["objects"][(int)obj_idx]);
                    } 
                   if (style == "entities") {
                        int obj_idx = stoi(cell);
                        if (obj_idx == 394) {
                        std::cout << "[DEBUG] Placing Player at (" << x << ", " << y << ")\n";
                        player = createPlayer(renderer, {x, y}, { &visible_sprites }, &obstacle_sprites,
                                            [this]() { this->create_attack(); },
                                            [this]() { this->create_magic(); });
                    } else {
                        std::string type;
                        switch (obj_idx) {
                            case 390: type = "bamboo"; std::cout << "[DEBUG] Placing Bamboo at (" << x << ", " << y << ")\n"; break;
                            case 391: type = "spirit"; std::cout << "[DEBUG] Placing Spirit at (" << x << ", " << y << ")\n"; break;
                            case 392: type = "raccoon"; std::cout << "[DEBUG] Placing Raccoon at (" << x << ", " << y << ")\n"; break;
                            case 393: type = "squid"; std::cout << "[DEBUG] Placing Squid at (" << x << ", " << y << ")\n"; break;
                            default:
                                type = "bamboo";
                                std::cout << "[DEBUG] Placing Default Enemy (Bamboo) for obj_idx=" << obj_idx << " at (" << x << ", " << y << ")\n";
                                break;
                        }

                        auto enemy = createEnemy(renderer, {x, y}, {&visible_sprites, &attackable_sprites}, &obstacle_sprites, type);
                        std::cout << "[DEBUG] Enemy of type '" << type << "' created and added to visible_sprites.\n";
                    }

                    }
                }
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
    std::vector<std::shared_ptr<Sprite>> collided_sprites;

    for (const auto& attack_sprite : attack_sprites.getSprites()) {
        for (const auto& attackable_sprite : attackable_sprites.getSprites()) {
            SDL_Rect attack_hitbox = attack_sprite->getHitbox();
            SDL_Rect target_hitbox = attackable_sprite->getHitbox();

            if (SDL_HasIntersection(&attack_hitbox, &target_hitbox)) {
                auto enemy = std::dynamic_pointer_cast<Enemy>(attackable_sprite);
                if (enemy) {
                    collided_sprites.push_back(enemy);
                    std::cout << "Enemy collided\n";
                } else {
                    visible_sprites.remove(attackable_sprite);
                    attackable_sprites.remove(attackable_sprite);
                    std::cout << "Tile collided\n";
                }
            }
        }
    }

    for (const auto& sprite : collided_sprites) {
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

    attackable_sprites.update();

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
