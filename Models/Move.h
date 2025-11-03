#pragma once
#include <stdlib.h>

typedef int8_t POS_T; // псевдоним для создания переменных размером 1 байт

struct move_pos // структура определяет как храниться ход
{
    POS_T x, y;             // откуда 
    POS_T x2, y2;           // куда
    POS_T xb = -1, yb = -1; // координата побитой фигуры

    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2) : x(x), y(y), x2(x2), y2(y2) // конструктор для хода без взятия фигуры
    {
    }
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2, const POS_T xb, const POS_T yb) // конструктор для хода с взятием фигуры
        : x(x), y(y), x2(x2), y2(y2), xb(xb), yb(yb)
    {
    }

    bool operator==(const move_pos &other) const // перегруженный оператор для сравнивания двух ходов (равно)
    {
        return (x == other.x && y == other.y && x2 == other.x2 && y2 == other.y2);
    }
    bool operator!=(const move_pos &other) const // перегруженный оператор для сравнивания двух ходов (не равно)
    {
        return !(*this == other);
    }
};
