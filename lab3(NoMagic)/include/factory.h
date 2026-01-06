#ifndef FACTORY_H_
#define FACTORY_H_

#include "hero.h"

#include <memory>
#include <vector>

// Хелпер-фабрика для генерации кандидатов на найм.
class HeroFactory {
  public:
  // Сгенерировать N кандидатов. Каждый вызов - новый набор кандидатoв.
  // Используется unique_ptr, чтобы владение было однозначным.
    static std::vector<std::unique_ptr<Hero>> generateCandidates(size_t count = 4);

  // Утилита для создания конкретного героя (используется внутри фабрики)
    static std::unique_ptr<Hero> createHero(std::string name, Role role, int level);
};

#endif  // FACTORY_H_