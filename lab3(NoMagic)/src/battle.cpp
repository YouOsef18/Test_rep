#include "battle.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <string>

// Локальный генератор для боя
static std::mt19937& battleRng() {
  static std::random_device rd;
  static std::mt19937 mt(rd());
  return mt;
}

// Утилита: Role -> string. Превращает Role::Warrior в строку "Warrior".
static const char* roleToString(Role r) {
  switch (r) {
    case Role::Warrior: return "Warrior";
    case Role::Mage: return "Mage";
    case Role::Archer: return "Archer";
    case Role::Cleric: return "Cleric";
    default: return "Unknown";
  }
}

//Конструктор + семантика перемещения
Battle::Battle(std::vector<Hero*> heroes, std::vector<Enemy*> enemies)
    : heroes_(std::move(heroes)), enemies_(std::move(enemies)) {}

// Правило очередности: идём по индексам и чередуем, как было до этого — простая детерминированная очередь
std::vector<Creature*> Battle::calculateTurnOrder() {
  std::vector<Creature*> order;
  //size_t — это тип для размеров
  size_t maxIndex = std::max(heroes_.size(), enemies_.size());
  for (size_t i = 0; i < maxIndex; ++i) {
    //Заполяем очередь боя. Герои чередуются с монстрами. Оставшиеся в конце
    if (i < heroes_.size() && heroes_[i]->isAlive()) order.push_back(heroes_[i]);
    if (i < enemies_.size() && enemies_[i]->isAlive()) order.push_back(enemies_[i]);
  }
  return order;
}

//Проверяем есть ли живой герой
bool Battle::allHeroesDead() const {
  for (auto* h : heroes_) {
    if (h && h->isAlive()) return false;
  }
  return true;
}

//Проверяем, есть ли живой моб
bool Battle::allEnemiesDead() const {
  for (auto* e : enemies_) {
    if (e && e->isAlive()) return false;
  }
  return true;
}

//Обработка хода героя
void Battle::handleHeroTurn(Hero* hero) {
  if (!hero || !hero->isAlive()) return;
  std::cout << "\nХод героя: " << hero->name() << " [" << roleToString(hero->role()) << "]"
            << " (HP " << hero->hp() << "/" << hero->maxHp() << ", MP " << hero->mp() << "/" << hero->maxMp()
            << ")\n";

  // Сбор живых врагов и списка союзников
  std::vector<Enemy*> aliveEnemies;
  for (auto* e : enemies_) if (e && e->isAlive()) aliveEnemies.push_back(e);
  std::vector<Hero*> allies;
  for (auto* h : heroes_) if (h) allies.push_back(h);

  //Герой выбирает, как действовать
  int choice = 0;
  std::cout << "Выберите действие: 1) Атака  2) Способность  3) Защита\n> ";
  std::cin >> choice;
  if (choice == 1) {
    if (aliveEnemies.empty()) return;
    std::cout << "Выберите цель:\n";
    //Выводим доступных для атаки мобой
    for (size_t i = 0; i < aliveEnemies.size(); ++i) {
      std::cout << i + 1 << ") " << aliveEnemies[i]->name() << " (HP " << aliveEnemies[i]->hp() << ")\n";
    }
    int t;
    std::cin >> t;
    //std::clamp — это "ограничитель значений". 
    //Он говорит: "если число вышло за пределы — верни ближайшую границу".
    size_t idx = std::clamp(t - 1, 0, static_cast<int>(aliveEnemies.size() - 1));
    hero->attackTarget(aliveEnemies[idx]);
  } else if (choice == 2) {
    bool ok = hero->useAbility(aliveEnemies, allies);
    if (!ok) {
      std::cout << "Не удалось использовать способность. Потерян ход.\n";
    }
  } else {
    hero->multiplyDefenseMultiplier(1.5);
    std::cout << hero->name() << " защищается (защита +50% на раунд).\n";
  }
}

void Battle::handleEnemyTurn(Enemy* enemy) {
  if (!enemy || !enemy->isAlive()) return;
  // Выбирает случайного живого героя
  std::vector<Hero*> alive;
  for (auto* h : heroes_) if (h && h->isAlive()) alive.push_back(h);
  if (alive.empty()) return;

  //Создает "генератор случайных целых чисел" в заданном диапазоне.
  //battleRng() — дает генератор случайных чисел (как источник случайности)
  //dist(...) — использует генератор чтобы выдать число по правилам
  std::uniform_int_distribution<size_t> dist(0, alive.size() - 1);
  Hero* target = alive[dist(battleRng())];
  int dmg = std::max(1, enemy->attack() - target->defense());
  target->takeDamage(dmg);
  std::cout << enemy->name() << " атакует " << target->name() << " на " << dmg << " урона.\n";
  if (!target->isAlive()) {
    std::cout << target->name() << " пал!\n";
  }
}

void Battle::resetTemporaryEffects() {
  for (auto* h : heroes_) if (h) h->resetTemporaryEffects();
  for (auto* e : enemies_) if (e) e->resetTemporaryEffects();
}


//Основной метод боя
bool Battle::executeBattle() {
  std::cout << "Бой начинается! Герои: ";
  //Выводим участников битвы
  for (auto* h : heroes_) if (h && h->isAlive()) std::cout << h->name() << " [" << roleToString(h->role()) << "] ";
  std::cout << "VS Враги: ";
  for (auto* e : enemies_) if (e && e->isAlive()) std::cout << e->name() << " ";
  std::cout << "\n";
  
  while (true) {
    //Просчитываем очередность ходов
    auto order = calculateTurnOrder();
    for (auto* c : order) {
      if (!c || !c->isAlive()) continue;
      //dynamic_cast — это "безопасный преобразователь типов с проверкой. Иначе выведет nullprt
      //Обработка ходов мобов и героев
      if (auto* h = dynamic_cast<Hero*>(c)) handleHeroTurn(h);
      else if (auto* e = dynamic_cast<Enemy*>(c)) handleEnemyTurn(e);

      if (allEnemiesDead()) {
        std::cout << "Все враги повержены!\n";
        return true;
      }
      if (allHeroesDead()) {
        std::cout << "Все герои пали...\n";
        return false;
      }
    }
    //Обнуление эффектов
    resetTemporaryEffects();
  }
}