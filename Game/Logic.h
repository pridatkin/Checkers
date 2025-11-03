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

    // Основная публичная функция для поиска лучшего хода
    // Возвращает последовательность ходов для достижения лучшей позиции
    // color - цвет текущего игрока (true/false для белых/черных)
    vector<move_pos> find_best_turns(const bool color)
    {
		// очищаем векторы для записи лучшей последовательности ходов
        next_move.clear();
        next_best_state.clear();

		// запускаем поиск лучшего хода
        find_first_best_turn(board->get_board(), color, -1, -1, 0);

		// формируем результат в виде вектора ходов
        vector<move_pos> res;
        int state = 0;
        do {
            res.push_back(next_move[state]);
            state = next_best_state[state];
        } while (state != -1 && next_move[state].x != -1);
        return res;
    }

private:
    // Рекурсивная функция для поиска лучшего первого хода и построения цепочки ходов
    // mtx - текущее состояние доски
    // color - цвет текущего игрока
    // x, y - координаты фигуры, которая должна продолжить взятие (если применимо)
    // state - текущий индекс в векторах next_move/next_best_state
    // alpha - текущее значение alpha для альфа-бета отсечения
    // Возвращает оценку лучшей найденной позиции
    double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color, const POS_T x, const POS_T y, size_t state,
        double alpha = -1)
    {
        // Инициализируем векторы для нового состояния
        next_move.emplace_back(-1, -1, -1, -1);
        next_best_state.push_back(-1);

        // Если это не начальное состояние, ищем возможные ходы для указанной фигуры
		if (state != 0) {
            find_turns(x, y, mtx);
        }
       
		// сохраняем промежуточный результат для рекурсивного подсчета
        auto now_turns = turns;
        auto now_have_beats = have_beats;

        // Если нет обязательных взятий и это не начальное состояние,
        // переходим к рекурсивному поиску ходов противника
		if (!now_have_beats && state != 0) { 
			return find_best_turns_rec(mtx, 1 - color, 0, alpha); // передаем параметры для подсчета хода соперника, цвет меняется, глубина увеличивается, позиция не передается
        }

		double best_score = -1; // объявляем переменную для хранения лучшей оценки

		for (auto turn : now_turns) { // проходим по возможным ходам
			size_t new_state = next_move.size(); // определяем индекс для нового состояния
			double score; // переменная для хранения оценки текущего хода
			if (now_have_beats) { // если есть побития
				score = find_first_best_turn(make_turn(mtx, turn), color, turn.x2, turn.y2, new_state, best_score); // передаем параметры для подсчета следующего хода в серии побитий, глубина и цвет фигуры не меняется х и у соответствуют позиции фигуры после взятия
            }
            else {
				score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, 0, best_score); // передаем параметры для подсчета хода соперника, цвет меняется, глубина увеличивается, позиция не передается
            }
			if (score > best_score) { // если нашли лучший ход
				best_score = score; // обновляем лучшую оценку
				next_move[state] = turn; // записываем ход в вектор лучшей последовательности ходов
				next_best_state[state] = (now_have_beats ? new_state : -1); // записываем связь между состояниями в последовательности ходов
            }
        }
        return best_score;
    }

    // Рекурсивная функция минимакс с альфа-бета отсечением для оценки позиций
    // mtx - текущее состояние доски
    // color - цвет текущего игрока
    // depth - текущая глубина поиска
    // alpha, beta - границы для альфа-бета отсечения
    // x, y - координаты фигуры, которая должна продолжить взятие (если применимо)
    // Возвращает оценку позиции для текущего игрока
    double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color, const size_t depth, double alpha = -1,
        double beta = INF + 1, const POS_T x = -1, const POS_T y = -1)
    {
        if (depth == Max_depth) { // условие выхода из рекурсии, если достигли максимальной глубины
            return calc_score(mtx, (depth % 2 == color)); // возвращаем подсчет очков из данной позиции
        }

        if (x != -1) {  // значит, что было побитие, ищем ход данной фигуры
            find_turns(x, y, mtx);
        }
        else {          // ищем ход по всем фигурам данного цвета
            find_turns(color, mtx);
        }
        auto now_turns = turns; // сохраняем промежуточный результат для рекурсивного подсчета
        auto now_have_beats = have_beats;

        if (!now_have_beats && x != -1) { // нечего бить и не серия побитий
            return find_best_turns_rec(mtx, 1 - color, depth + 1, alpha, beta);
        }

        if (now_turns.empty()) { // если нет ходов 
            return (depth % 2 ? 0 : INF); // возвращаем проигрыш или выйгрыш в зависимости от того, чей ход
        }

        double min_score = INF + 1; // объявляем переменные для хранения оценки для алгоритма Min-Max
        double max_score = -1;      // значения выбраны так, чтобы они точно заменились при подсчете
        for (auto turn : now_turns) {   // проходим по возможным ходам
            double score = 0;
            if (now_have_beats) { // если есть побития
                score = find_best_turns_rec(make_turn(mtx, turn), color, depth, alpha, beta, turn.x2, turn.y2); //передаем параметры для подсчета следующего хода в серии побитий, глубина и цвет фигуры не меняется х и у соответствуют позиции фигуры после взятия
            }
            else { // если нет побитий
                score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, depth + 1, alpha, beta); //передаем параметры для подсчета хода соперника, цвет меняется, глубина увеличивается, позиция не передается
            }

            min_score = min(min_score, score); // выбираем найменшее количество очков
            max_score = max(max_score, score); // выбираем наибольшее количество очков

            if (depth % 2) {   // в зависимости от того чей ход, двигаем альфа или бета границу
                alpha = max(alpha, max_score);
            }
            else {
                beta = min(beta, min_score);
            }
            if (optimization != "O0" && alpha > beta) { // производим строгое отсечение в случае оптимизации
                break;
            }
            if (optimization == "O2" && alpha == beta) { // производим нестрогое отсечение в случае если оптмизация O2
                return (depth % 2 ? max_score + 1 : min_score - 1); // возвращаем значения таким образом, чтобы они не были выбраны как оптимум выше
            }
            
        }

        return (depth % 2 ? max_score : min_score); // в зависимости для кого производиться подсчеп передаем минимальное или максимальное значение
    }

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


	// Функция оценивает позицию на доске и возвращает значение оценки
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
    vector<move_pos> next_move; // вектор для хранения последовательности ходов, приводящих к лучшей позиции
    vector<int> next_best_state; // вектор для связи между состояниями в последовательности ходов
    Board *board;   // ссылка на доску
    Config *config; // ссылка на настройки игры
};
