#include "battle.h"
#include "dungeon.h"
#include "factory.h"
#include "guild.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <random>
#include <algorithm>


//clang++ -std=c++17 -O2 -Wall -Wextra -Iinclude main.cpp $(find src -name '*.cpp' -print) -o bin/guild_rogue 
//./bin/guild_rogue


// Локальный генератор случайных чисел для main.cpp (используется в меню/событиях)
static std::mt19937& localRng() {
  static std::random_device rd;
  static std::mt19937 mt(rd());
  return mt;
}

// Утилита для отображения роли 
static const char* roleToString(Role r) {
  switch (r) {
    case Role::Warrior: return "Warrior";
    case Role::Mage: return "Mage";
    case Role::Archer: return "Archer";
    case Role::Cleric: return "Cleric";
    default: return "Unknown";
  }
}

// Показываем главное меню
static void showMainMenu() {
  std::cout << "\n=== Гильдия Аркадия ===\n";
  std::cout << "Золото: " << Guild::instance().gold() << "\n";
  std::cout << "1) Нанять героев\n";
  std::cout << "2) Управление отрядом\n";
  std::cout << "3) Выбрать задание\n";
  std::cout << "4) Лечение героев (50 золота)\n";
  std::cout << "5) Выйти из игры\n> ";
}

// Меню найма. Кандидаты хранятся статически и обновляются только при выборе опции 5.
static void hireMenu() {
  static std::vector<std::unique_ptr<Hero>> candidates;
  if (candidates.empty()) candidates = HeroFactory::generateCandidates();

  while (true) {
    std::cout << "\n--- Меню найма ---\n";
    for (size_t i = 0; i < candidates.size(); ++i) {
      auto& c = candidates[i];
      std::cout << i + 1 << ") ";
      if (c) {
        std::cout << c->name() << " (L" << c->level() << ", " << roleToString(c->role()) << ")"
                  << " — Цена найма: " << 100 * c->level();
      } else {
        std::cout << "(занят/недоступен)";
      }
      std::cout << "\n";
    }
    std::cout << "----------------------------------------\n";
    std::cout << "1-4) Нанять соответствующего кандидата (Баланс: " << Guild::instance().gold() << ")\n";
    std::cout << "5) Обновить список (50 золота)\n";
    std::cout << "6) Назад\n> ";

    int ch;
    std::cin >> ch;
    if (ch >= 1 && ch <= static_cast<int>(candidates.size())) {
      size_t idx = ch - 1;
      if (!candidates[idx]) {
        std::cout << "Кандидат недоступен. Выберите другого или обновите список (5).\n";
        continue;
      }
      if (Guild::instance().hireHero(candidates[idx])) {
        std::cout << "Герой нанят.\n";
        // слот останется пустым — обновление только через опцию 5
      } 
      else {
        std::cout << "Не удалось нанять героя (проверьте золото или размер отряда).\n";
      }
    } 

    else if (ch == 5) {
      if (Guild::instance().spendGold(50)) {
        candidates = HeroFactory::generateCandidates();
        std::cout << "Список обновлен.\n";
      } 
      else {
        std::cout << "Не хватает золота для обновления.\n";
      }
    } 
    else { break; }
  }
}

// Управление отрядом — теперь выводим роль рядом с именем и корректный maxMp.
static void manageSquad() {
  auto& roster = Guild::instance().rosterMutable();
  while (true) {
    std::cout << "\n--- Управление отрядом ---\n";
    if (roster.empty()) {
      std::cout << "Отряд пуст.\n";
    } 
    else {
      for (size_t i = 0; i < roster.size(); ++i) {
        auto& h = roster[i];
        std::cout << i + 1 << ") " << h->name()
                  << " [" << roleToString(h->role()) << "]"
                  << " L" << h->level()
                  << " HP " << h->hp() << "/" << h->maxHp()
                  << " MP " << h->mp() << "/" << h->maxMp()
                  << "\n";
      }
    }
    std::cout << "----------------------------------------\n";
    std::cout << "1) Уволить героя\n2) Изменить порядок\n3) Назад\n> ";
    int ch;
    std::cin >> ch;
    if (ch == 1) {
      std::cout << "Индекс героя для увольнения: ";
      int idx; std::cin >> idx;
      if (idx >= 1 && static_cast<size_t>(idx) <= roster.size()) {
        Guild::instance().fireHero(idx - 1);
        std::cout << "Герой уволен.\n";
      } 
      else {
        std::cout << "Неверный индекс.\n";
      }
    } 
    else if (ch == 2) {
      if (roster.size() < 2) { std::cout << "Недостаточно героев для перестановки.\n"; continue; }
      std::cout << "Введите два индекса для обмена (например: 1 2): ";
      int a, b; std::cin >> a >> b;
      if (a >= 1 && b >= 1 && static_cast<size_t>(a) <= roster.size() &&
          static_cast<size_t>(b) <= roster.size()) {
        std::swap(roster[a - 1], roster[b - 1]);
        std::cout << "Порядок обновлен.\n";
      } 
      else {
        std::cout << "Неверные индексы.\n";
      }
    } 
    else break;
  }
}

// Лечение героя или всех героев за плату
static void healMenu() {
  auto& roster = Guild::instance().rosterMutable();
  if (roster.empty()) { std::cout << "Отряд пуст.\n"; return; }
  std::cout << "\n--- Лечение героев ---\n";
  //Выводим героев и их статы
  for (size_t i = 0; i < roster.size(); ++i) {
    std::cout << i + 1 << ") " << roster[i]->name() << " HP " << roster[i]->hp()
              << "/" << roster[i]->maxHp() << "\n";
  }
  std::cout << "Выберите героя для лечения (индекс) или 0 для всех: ";
  int idx; std::cin >> idx;
  if (idx == 0) {
    for (size_t i = 0; i < roster.size(); ++i) {
      if (Guild::instance().gold() < 50) { std::cout << "Не хватает золота на дальнейшее лечение.\n"; break; }
      Guild::instance().healHero(i);
    }
    std::cout << "Лечение всех выполнено.\n";
  } 
  else if (idx >= 1 && static_cast<size_t>(idx) <= roster.size()) {
    if (Guild::instance().healHero(idx - 1)) std::cout << "Герой вылечен.\n"; else std::cout << "Не хватает золота.\n";
  } 
  else {
    std::cout << "Отмена.\n";
  }
}

// QuestMenu: при выборе задания каждый заход в подземелье генерирует новое подземелье
static void questMenu() {
  std::vector<std::string> dungeonNames = {"Goblin Caves", "Magic Forest", "Undead Castle", "Temple of Darkness", "Dragon Lair"};
  std::cout << "\n--- Выбор задания ---\n";
  for (size_t i = 0; i < dungeonNames.size(); ++i) std::cout << i + 1 << ") " << dungeonNames[i] << "\n";
  std::cout << dungeonNames.size() + 1 << ") Назад\n> ";
  int ch; std::cin >> ch;
  if (ch < 1 || ch > static_cast<int>(dungeonNames.size())) return;
  std::string chosen = dungeonNames[ch - 1];

  auto& roster = Guild::instance().rosterMutable();
  if (roster.empty()) { std::cout << "Отряд пуст — наймите героев перед походом.\n"; return; }

  // Генерируем подземелье прямо сейчас (каждый вход новое подземелье)
  Dungeon d = Dungeon::generateDungeonByName(chosen);
  std::cout << "Отправляемся в " << d.name() << " (" << d.rooms().size() << " комнат)\n";

  int roomsCleared = 0;
  for (size_t i = 0; i < d.rooms().size(); ++i) {
    const Room& r = d.rooms()[i];
    std::cout << "\n--- " << r.title << " ---\n";
    if (!r.description.empty()) std::cout << r.description << "\n";

    if (r.type == RoomType::Enemy) {
      // Подготавливаем временные объекты врагов для боя (чтобы оригинальные объекты из Dungeon остались в d)
      std::vector<std::unique_ptr<Enemy>> temp;
      std::vector<Enemy*> enemyPtrs;
      for (auto& eptr : r.enemies) {
        // клонируем параметры в новый объект (чтобы у каждой комнаты был свой набор экземпляров)
        temp.push_back(std::make_unique<Enemy>(eptr->name(), eptr->hp(), eptr->attack(), eptr->defense()));
      }
      for (auto& p : temp) enemyPtrs.push_back(p.get());
      std::vector<Hero*> heroPtrs;
      for (auto& h : roster) heroPtrs.push_back(h.get());

      Battle battle(heroPtrs, enemyPtrs);
      bool heroesWin = battle.executeBattle();
      //Герои не победили
      if (!heroesWin) {
        // Удаляем мёртвых из отряда
        for (auto it = roster.begin(); it != roster.end();) {
          if (!(*it)->isAlive()) it = roster.erase(it); else ++it;
        }
        std::cout << "Вы отступаете в город.\n";
        if (Guild::instance().roster().empty() && Guild::instance().gold() < 100) {
          std::cout << "Все герои мертвы и недостаточно золота для найма нового героя. Игра проиграна.\n";
          exit(0);
        }
        return;
      } 
      //Герои одержади победу
      else {
        int gained = 20 * static_cast<int>(r.enemies.size());
        Guild::instance().addGold(gained);
        std::cout << "Награда за комнату: " << gained << " золота.\n";
        // Удаляем умерших из отряда
        for (auto it = roster.begin(); it != roster.end();) {
          if (!(*it)->isAlive()) it = roster.erase(it); else ++it;
        }
      }
    } 
    //Если в комнате сундук
    else if (r.type == RoomType::Chest) {
      std::uniform_int_distribution<int> pickDist(0, 1);
      std::uniform_int_distribution<int> goldDist(50, 150);
      if (pickDist(localRng()) == 0) {
        int g = goldDist(localRng());
        Guild::instance().addGold(g);
        std::cout << "Вы нашли сундук с " << g << " золотом.\n";
      } 
      else {
        for (auto& h : roster) if (h->isAlive()) h->heal(20);
        std::cout << "В сундуке оказалось лечебное зелье — отряд немного восстановлен.\n";
      }
    } 
    else {
      // Пустая комната — ничего дополнительно не происходит
      // (описание уже выведено выше)
    }

    ++roomsCleared;
    //Возмножность отступить после пройденной комнаты 
    if (i + 1 < d.rooms().size()) {
      std::cout << "1) Продолжить  2) Отступить\n> ";
      int c; std::cin >> c;
      if (c == 2) {
        int reward = d.rewardGold() * roomsCleared / static_cast<int>(d.rooms().size());
        Guild::instance().addGold(reward);
        std::cout << "Вы отступили и получили " << reward << " золота.\n";
        return;
      }
    } 
    else {
      // последняя комната — даём полную награду
      Guild::instance().addGold(d.rewardGold());
      std::cout << "Вы прошли подземелье! Награда: " << d.rewardGold() << " золота.\n";
      if (d.isFinal()) {
        std::cout << "Дракон побежден! Игра завершена — теперь вы легенда Аркадии!!!\n";
        exit(0);
      }
      return;
    }
  }
}

// Вынесенная игровая функция — основной цикл, чтобы main() оставался коротким.
static void runGame() {
  std::cout << "Добро пожаловать в Гильдию Аркадии!\n";
  while (true) {
    showMainMenu();
    int choice;
    if (!(std::cin >> choice)) break;
    if (choice == 1) hireMenu();
    else if (choice == 2) manageSquad();
    else if (choice == 3) questMenu();
    else if (choice == 4) healMenu();
    else if (choice == 5) { std::cout << "Выход из игры.\n"; break; }
    else std::cout << "Неверный выбор.\n";
  }
}

//А вот тута наконец-то main
int main() {
  runGame();
  return 0;
}