#include "hero.h"
#include "enemy.h"

//Для операций с контейнерами
#include <algorithm>
#include <iostream>
#include <random>

// Локальный глобальный генератор для случайных операций внутри hero.cpp
static std::mt19937& globalRng() { // Без static - глобальная видимость
  static std::random_device rd;
  static std::mt19937 mt(rd());
  return mt;
}

// Конструктор + семантика перемещения с move
Hero::Hero(std::string name, Role role, int level, int maxHp, int maxMp,
           int attack, int defense)
    : name_(std::move(name)),
      role_(role),
      level_(level),
      hp_(maxHp),
      maxHp_(maxHp),
      mp_(maxMp),
      maxMp_(maxMp),
      attack_(attack),
      defense_(defense),
      defenseMultiplier_(1.0) {}

// Реализации геттеров 
const std::string& Hero::name() const { return name_; }
int Hero::hp() const { return hp_; }
int Hero::maxHp() const { return maxHp_; }
bool Hero::isAlive() const { return hp_ > 0; }
int Hero::attack() const { return attack_; }
int Hero::defense() const { return static_cast<int>(defense_ * defenseMultiplier_); } // Преобразование
//в int числа через static_cast

void Hero::takeDamage(int amount) {
  hp_ -= amount;
  if (hp_ < 0) hp_ = 0;
}

//Эффект спадает и мультипликатор брони возвращается в норму
void Hero::resetTemporaryEffects() { defenseMultiplier_ = 1.0; }

//Выводим роль игрока из enum класса
Role Hero::role() const { return role_; }
int Hero::level() const { return level_; }

int Hero::mp() const { return mp_; }
int Hero::maxMp() const { return maxMp_; }

//Восставновление маны
void Hero::restoreMp(int amount) {
  mp_ += amount;
  if (mp_ > maxMp_) mp_ = maxMp_;
}

//Использование маны
bool Hero::useMp(int amount) {
  if (mp_ < amount) return false;
  mp_ -= amount;
  return true;
}

// Обычная атака по цели. Сообщение о смерти выводится здесь сразу после нанесения урона.
void Hero::attackTarget(Enemy* enemy) {
  if (!enemy || !enemy->isAlive()) return;
  //Расчёт урона
  int dmg = std::max(1, attack_ - enemy->defense());
  //Враг получает урон
  enemy->takeDamage(dmg);
  std::cout << name_ << " атакует " << enemy->name() << " на " << dmg << " урона.\n";
  if (!enemy->isAlive()) {
    std::cout << enemy->name() << " повержен!\n";
  }
}

//Лечение
void Hero::heal(int amount) {
  hp_ += amount;
  if (hp_ > maxHp_) hp_ = maxHp_;
}

//Увеличение модификатора брони
void Hero::multiplyDefenseMultiplier(double mul) { defenseMultiplier_ *= mul; }

// Использование способности в зависимости от роли.
// Возвращает true при успешном выполнении (MP списана), false — если не хватает MP.
bool Hero::useAbility(std::vector<Enemy*>& enemies,
                      std::vector<Hero*>& allies) {
  //switch выбирает какой case использовать в зависимости от значения role_
  switch (role_) {
    case Role::Warrior: {
      //Стоимость каста
      int cost = 10 + 2 * level_;
      //Проверка на ману
      if (!useMp(cost)) {
        std::cout << name_ << " не хватает MP для 'Прикрытие'.\n";
        return false;
      }
      //Количество целей для бонуса
      int count = 1 + (level_ / 2);
      //Добавляем всех союзником в кандидаты на цель способности
      std::vector<Hero*> candidates;
      for (Hero* h : allies) {
        if (h->isAlive()) candidates.push_back(h);
      }
      //Случайно меняем порядок в контейнере
      std::shuffle(candidates.begin(), candidates.end(), globalRng());
      int applied = 0;
      for (Hero* h : candidates) {
        if (applied >= count) break;
        double bonus = 1.5 + 0.05 * level_;
        h->multiplyDefenseMultiplier(bonus);
        std::cout << name_ << " накладывает 'Прикрытие' на " << h->name()
                  << " (защита x" << bonus << ")\n";
        ++applied;
      }
      return true;
    }
    case Role::Mage: {
      //Стоимость каста (Дальше у всех аналогично)
      int cost = 15 + 3 * level_;
      //Проверка на ману(Дальше у всех аналогично)
      if (!useMp(cost)) {
        std::cout << name_ << " не хватает MP для 'Огненный шар'.\n";
        return false;
      }
      if (enemies.empty()) return true;
      //Мультипликатор урона
      double mul = 1.5 + 0.1 * level_;
      std::vector<Enemy*> alive;
      //Создаем контейнер живых мобов
      for (auto* e : enemies) {
        if (e->isAlive()) alive.push_back(e);
      }
      if (alive.empty()) return true;
      //Тасуем контейнер
      std::shuffle(alive.begin(), alive.end(), globalRng());
      //Количество целей до 2(Вдруг остался всего 1 моб)
      int hits = std::min<int>(2, alive.size());
      for (int i = 0; i < hits; ++i) {
        Enemy* e = alive[i];
        //Высчитываем урон
        int dmg = std::max(1, static_cast<int>(attack_ * mul) - e->defense());
        e->takeDamage(dmg);
        std::cout << name_ << " выпускает 'Огненный шар' по " << e->name()
                  << " на " << dmg << " урона.\n";
        //Если вдруг убили, выводим инфу
        if (!e->isAlive()) std::cout << e->name() << " Сгорел заживо!\n";
      }
      return true;
    }
    case Role::Archer: {
      int cost = 12 + 2 * level_;
      if (!useMp(cost)) {
        std::cout << name_ << " не хватает MP для 'Выстрел Гадюки'.\n";
        return false;
      }
      std::vector<Enemy*> alive;
      for (auto* e : enemies) {
        if (e->isAlive()) alive.push_back(e);
      }
      if (alive.empty()) return true;
      std::shuffle(alive.begin(), alive.end(), globalRng());
      Enemy* target = alive.front();
      double mul = 1.2 + 0.08 * level_;
      int dmg = std::max(1, static_cast<int>(attack_ * mul) - target->defense());
      target->takeDamage(dmg);
      std::cout << name_ << " делает 'Выстрел Гадюки' в " << target->name()
                << " на " << dmg << " урона.\n";
      if (!target->isAlive()) std::cout << target->name() << " Токсично помирает!\n";

      // Восстанавливаем MP случайному другому союзнику (исключая себя).
      std::vector<Hero*> alliesAlive;
      for (Hero* h : allies) {
        if (h->isAlive() && h != this) alliesAlive.push_back(h);
      }
      int mpGain = 10 + 3 * level_;
      if (!alliesAlive.empty()) {
        std::shuffle(alliesAlive.begin(), alliesAlive.end(), globalRng());
        Hero* rec = alliesAlive.front();
        rec->restoreMp(mpGain);
        std::cout << name_ << " восстанавливает " << mpGain << " MP у "
                  << rec->name() << ".\n";
      } else {
        // Никого другого нет — восстанавливаем себе
        restoreMp(mpGain);
        std::cout << name_ << " восстанавливает " << mpGain << " MP себе (нет других союзников).\n";
      }
      return true;
    }
    case Role::Cleric: {
      int cost = 20 + 3 * level_;
      if (!useMp(cost)) {
        std::cout << name_ << " не хватает MP для 'Лечение'.\n";
        return false;
      }
      std::vector<Hero*> wounded;
      for (Hero* h : allies) {
        if (h->isAlive() && h->hp() < h->maxHp()) wounded.push_back(h);
      }
      //Если союзники есть
      if (!wounded.empty()) {
        std::shuffle(wounded.begin(), wounded.end(), globalRng());
        Hero* tgt = wounded.front();
        //Размер хила
        int healAmount = 30 + 8 * level_;
        tgt->heal(healAmount);
        std::cout << name_ << " лечит " << tgt->name() << " на " << healAmount << " HP.\n";
      } else {
        int healAmount = 15 + 4 * level_;
        heal(healAmount);
        std::cout << name_ << " лечит себя на " << healAmount << " HP (все союзники здоровы).\n";
      }
      return true;
    }
  }
  return false;
}