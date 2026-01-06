//Генерация комнат подземелья
#ifndef DUNGEON_H_
#define DUNGEON_H_

#include "enemy.h"
//Для умных указателей
#include <memory>
#include <string>
#include <vector>

// Тип комнаты
enum class RoomType { Enemy, Chest, Empty };

// Описание комнаты
//Используем struc просто как удобный контейнер для хранения данных
struct Room {
  std::string title;
  std::string description;
  RoomType type = RoomType::Empty;
  // В случае врагов — шаблоны врагов (в виде unique_ptr для удобства)
  std::vector<std::unique_ptr<Enemy>> enemies;
};

// Подземелье: набор линейных комнат и награда
//Используем class для описания поведения и работы с данными комнат
class Dungeon {
  public:
    Dungeon() = default;
    
    const std::string& name() const;
    //Отображения комнат подземелья
    const std::vector<Room>& rooms() const;
    int rewardGold() const;
    bool isFinal() const;

  // Фабричный метод: генерирует подземелье по имени (каждый вызов — новая генерация)
  // Реализация использует вероятности: Enemy 70%, Chest 20%, Empty 10%.
    static Dungeon generateDungeonByName(const std::string& dungeonName);

  private:
    std::string name_;
    //Вектор комнат подземелья
    std::vector<Room> rooms_;
    int rewardGold_ = 0;
    bool isFinal_ = false;
};

#endif  // DUNGEON_H_