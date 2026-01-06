#include "guild.h"

#include <algorithm>

// Meyers singleton: потокобезопасна в C++11+
Guild& Guild::instance() noexcept {
  static Guild inst;
  return inst;
}

Guild::Guild() = default;

// Деньги
int Guild::gold() const {
  std::lock_guard<std::mutex> lock(mutex_); // Блокировки мьютекса
  return gold_;
} //Автоматический разблокировка при выходе 

//Начисление золота
void Guild::addGold(int amount) {
  if (amount <= 0) return;
  std::lock_guard<std::mutex> lock(mutex_);
  gold_ += amount;
}

//Трата золота
bool Guild::spendGold(int amount) {
  if (amount <= 0) return false;
  std::lock_guard<std::mutex> lock(mutex_);
  //Если не хватает золота, операция отклонена
  if (gold_ < amount) return false;
  gold_ -= amount;
  return true;
}

// Прямой const-доступ к отряду (возвращаем ссылку на внутренний контейнер).Только для чтения
const std::vector<std::unique_ptr<Hero>>& Guild::roster() const {
  return roster_;
}

// Возвращаем mutable-ссылку (как было раньше). См. предупреждение в заголовке.
std::vector<std::unique_ptr<Hero>>& Guild::rosterMutable() {
  return roster_;
}

// Отряд (без передачи владения)
size_t Guild::rosterSize() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return roster_.size();
}

const Hero* Guild::getHero(size_t index) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (index >= roster_.size()) return nullptr;
  return roster_[index].get();
}

// Нанимаем героя: перевод владения только при успешном найме.
// Сначала проверяем условия, затем вставляем и списываем плату.
// Используем лок на весь блок для атомарности с точки зрения состояния гильдии.
bool Guild::hireHero(std::unique_ptr<Hero>& heroPtr) {
  if (!heroPtr) return false;

  std::lock_guard<std::mutex> lock(mutex_);

  if (roster_.size() >= kMaxSquadSize) {
    return false;
  }

  int basePrice = 100 * heroPtr->level();
  if (gold_ < basePrice) {
    return false;
  }

  // Перемещаем временно в локальный указатель, чтобы иметь возможность восстановить
  // в случае исключения при push_back.
  std::unique_ptr<Hero> tmp = std::move(heroPtr);
  try {
    roster_.push_back(std::move(tmp));
    // После успешной вставки списываем плату.
    gold_ -= basePrice;
    return true;
  } catch (...) {
    // Если вставка провалилась, вернём владение назад и сообщим о неудаче.
    heroPtr = std::move(tmp);
    return false;
  }
}

// Уволить героя по индексу
bool Guild::fireHero(size_t index) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (index >= roster_.size()) return false;
  roster_.erase(roster_.begin() + static_cast<std::ptrdiff_t>(index));
  return true;
}

// Лечим героя до полного здоровья за фиксированную плату
bool Guild::healHero(std::size_t index) {
  constexpr int kHealCost = 50;
  std::lock_guard<std::mutex> lock(mutex_);
  if (index >= roster_.size()) return false;
  if (gold_ < kHealCost) return false;
  Hero* h = roster_[index].get();
  if (!h) return false;
  h->heal(h->maxHp());
  gold_ -= kHealCost;
  return true;
}