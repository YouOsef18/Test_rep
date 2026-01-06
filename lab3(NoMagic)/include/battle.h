#ifndef BATTLE_H_
#define BATTLE_H_

#include "hero.h"
#include "enemy.h"

#include <vector>

// Класс Battle проводит пошаговую логику боя между векторами сырых указателей
// (Hero* и Enemy*). Он НЕ владеет этими объектами — владение остаётся у вызывающего.
class Battle {
  public:
    Battle(std::vector<Hero*> heroes, std::vector<Enemy*> enemies);

  // Запустить бой; возвращает true если победили герои, false — если все герои погибли.
    bool executeBattle();

  private:
  //Вступающие в бой
    std::vector<Hero*> heroes_;
    std::vector<Enemy*> enemies_;

  // Вспомогательные методы
    std::vector<Creature*> calculateTurnOrder();
    bool allHeroesDead() const;
    bool allEnemiesDead() const;
    void handleHeroTurn(Hero* hero);
    void handleEnemyTurn(Enemy* enemy);
    void resetTemporaryEffects();
};

#endif  // BATTLE_H_