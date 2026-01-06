#ifndef CREATURE_H_
#define CREATURE_H_

// Базовый полиморфный интерфейс для всех игровых существ (герои и враги).
// Включаем <string> здесь, чтобы производные могли использовать std::string в
// сигнатурах методов без дополнительных подключений.
#include <string>


//!!!Абстрактный класс!!!

class Creature {
  public:
  //Виртуальный деструктор. Это позволяет корректно удалять объекты через указатель на Creature
    virtual ~Creature() = default;

  // Основные свойства/действия, ожидаемые от существа.
  // Возвращаем ссылку на имя, геттеры и методы, влияющие на состояние.
    virtual const std::string& name() const = 0;  // read-only имя
    virtual int hp() const = 0;
    virtual int maxHp() const = 0;
    virtual bool isAlive() const = 0;

  // Атака/защита: используются для расчёта урона.
    virtual int attack() const = 0;
    virtual int defense() const = 0;

  // Уменьшить hp на amount (не печатает сообщения — это делает вызывающий код).
    virtual void takeDamage(int amount) = 0;

  // Сброс временных эффектов (например, множители) в конце раунда.
    virtual void resetTemporaryEffects() = 0;
};

#endif  // CREATURE_H_