#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <filesystem>
#include "Console.h"
#include "Basics.h"

// Класс-менеджер для хранения текстур и шейдеров, стремится минимизировать попытки загрузки дубликатов текстур/шейдеров и просто для удобства
template<class T>
class Resource_manager {
private:
	std::map<std::string, T*> resources;

	Console* con = Console::get_instance();

	Resource_manager() {};
	Resource_manager(const Resource_manager& r) {};
	Resource_manager operator=(const Resource_manager& r) {};
	Resource_manager(Resource_manager&& r) {};
	Resource_manager& operator=(const Resource_manager&& r) {};

	bool load_object_from_file(T* obj, std::string filename); // Нужно самостоятельно переопределять эту функцию загрузки под тип T
public:
	static Resource_manager* get_instance() {
		static Resource_manager instance;
		return &instance;
	}

	void preload(std::string filename) {
		std::ifstream file(filename.c_str());
		if (file.is_open()) {
			std::string object_filename;
			while (getline(file, object_filename)) { // Загружаем название файла обьекта
				if (object_filename.size() == 0) // Если размер строки 0
					continue;
				else if (object_filename[object_filename.find_first_not_of(" ")] == '#') // Или она начинается с символа для комментария
					continue; // То переходим к следующей строке

				object_filename = std::filesystem::relative(object_filename).string(); // Приводм название файла к единому виду
				T* new_object = new T; // Создаем указатель на обьект

				if (!load_object_from_file(new_object, object_filename)) // Пробуем грузить из файла
					delete new_object; // Если не удалось, то удаляем и идем дальше
				else {
					resources.insert(std::pair<std::string, T*>(object_filename, new_object)); // Иначе закидываем в карту и тоже идем дальше
				}
			}
		}
		else {
			std::cout << "cannot load objects, file '" << filename << "' not found!" << std::endl;
		}
		file.close();
	}

	T* get_object(std::string filename) {
		filename = std::filesystem::relative(filename).string(); // Приводм название файла к единому виду
		auto itrtr = resources.find(filename);

		if (itrtr == resources.end()) { // Если нужного обьекта нету, то попробуем его найти, но вообще лучше сразу сначала грузить
			T* new_object = new T; // Создаем указатель на обьект

			if (!load_object_from_file(new_object, filename)) { // Пробуем грузить из файла
				return nullptr;
			}
			else {
				resources.insert(std::pair<std::string, T*>(filename, new_object)); // Иначе закидываем в карту
				return new_object; // И возвращаем указатель на новый обьект
			}
		}
		else // Если обьект все же нашли, то возвращаем на него указатель
			return itrtr->second;
	}

	void print() {
		con->log("All objects loaded in " + get_message_prefix(this), ConsoleMessageType::INFO);
		for (auto& r : resources)
			con->log(r.first, ConsoleMessageType::INFO);
		con->log("End of Resource_manager info", ConsoleMessageType::INFO);
	}

	~Resource_manager() {
		for (const auto& t : resources)
			delete t.second;
	}
};
