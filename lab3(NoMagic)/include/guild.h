#ifndef GUILD_H_
#define GUILD_H_

#include "hero.h"

#include <memory>
#include <vector>
#include <mutex>

// Singleton — управление золотом, отрядом, наймом/увольнением.
class Guild {
 public:
  // Получить singleton-инстанс (Meyers singleton, потокобезопасен в C++11+)
  static Guild& instance() noexcept;

  // Удаляем копирование/перемещение
  Guild(const Guild&) = delete;
  Guild& operator=(const Guild&) = delete;
  Guild(Guild&&) = delete;
  Guild& operator=(Guild&&) = delete;

  // Управление деньгами
  int gold() const;
  void addGold(int amount);
  bool spendGold(int amount);

  // Прямой доступ к отряду (const): возвращает ссылку на внутренний контейнер.
  // ВАЖНО: возвращаемая ссылка даёт доступ к внутреннему состоянию; для многопоточного
  // кода это небезопасно — нужно использоавать rosterSnapshot() или другие контролируемые методы.
  const std::vector<std::unique_ptr<Hero>>& roster() const;

  // Возвращает изменяемую ссылку на внутренний контейнер.
  // ВАЖНО: небезопасно в многопоточном окружении
  std::vector<std::unique_ptr<Hero>>& rosterMutable();

  // Info / безопасные операции
  size_t rosterSize() const;
  const Hero* getHero(size_t index) const;

  // Нанять героя: принимает unique_ptr по ссылке и перемещает владение ТОЛЬКО при успехе.
  // Возвращает true при успешном найме. При неудаче владение остаётся у caller.
  bool hireHero(std::unique_ptr<Hero>& heroPtr);

  // Уволить героя по индексу. Возвращает true при успехе.
  bool fireHero(size_t index);

  // Лечить героя до полного здоровья за фиксированную плату. Возвращает true при успехе.
  bool healHero(size_t index);

  // Максимальный размер отряда
  static constexpr size_t kMaxSquadSize = 4;

 private:
  Guild();  // приватный конструктор
  ~Guild() = default;

  mutable std::mutex mutex_;  // защищает доступ к gold_ и roster_
  int gold_ = 250;            // стартовый баланс
  std::vector<std::unique_ptr<Hero>> roster_;
};

#endif  // GUILD_H_