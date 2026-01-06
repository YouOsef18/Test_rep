#ifndef HERO_H_
#define HERO_H_

#include "creature.h"

#include <string>
#include <vector>

//Роли
enum class Role { Warrior, Mage, Archer, Cleric };

class Enemy;  // Предварительное объявление, чтобы компилятор
//Корректно работал (Enemy определён в enemy.h)

// Класс Hero — наследник Creature
class Hero : public Creature {
  public:
  // Конструктор: name, role, level, maxHp, maxMp, attack, defense
    Hero(std::string name, Role role, int level, int maxHp, int maxMp,
         int attack, int defense);

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

  // Специальные методы героя
    Role role() const;
    int level() const;

  // MP: текущая и максимальная манна
    int mp() const;
    int maxMp() const;
    void restoreMp(int amount);
    bool useMp(int amount);

  // Боевые действия
    void attackTarget(Enemy* enemy);
  //Использование способности 
    bool useAbility(std::vector<Enemy*>& enemies, std::vector<Hero*>& allies);

  // Восстановление HP
    void heal(int amount);

  // Изменение временного множителя защиты (публичный интерфейс)
    void multiplyDefenseMultiplier(double mul);

  private:
    std::string name_;
    Role role_;
    int level_;
    int hp_;
    int maxHp_;
    int mp_;
    int maxMp_;
    int attack_;
    int defense_;

  // Временный множитель защиты (сбрасывается каждый раунд)
    double defenseMultiplier_;
};

#endif  // HERO_H_