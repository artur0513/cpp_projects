#pragma once

#include "Basics.h"
#include <SFML/Graphics.hpp>

enum class ConsoleMessageType {
	DEF, // По умолчанию
	INFO, // Просто информация
	WARN, // Предупреждение
	ERR, // Ошибка
	SUCC, // Сообщение об успехе
};

class Console {
private:
	const int max_console_size = 80;
	const sf::Color background_color = sf::Color(0, 0, 0, 70);
	const int character_size = 18;

	sf::Vector2f offset_koeff = sf::Vector2f(1.1f, 1.1f);
	sf::Vector2f avg_symbol_size;
	unsigned max_symbols_in_line;

	sf::Vector2u size;
	sf::Text text;
	sf::RenderTexture console_texture;

	sf::Color type_to_color(ConsoleMessageType type);

	Console(sf::Vector2u _size = sf::Vector2u(1080, 600));
	Console(const Console&);
	Console& operator=(const Console&) = delete;
	~Console();

public:
	sf::Font font;
	std::deque<std::pair<std::string, ConsoleMessageType>> messages; // Массив со всеми сообщениями в консоли
	sf::Sprite sprite;

	static Console* get_instance() {
		static Console instance;
		return &instance;
	}

	void log(std::string msg, ConsoleMessageType type = ConsoleMessageType::DEF); // Отправить сообщение в консоль

	void render();
};