#include "factory.h"

#include <random>
#include <string>

// Простая фабрика кандидатов: имена + суффикс уникальности.

static std::mt19937& factoryRng() {
  static std::random_device rd;
  static std::mt19937 mt(rd());
  return mt;
}

// Вспомогательная функция для получения случайной роли
static Role randomRole() {
  std::uniform_int_distribution<int> dist(0, 3);
  return static_cast<Role>(dist(factoryRng()));
}

// Создаёт героя с базовыми характеристиками в зависимости от роли и уровня.
// Для простоты maxMp и maxHp подбираются по уровню/роли.
std::unique_ptr<Hero> HeroFactory::createHero(std::string name, Role role, int level) {
  int baseHp = 80 + level * 20;      // преимущественно HP растёт с уровнем
  int baseMp = 20 + level * 15;      // MP зависит от роли, но упрощаем
  int attack = 15 + level * 3;       // Атака зависит от уровня
  int defense = 5 + level * 2;       //Защита зависит от уровня

  // Небольшая корректировка по роли (чтобы классы отличались)
  switch (role) {
    case Role::Warrior:
      baseHp += 20;
      defense += 3;
      break;
    case Role::Mage:
      baseMp += 20;
      attack -= 2;
      break;
    case Role::Archer:
      attack += 2;
      break;
    case Role::Cleric:
      baseMp += 10;
      defense += 1;
      break;
  }

  return std::make_unique<Hero>(std::move(name), role, level, baseHp, baseMp, attack, defense);
}

// Генерация кандидатов: имена + суффиксы #i, уровень случайный от 1 до 5
std::vector<std::unique_ptr<Hero>> HeroFactory::generateCandidates(size_t count) {
  std::vector<std::unique_ptr<Hero>> out;
  static std::vector<std::string> names = {"Arin", "Bela", "Caro", "Dorn", "Ena", "Fynn"};
  std::uniform_int_distribution<int> nameDist(0, static_cast<int>(names.size() - 1));
  std::uniform_int_distribution<int> levelDist(1, 5);

  static int globalHeroId = 1;
  for (size_t i = 0; i < count; ++i) {
    //Само создание героя и его имени
    std::string name = names[nameDist(factoryRng())] + "#" + std::to_string(globalHeroId++);
    Role r = randomRole();
    int lvl = levelDist(factoryRng());
    out.push_back(createHero(std::move(name), r, lvl));
  }
  return out;
}