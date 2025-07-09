#pragma once
#include <vector>
#include <unordered_map>
#include <string>

const int WIDTH = 1280;
const int HEIGHT = 720;
const int FPS = 60;
const int TILESIZE = 64;

struct PlayerStats {
    int health = 100;
    int mana = 60;
    int attack = 10;
    int magic = 4;
    int speed = 5; 
};

std::vector<std::string> weapons = {"sword", "lance", "rapier", "sai"};
std::vector<std::string> magic = {"fire", "heal"};

std::unordered_map<std::string, int> weapon_cooldowns = {
    {"sword", 100},
    {"lance", 400},
    {"rapier", 50},
    {"sai", 80}
};

std::unordered_map<std::string, int> weapon_damage = {
    {"sword", 15},
    {"lance", 30},
    {"rapier", 8},
    {"sai", 10}
};

std::unordered_map<std::string, std::string> weapon_graphics = {
    {"sword", "./graphics/weapons/sword/"},
    {"lance", "./graphics/weapons/lance/"},
    {"rapier", "./graphics/weapons/rapier/"},
    {"sai", "./graphics/weapons/sai/"}
};

std::unordered_map<std::string, int> magic_strength = {
    {"fire", 5},
    {"heal", 20}
};
std::unordered_map<std::string, int> magic_cost = {
    {"fire", 20},
    {"heal", 10}
};

std::unordered_map<std::string, std::string> magic_graphics = {
    {"fire", "./graphics/particles/flame/fire.png"},
    {"heal", "./graphics/particles/heal/heal.png"}
};

struct EnemyStats {
    int health;
    int exp;
    int damage;
    std::string attack_type;
    std::string attack_sound;
    int speed;
    int resistance;
    int attack_radius;
    int notice_radius;
};

std::unordered_map<std::string, EnemyStats> monster_data = {
    { "squid", {
        100, 100, 20, "slash", "./audio/attack/slash.wav", 3, 3, 80, 360
    }},
    { "raccoon", {
        300, 250, 40, "claw", "./audio/attack/claw.wav", 2, 3, 120, 400
    }},
    { "spirit", {
        100, 110, 8, "thunder", "./audio/attack/fireball.wav", 4, 3, 60, 350
    }},
    { "bamboo", {
        70, 120, 6, "leaf_attack", "./audio/attack/slash.wav", 3, 3, 50, 300
    }}
};


// ui
const int BAR_HEIGHT = 20;
const int HEALTH_BAR_WIDTH = 200;
const int MANA_BAR_WIDTH = 140;
const int ITEM_BOX_SIZE = 80;
const std::string UI_FONT = "./graphics/font/joystix.ttf";
const int UI_FONT_SIZE = 18;

// general colors
const std::string WATER_COLOR = "#71ddee";
const std::string UI_BG_COLOR = "#222222";
const std::string UI_BORDER_COLOR = "#111111";
const std::string TEXT_COLOR = "#EEEEEE";

// ui colors
const std::string HEALTH_COLOR = "red";
const std::string MANA_COLOR = "blue";
const std::string UI_BORDER_COLOR_ACTIVE = "gold";
