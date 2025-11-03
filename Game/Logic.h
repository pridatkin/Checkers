#pragma once
#include <random>
#include <vector>

#include "../Models/Move.h"
#include "Board.h"
#include "Config.h"

const int INF = 1e9;

class Logic
{
  public:
    Logic(Board *board, Config *config) : board(board), config(config)
    {
        rand_eng = std::default_random_engine (
            !((*config)("Bot", "NoRandom")) ? unsigned(time(0)) : 0); // выбираем генератор случайных чисел в соответствии с конфигурацией (есть или нет)
        scoring_mode = (*config)("Bot", "BotScoringType"); // записываем способ поиска лучшего хода
        optimization = (*config)("Bot", "Optimization"); // записываем уровень оптимизации
    }

    vector<move_pos> find_best_turns(const bool color)
    {
        
    }

private:
    // функция производит ход и возвращает матрицу с выполненным ходом
    vector<vector<POS_T>> make_turn(vector<vector<POS_T>> mtx, move_pos turn) const 
    {
        if (turn.xb != -1)              // если ход со взятием вражеской фигуры, то
            mtx[turn.xb][turn.yb] = 0;  // убираем ее с поля
        if ((mtx[turn.x][turn.y] == 1 && turn.x2 == 0) || (mtx[turn.x][turn.y] == 2 && turn.x2 == 7))   // условие выхода фигуры в дамки
            mtx[turn.x][turn.y] += 2;                                                                   // превращаем фигуру в дамку
        mtx[turn.x2][turn.y2] = mtx[turn.x][turn.y];    // перемещаем фигуру на новое поле
        mtx[turn.x][turn.y] = 0;                        // убираем фигуру с прежнего поля
        return mtx;                                     // возвращаем новое состояние поля
    }


    // Функция считает 
    double calc_score(const vector<vector<POS_T>> &mtx, const bool first_bot_color) const
    {
        // color - who is max player
        double w = 0, wq = 0, b = 0, bq = 0; // переменные для хранения количества фигур на поле
        for (POS_T i = 0; i < 8; ++i)           //
        {                                       // производим обход поля
            for (POS_T j = 0; j < 8; ++j)       //
            {
                w += (mtx[i][j] == 1);      //
                wq += (mtx[i][j] == 3);     // заполняем счетчики фигур
                b += (mtx[i][j] == 2);      //
                bq += (mtx[i][j] == 4);     //
                if (scoring_mode == "NumberAndPotential") // дополнительные очки за продвижение по доске к крайней горизонтали, если scoring_mode == NumberAndPotential
                {
                    w += 0.05 * (mtx[i][j] == 1) * (7 - i);
                    b += 0.05 * (mtx[i][j] == 2) * (i);
                }
            }
        }
        if (!first_bot_color) // ели белые ходят, то обмениваемся значениями для подсчета очков
        {
            swap(b, w);
            swap(bq, wq);
        }
        if (w + wq == 0) // если фигур для хода нет
            return INF;
        if (b + bq == 0) // если у противника нет фигур
            return 0;
        int q_coef = 4; // устанавливаем вес дамки при подсчете очков
        if (scoring_mode == "NumberAndPotential") // меняем вес дамки при установленном режиме подсчета очков
        {
            q_coef = 5;
        }
        return (b + bq * q_coef) / (w + wq * q_coef); // возвращаем результат подсчета очков
    }

    double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color, const POS_T x, const POS_T y, size_t state,
                                double alpha = -1)
    {
        
    }

    double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color, const size_t depth, double alpha = -1,
                               double beta = INF + 1, const POS_T x = -1, const POS_T y = -1)
    {
        
    }

public:
    void find_turns(const bool color) // используется получения и передачи состояния доски в приватный метод void find_turns(const bool color, const vector<vector<POS_T>> &mtx)
    {
        find_turns(color, board->get_board());
    }

    void find_turns(const POS_T x, const POS_T y) // используется получения и передачи состояния доски в приватный метод void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>> &mtx)
    {
        find_turns(x, y, board->get_board());
    }

private:
    void find_turns(const bool color, const vector<vector<POS_T>> &mtx) // ищет ходы для всех фигур определенного цвета
    {
        vector<move_pos> res_turns; // вектор для записи промежуточного результата поиска ходов
        bool have_beats_before = false; // флаг указывает на то, есть ли фигура среди фигур данного цвета (color), которая может побить фигуру противника
        for (POS_T i = 0; i < 8; ++i)                       //
        {                                                   // Проходим по всем полям 
            for (POS_T j = 0; j < 8; ++j)                   //
            {                                               //
                if (mtx[i][j] && mtx[i][j] % 2 != color)    // если на поле есть фигура и она соответсвует цвету, то
                {
                    find_turns(i, j, mtx);                  // запускаем поиск ходов с клетки на которой обнаружили фигуру заданного цвета
                    if (have_beats && !have_beats_before)   // если можем побить фигуру противника и ранее такой возможности не было
                    {
                        have_beats_before = true;           // указываем но то, что есть возможность побить фигуру противника
                        res_turns.clear();                  // очищаем ранее найденные ходы
                    }
                    if ((have_beats_before && have_beats) || !have_beats_before) // если была найдена побить фигуру противника и в данной итерации такая возможность есть или ранее не была найдена побить фигуру противника
                    {
                        res_turns.insert(res_turns.end(), turns.begin(), turns.end()); // записываем результат поиска ходов в промежуточный результат
                    }
                }
            }
        }
        turns = res_turns; // записываем промежуточный результат в вектор ходов
        shuffle(turns.begin(), turns.end(), rand_eng); // перемешиваем результат в соответствии с генератором случайных чисел
        have_beats = have_beats_before; // записываем промежуточный результат в флаг возможности побить фигуру противника
    }

    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>> &mtx) // Ищет ходы для фигуры на клетке (x,y)
    {
        turns.clear();          // отчищает вектор для записи ходов
        have_beats = false;     // флаг указывает на то, можно ли побить фигуру соперника
        POS_T type = mtx[x][y]; // записиваем тип фигуры в переменную
        
        switch (type) // Выбираем есть ли возможность съесть фигуру противника в зависимости от типа фигуры
        {
        case 1: // если белые 
        case 2: // или черные
            for (POS_T i = x - 2; i <= x + 2; i += 4)       // рассматриваем четыре поля по диагонали 
            {                                               // (места куда встанет фигура после взятия)
                for (POS_T j = y - 2; j <= y + 2; j += 4)   //
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7)   // если вышли за пределы поля 
                        continue;                           // пропускаем итерацию цикла
                    POS_T xb = (x + i) / 2, yb = (y + j) / 2; // записываем координаты поля, на которой может быть фигура противника
                    if (mtx[i][j] || !mtx[xb][yb] || mtx[xb][yb] % 2 == type % 2) //проверяем свободно ли конечное поле, не пустое ли поле через которое перепрыгиваем, и не стоит ли на нем фигура нашего цвета 
                        continue;                                                 // пропускаем итерацию цикла          
                    turns.emplace_back(x, y, i, j, xb, yb); // добавляем ход в конец вектора, если все проверки прошли

                }
            }
            break;
        default: // аналогично обычным фигурам, но рассматриваем диагональ целиком (для дамок)
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    POS_T xb = -1, yb = -1;
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                        {
                            if (mtx[i2][j2] % 2 == type % 2 || (mtx[i2][j2] % 2 != type % 2 && xb != -1))
                            {
                                break;
                            }
                            xb = i2;
                            yb = j2;
                        }
                        if (xb != -1 && xb != i2)
                        {
                            turns.emplace_back(x, y, i2, j2, xb, yb);
                        }
                    }
                }
            }
            break;
        }
        // check other turns
        if (!turns.empty()) // если нашли ходы со взятием фигуры, то ставим флаг true и завершаем поиск ходов
        {
            have_beats = true;
            return;
        }
        switch (type) // Выбираем есть ли возможность походить в зависимости от типа фигуры
        {
        case 1: // если белая фигура
        case 2: // или черная фигура
            // check pieces
            {
                POS_T i = ((type % 2) ? x - 1 : x + 1);
                for (POS_T j = y - 1; j <= y + 1; j += 2)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7 || mtx[i][j])
                        continue;
                    turns.emplace_back(x, y, i, j);
                }
                break;
            }
        default: // то же для дамок
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                            break;
                        turns.emplace_back(x, y, i2, j2);
                    }
                }
            }
            break;
        }
    }

  public:
    vector<move_pos> turns; // вектор для хранения ходов
    bool have_beats;    // флаг указывающий на возможность побить фигуру противника
    int Max_depth;      // Определяет насколько далеко будут просчитываться ходы (глубину графа)

  private:
    default_random_engine rand_eng; // объект для харения генератора случайных чисел
    string scoring_mode; // строка для хранения способа поиска лучшего хода
    string optimization; // строка для хранения уровня оптимизации поиска лучшего хода 
    vector<move_pos> next_move;
    vector<int> next_best_state;
    Board *board;   // ссылка на доску
    Config *config; // ссылка на настройки игры
};
