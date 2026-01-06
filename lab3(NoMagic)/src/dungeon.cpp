#include "dungeon.h"

#include <algorithm>
#include <random>
//Удобная запись строки
#include <sstream>

// Локальный генератор
static std::mt19937& dungeonRng() {
  static std::random_device rd;
  static std::mt19937 mt(rd());
  return mt;
}

// Геттеры
const std::string& Dungeon::name() const { return name_; }
const std::vector<Room>& Dungeon::rooms() const { return rooms_; }
int Dungeon::rewardGold() const { return rewardGold_; }
bool Dungeon::isFinal() const { return isFinal_; }

// Вспомогательные: создаёт Enemy шаблон с именем Monster(i)
//Static позволяет пользоваться этим методом только внутри dungeon.c
static std::unique_ptr<Enemy> createMonster(int index, int hp, int attack, int defense) {
  //Поток вывода
  std::ostringstream ss;
  ss << "Monster(" << index << ")";
  //Создание моснтра
  return std::make_unique<Enemy>(ss.str(), hp, attack, defense);
}

// Вспомогательная генерация комнаты типа Enemy: заполняет случайным числом монстров
static Room generateEnemyRoom(int roomIndex, int difficultyLevel) {
  Room r;
  r.title = "Комната с монстрами";
  r.description = "В этой комнате вас поджидают обитатели...";
  r.type = RoomType::Enemy;

  // Чем выше сложность — больше и сильнее монстров
  //Генерирурем количество монстров в комнатах в зависимости от сложности
  //Случайное число от 1 до 1+сложность данжа
  std::uniform_int_distribution<int> countDist(1, 1 + difficultyLevel);
  //Записываем в переменную случайное выпавшее число
  int count = countDist(dungeonRng());
  for (int i = 0; i < count; ++i) {
    //Создаём монстра
    int hp = 20 + difficultyLevel * 15 + i * 5;
    int attack = 4 + difficultyLevel * 2 + i;
    int defense = 1 + difficultyLevel * 2;
    r.enemies.push_back(createMonster(roomIndex * 10 + i + 1, hp, attack, defense));
  }
  //Возвращаем объект типа Room
  return r;
}

// Генерация сундука
static Room generateChestRoom() {
  Room r;
  r.title = "Комната со сундуком";
  r.description = "В комнате вы нашли сундук. Внутри может быть золото или зелье.";
  r.type = RoomType::Chest;
  return r;
}

// Пустая комната
static Room generateEmptyRoom() {
  Room r;
  r.title = "Пустая комната";
  r.description = "Вы видите лишь голый пол и стены. Комната пустует";
  r.type = RoomType::Empty;
  return r;
}

// Основная фабрика подземелий. Каждый вызов генерирует новое подземелье по имени.
// Для финального "Dragon Lair" гарантируем финального босса в последней комнате.
Dungeon Dungeon::generateDungeonByName(const std::string& dungeonName) {
  Dungeon d;
  d.name_ = dungeonName;
  d.isFinal_ = false;

  int roomsCount = 2;
  int reward = 200;
  int difficulty = 1;
  //Генерирует подземелья по названию
  if (dungeonName == "Goblin Caves") {
    roomsCount = 2;
    reward = 200;
    difficulty = 1;
  } else if (dungeonName == "Magic Forest") {
    roomsCount = 3;
    reward = 350;
    difficulty = 2;
  } else if (dungeonName == "Undead Castle") {
    roomsCount = 4;
    reward = 500;
    difficulty = 3;
  } else if (dungeonName == "Temple of Darkness") {
    roomsCount = 4;
    reward = 650;
    difficulty = 4;
  } else if (dungeonName == "Dragon Lair") {
    roomsCount = 5;
    reward = 1000;  // основная награда
    difficulty = 5;
    d.isFinal_ = true;
  }
  //Награда в зависимоси от подземелья 
  d.rewardGold_ = reward;
  // Генерируем комнаты линейно: для каждой комнаты используем вероятности (70% enemy, 20% chest, 10% empty).
  std::uniform_real_distribution<double> prob(0.0, 1.0);
  for (int i = 0; i < roomsCount; ++i) {
    // Если это последняя комната и финальный данж — делаем босса-дракона (как Enemy с большой статой).
    if (d.isFinal_ && i == roomsCount - 1) {
      Room bossRoom;
      bossRoom.title = "Логово Дракона";
      bossRoom.description = "Последняя комната — здесь дремоет владыка-дракон. Его придётся победиь!";
      bossRoom.type = RoomType::Enemy;
      // Один мощный монстр — дракон
      bossRoom.enemies.push_back(std::make_unique<Enemy>("Dragon(Boss)", 350, 25, 8));
      d.rooms_.push_back(std::move(bossRoom));
      continue;
    }
    //Генерируем комнаты 
    double p = prob(dungeonRng());
    if (p < 0.70) {
      // враги
      d.rooms_.push_back(generateEnemyRoom(i + 1, difficulty));
    } else if (p < 0.90) {
      d.rooms_.push_back(generateChestRoom());
    } else {
      d.rooms_.push_back(generateEmptyRoom());
    }
  }

  return d;
}