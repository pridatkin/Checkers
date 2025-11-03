#pragma once
#include <chrono>
#include <thread>

#include "../Models/Project_path.h"
#include "Board.h"
#include "Config.h"
#include "Hand.h"
#include "Logic.h"

class Game
{
  public:
    Game() : board(config("WindowSize", "Width"), config("WindowSize", "Hight")), hand(&board), logic(&board, &config)
    {
        ofstream fout(project_path + "log.txt", ios_base::trunc);
        fout.close();
    }

    // to start checkers
    int play()
    {
        auto start = chrono::steady_clock::now(); // сохраняем время начала для рассчета времени игры
        if (is_replay) // если была нажата кнопка переиграть
        {
            logic = Logic(&board, &config);
            config.reload();
            board.redraw();
        }
        else
        {
            board.start_draw();
        }
        is_replay = false;

        int turn_num = -1;
        bool is_quit = false;
        const int Max_turns = config("Game", "MaxNumTurns"); // загружаем максимальное количество шагов из конфигурационного файла
        while (++turn_num < Max_turns)  // Цикл для подсчета и выполнения ходов
        {
            beat_series = 0; // устанавливаем счетчиу взятий вражеских фигур на 0
            logic.find_turns(turn_num % 2);
            if (logic.turns.empty())
                break;
            logic.Max_depth = config("Bot", string((turn_num % 2) ? "Black" : "White") + string("BotLevel")); // Загружаем глубину поискаа лучшего хода 
            if (!config("Bot", string("Is") + string((turn_num % 2) ? "Black" : "White") + string("Bot"))) // определяем ходит ли сейчас игрок (не ходит ли чейчас бот)
            {
                auto resp = player_turn(turn_num % 2); // записываем ход игрока в переменную resp
                if (resp == Response::QUIT) // если игрок нажал выход
                {
                    is_quit = true;
                    break;
                }
                else if (resp == Response::REPLAY) // если игрок нажал заново
                {
                    is_replay = true;
                    break;
                }
                else if (resp == Response::BACK) // если игрок нажал вернуть ход
                {
                    if (config("Bot", string("Is") + string((1 - turn_num % 2) ? "Black" : "White") + string("Bot")) &&
                        !beat_series && board.history_mtx.size() > 2)
                    {
                        board.rollback();
                        --turn_num;
                    }
                    if (!beat_series)
                        --turn_num;

                    board.rollback();
                    --turn_num;
                    beat_series = 0;
                }
            }
            else // если ходит бот
                bot_turn(turn_num % 2);
        }
        auto end = chrono::steady_clock::now(); // сохраняем время конца для рассчета времени игры
        ofstream fout(project_path + "log.txt", ios_base::app); // открываем файл лога
        fout << "Game time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n"; // записываем в файл лога время игры
        fout.close(); // закрываем файл лога
        if (is_replay) // если гажали переиграть
            return play();
        if (is_quit)   // если нажали выход
            return 0;
        int res = 2;
        if (turn_num == Max_turns) // если был достигнут лимит ходов
        {
            res = 0;
        }
        else if (turn_num % 2)
        {
            res = 1;
        }
        board.show_final(res); // вывести результат
        auto resp = hand.wait(); // лжтдаем нажатие от игрока
        if (resp == Response::REPLAY)
        {
            is_replay = true;
            return play();
        }
        return res;
    }

  private:
    void bot_turn(const bool color)
    {
        auto start = chrono::steady_clock::now();       // засекаем нвчало хода бота

        auto delay_ms = config("Bot", "BotDelayMS");    // Записываем настройку задержки хода для бота 
        thread th(SDL_Delay, delay_ms);                 // Включаем задержку в параллельном потоке, что бы в это время прозводился рассчет хода
        auto turns = logic.find_best_turns(color);      // Находим лучший ход или список ходов
        th.join();                                      // Ожидаем завершение задержки в параллельном потоке
        bool is_first = true;                           // Флаг первого хода в серии ходов
        // making moves
        for (auto turn : turns)                         // цикл проходит список ходов
        {
            if (!is_first)
            {
                SDL_Delay(delay_ms);
            }
            is_first = false;
            beat_series += (turn.xb != -1);             // Если было взятие увеличить счетчик взятых фигур
            board.move_piece(turn, beat_series);        // Производим ход
        }

        auto end = chrono::steady_clock::now();         // засекаем конец хода бота
        ofstream fout(project_path + "log.txt", ios_base::app); // открываем файл лога на запись
        fout << "Bot turn time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n"; // Записываем в файл лога время, которое бот потратил на ход
        fout.close(); // Закрываем файл
    }

    Response player_turn(const bool color)
    {
        // return 1 if quit
        vector<pair<POS_T, POS_T>> cells;           //
        for (auto turn : logic.turns)               //
        {                                           //  подсветка доступных для хода фигур
            cells.emplace_back(turn.x, turn.y);     //
        }                                           //
        board.highlight_cells(cells);               //
        move_pos pos = {-1, -1, -1, -1};
        POS_T x = -1, y = -1;
        // trying to make first move
        while (true) // бесконечный цикл обработки пользовательских действий
        {
            auto resp = hand.get_cell(); // возвращает ответ на пользовательское нажатие и записывает в resp вместе с координатами нажатия
            if (get<0>(resp) != Response::CELL) // если нажатие было не на игровую доску, то возвращаем событие
                return get<0>(resp);
            pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)}; // записываем на какую ячейку было нажатие

            bool is_correct = false; // флаг корректности хода
            for (auto turn : logic.turns) // просматриваем возможные ходы
            {
                if (turn.x == cell.first && turn.y == cell.second)
                {
                    is_correct = true;
                    break;
                }
                if (turn == move_pos{x, y, cell.first, cell.second})
                {
                    pos = turn;
                    break;
                }
            }
            if (pos.x != -1)
                break;
            if (!is_correct)
            {
                if (x != -1)
                {
                    board.clear_active();
                    board.clear_highlight();
                    board.highlight_cells(cells);
                }
                x = -1;
                y = -1;
                continue;
            }
            x = cell.first;
            y = cell.second;
            board.clear_highlight();
            board.set_active(x, y);
            vector<pair<POS_T, POS_T>> cells2;
            for (auto turn : logic.turns)
            {
                if (turn.x == x && turn.y == y)
                {
                    cells2.emplace_back(turn.x2, turn.y2);
                }
            }
            board.highlight_cells(cells2);
        }
        board.clear_highlight();
        board.clear_active();
        board.move_piece(pos, pos.xb != -1);
        if (pos.xb == -1) // если не было побитой фигуры, то возвращаем ОК
            return Response::OK;
        // continue beating while can
        beat_series = 1;
        while (true) // если побили фигуру, то пробуем побить еще
        {
            logic.find_turns(pos.x2, pos.y2);   // проверяем можем ли побить еще фигуру
            if (!logic.have_beats)              // если нет, то выходим из цикла
                break;

            vector<pair<POS_T, POS_T>> cells;
            for (auto turn : logic.turns)
            {
                cells.emplace_back(turn.x2, turn.y2);
            }
            board.highlight_cells(cells);
            board.set_active(pos.x2, pos.y2);
            // trying to make move
            while (true)
            {
                auto resp = hand.get_cell();
                if (get<0>(resp) != Response::CELL)
                    return get<0>(resp);
                pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)};

                bool is_correct = false;
                for (auto turn : logic.turns)
                {
                    if (turn.x2 == cell.first && turn.y2 == cell.second)
                    {
                        is_correct = true;
                        pos = turn;
                        break;
                    }
                }
                if (!is_correct)
                    continue;

                board.clear_highlight();
                board.clear_active();
                beat_series += 1;
                board.move_piece(pos, beat_series);
                break;
            }
        }

        return Response::OK;
    }

  private:
    Config config;
    Board board;
    Hand hand;
    Logic logic;
    int beat_series;
    bool is_replay = false;
};
