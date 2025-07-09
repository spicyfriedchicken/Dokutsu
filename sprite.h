#pragma once
#include "/opt/homebrew/include/SDL2/SDL.h"
#include <vector>
#include <memory>

class Sprite {
public:
    virtual void update() = 0;
    virtual void draw(SDL_Renderer* renderer, SDL_Point offset) = 0;
    virtual SDL_Rect getRect() const = 0;
    virtual SDL_Rect getHitbox() const = 0;
    virtual std::string getType() { return "generic"; }
    virtual ~Sprite() = default;
};

class SpriteGroup {
public:    
    void add(std::shared_ptr<Sprite> sprite) {
        sprites.push_back(sprite);
    }

    void remove(std::shared_ptr<Sprite> sprite) {
        sprites.erase(std::remove_if(sprites.begin(), sprites.end(),
            [&sprite](const std::shared_ptr<Sprite>& s) {
                return s.get() == sprite.get();
            }),
            sprites.end());
    }


    void update() {
        for (auto& sprite : sprites) {
            sprite -> update();
        }
    }

    void draw(SDL_Renderer* renderer, SDL_Point offset) {
        for (auto& sprite : sprites) {
            sprite->draw(renderer, offset);
        }
    }

    const std::vector<std::shared_ptr<Sprite>>& getSprites() const {
        return sprites;
    }

private:
    std::vector<std::shared_ptr<Sprite>> sprites;
};

