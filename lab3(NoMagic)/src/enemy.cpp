#include "enemy.h"

#include <string>

// Конструктор + семантика перемещения с move
Enemy::Enemy(std::string name, int hp, int attack, int defense)
    : name_(std::move(name)), hp_(hp), maxHp_(hp), attack_(attack), defense_(defense) {}

//Сделал ссылку только для name, т.к строка тяжелее int и bool
const std::string& Enemy::name() const { return name_; }
int Enemy::hp() const { return hp_; }
int Enemy::maxHp() const { return maxHp_; }
bool Enemy::isAlive() const { return hp_ > 0; }
int Enemy::attack() const { return attack_; }
int Enemy::defense() const { return defense_; }

void Enemy::takeDamage(int amount) {
  hp_ -= amount;
  if (hp_ < 0) hp_ = 0;
}

void Enemy::resetTemporaryEffects() {
  // В данном простом классе нет временных эффектов.
}