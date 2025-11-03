#pragma once
#include <tuple>

#include "../Models/Move.h"
#include "../Models/Response.h"
#include "Board.h"

// methods for hands
class Hand
{
  public:
    Hand(Board *board) : board(board)
    {
    }
    tuple<Response, POS_T, POS_T> get_cell() const // функция обработке нажатия, возвращает ответ на нажатие и координаты активных элементов
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        int x = -1, y = -1;
        int xc = -1, yc = -1;
        while (true) // бесконечный цикл для обработки кликов, выполняется пока не произойдет нажатие на активный элемент
        {
            if (SDL_PollEvent(&windowEvent)) // Если происходит событие, то ...
            {
                switch (windowEvent.type) // условие выполнения в зависимости от типа событий
                {
                case SDL_QUIT: // нажатие на крестик (выход)
                    resp = Response::QUIT;
                    break;
                case SDL_MOUSEBUTTONDOWN: // нажатие на кнопку мыши внутри окна
                    x = windowEvent.motion.x;
                    y = windowEvent.motion.y;
                    xc = int(y / (board->H / 10) - 1);
                    yc = int(x / (board->W / 10) - 1);
                    if (xc == -1 && yc == -1 && board->history_mtx.size() > 1) // нажатие на кнопку назад и выполнен хотя бы один ход
                    {
                        resp = Response::BACK;
                    }
                    else if (xc == -1 && yc == 8) // нажатие на кнопку заново
                    {
                        resp = Response::REPLAY;
                    }
                    else if (xc >= 0 && xc < 8 && yc >= 0 && yc < 8) // нажатие на игровую доску
                    {
                        resp = Response::CELL;
                    }
                    else // нажатие на неактивные области внутри окна, возвращаем значения по умолчанию
                    {
                        xc = -1;
                        yc = -1;
                    }
                    break;
                case SDL_WINDOWEVENT: // изменение размера окна
                    if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        board->reset_window_size();
                        break;
                    }
                }
                if (resp != Response::OK) // условие выхода из бесконечного цикла
                    break;
            }
        }
        return {resp, xc, yc};
    }

    Response wait() const // то же что и get_cell(), но без обработки нажатия на игровую доску и возврата хода, вызывается в конце игры
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        while (true)
        {
            if (SDL_PollEvent(&windowEvent))
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT:
                    resp = Response::QUIT;
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    board->reset_window_size();
                    break;
                case SDL_MOUSEBUTTONDOWN: {
                    int x = windowEvent.motion.x;
                    int y = windowEvent.motion.y;
                    int xc = int(y / (board->H / 10) - 1);
                    int yc = int(x / (board->W / 10) - 1);
                    if (xc == -1 && yc == 8)
                        resp = Response::REPLAY;
                }
                break;
                }
                if (resp != Response::OK)
                    break;
            }
        }
        return resp;
    }

  private:
    Board *board;
};
