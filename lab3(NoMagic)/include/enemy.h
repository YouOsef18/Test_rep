#ifndef ENEMY_H_
#define ENEMY_H_

#include "creature.h"

#include <string>

// Простой класс Enemy — наследник Creature.
class Enemy : public Creature {
 public:
    Enemy(std::string name, int hp, int attack, int defense);
// Реализация интерфейса Creature
  //const — гарантирует, что метод не меняет объект
  //override — гарантирует, что метод переопределяет виртуальный метод базового класса
    const std::string& name() const override;
    int hp() const override;
    int maxHp() const override;
    bool isAlive() const override;
    int attack() const override;
    int defense() const override;
    void takeDamage(int amount) override;
    void resetTemporaryEffects() override;

  private:
    std::string name_;
    int hp_;
    int maxHp_;
    int attack_;
    int defense_;
};

#endif  // ENEMY_H_