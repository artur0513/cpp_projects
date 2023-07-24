#pragma once
#include "ResourceManager.h"
#include <SFML/Graphics.hpp>

template<> bool Resource_manager<sf::Texture>::load_object_from_file(sf::Texture* new_texture, std::string filename) {
	return new_texture->loadFromFile(filename);
}

template<> bool Resource_manager<sf::Shader>::load_object_from_file(sf::Shader* new_shader, std::string filename) {
	return new_shader->loadFromFile(filename, sf::Shader::Fragment);
}
