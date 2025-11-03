#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config
{
  public:
    Config() // Конструктор
    {
        reload();
    }

    void reload() // Функция загружает настройки из файла settings.json в объект settings
    {
        std::ifstream fin(project_path + "settings.json");  // открываем файл settings.json для чтения
        fin >> config;                                      // записываем текст из файла в объект config
        fin.close();                                        // закрываем файл settings.json
    }

    // operator() необходим, чтобы обращаться к свойствам объекта по ключу (getter для приватного объекта json)
    auto operator()(const string &setting_dir, const string &setting_name) const
    {
        return config[setting_dir][setting_name];
    }

  private:
    json config;
};
