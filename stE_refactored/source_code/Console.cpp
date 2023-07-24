#pragma once
#include "Console.h"

sf::Color Console::type_to_color(ConsoleMessageType type) { // Тип ошибки превращает в цвет
	switch (type) {
	case ConsoleMessageType::DEF:
		return sf::Color(255, 255, 255, 255);
	case ConsoleMessageType::INFO:
		return sf::Color(130, 130, 130, 255);
	case ConsoleMessageType::WARN:
		return sf::Color(255, 230, 0, 255);
	case ConsoleMessageType::ERR:
		return sf::Color(200, 0, 0, 255);
	case ConsoleMessageType::SUCC:
		return sf::Color(0, 170, 0, 255);
	}

	return sf::Color(255, 255, 255, 255); // На всякий случай
}

Console::Console(sf::Vector2u _size) {
	size = _size;
	if (!font.loadFromFile("resources/console/font.ttf")) {
		std::cout << "Unable to find font.ttf for in-game console!" << std::endl;
	}
	else if (!console_texture.create(sf::Vector2u(size.x, size.y))) {
		std::cout << "Unable to create texture for console!" << std::endl;
	}
	else
		std::cout << "In-game console created successfully, all further info you can see in it" << std::endl;
	text.setFont(font);
	text.setCharacterSize(character_size);
	sprite.setTexture(console_texture.getTexture());


	// Шрифт должен быть monospaced, ниже идет вычисление средней длины символа шрифта
	std::string test = "!#$ % &()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	text.setString(test);
	avg_symbol_size.x = offset_koeff.x * float(text.getGlobalBounds().width) / float(test.size());
	avg_symbol_size.y = offset_koeff.y * float(text.getGlobalBounds().height);
	max_symbols_in_line = float(size.x) / avg_symbol_size.x;

	log("Welcome to stEngine console. Today is " + get_date_string() + ", hope you don't encounter any bugs :)");
	log("Some error will not be printed in this console, you can find them in logs\\ directory in your game folder");
}

void Console::log(std::string msg, ConsoleMessageType type) {
	if (messages.size() >= max_console_size)
		messages.pop_front();

	msg = "[" + get_time_string() + "] " + msg;

	// Если текст нового сообщения совпадает с предыдущим сообщением, то не будем их повторять
	if (messages.size() > 0)
		if (msg == messages[messages.size() - 1].first)
			return;

	messages.push_back(std::pair<std::string, ConsoleMessageType>(msg, type));
	std::cout << msg << std::endl;

	render();
}

void Console::render() {
	if (max_symbols_in_line == 0)
		return;

	console_texture.clear(background_color);
	int sum_height = size.y, msg_counter = messages.size() - 1;
	int lines = 0;

	while (sum_height > 0 && msg_counter >= 0) {
		text.setFillColor(type_to_color(messages[msg_counter].second));
		for (int i = messages[msg_counter].first.size() / max_symbols_in_line; i >= 0; i--) {

			text.setString(messages[msg_counter].first.substr(i * max_symbols_in_line, max_symbols_in_line));
			lines++;
			sum_height = size.y - lines * int(avg_symbol_size.y);
			text.setPosition(sf::Vector2f(0, float(sum_height)));
			console_texture.draw(text);
		}
		msg_counter--;
	}
	console_texture.display();
}

Console::~Console() {}