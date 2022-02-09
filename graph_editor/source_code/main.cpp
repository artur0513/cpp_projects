#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <string>
#include <conio.h>
#include <fstream>

using namespace sf;
using namespace std;


void save_graph(vector<Vector2f> graph_points, string filename) {
    ofstream out_file(filename.c_str());
    if (out_file.is_open()) {
        int points_num = graph_points.size();
        for (int i = 0; i < points_num; i++) {
            out_file << graph_points[i].x << " " << graph_points[i].y << endl;
        }
    }
    out_file.close();
}

vector<Vector2f> load_graph(string filename) {
    ifstream in_file(filename.c_str());
    vector<Vector2f> graph_points;
    if (in_file.is_open()) {
        string line;
        while (getline(in_file, line)) {
            if (line[0] != '#') {
                int space = line.find_first_of(' '); // Вообще в строке должен быть только один пробел
                string s1 = line.substr(0, space);
                string s2 = line.substr(space + 1);
                Vector2f point;
                point.x = stof(s1);
                point.y = stof(s2);
                graph_points.push_back(point);
            }
        }
    }
    return graph_points;
}


void draw_line(RenderWindow& window, Vector2f start_point, Vector2f end_point, Color line_color = Color(255, 255, 255)) {
    Vertex line[] =
    {
        Vertex(start_point),
        Vertex(end_point)
    };
    line[0].color = line_color;
    line[1].color = line_color;

    window.draw(line, 2, Lines);
}

void draw_line(RenderWindow& window, float start_x, float start_y, float end_x, float end_y, Color line_color = Color(255, 255, 255)) {
    Vector2f start_point(start_x, start_y);
    Vector2f end_point(end_x, end_y);
    Vertex line[] =
    {
        Vertex(start_point),
        Vertex(end_point)
    };
    line[0].color = line_color;
    line[1].color = line_color;

    window.draw(line, 2, Lines);
}

string delete_zeros_from_string(string number) {
    bool is_float = false;
    for (int i = 0; i < size(number); i++) {
        if (number[i] == '.') {
            is_float = true;
        }
    }
    
    if (is_float == true) {
        while (number[size(number)-1] == '0') {
            number.pop_back();
        }
    }
    if (number[size(number) - 1] == '.') {
        number.pop_back();
    }
    if (size(number) == 2 && number[0] == '-' && number[1] == '0') {
        return "0";
    }
    return number;
}

int vector_sqare_length(Vector2i vec) {
    int length = vec.x * vec.x + vec.y * vec.y;
    return length;
}

int main()
{
    int cursor_counter = 0;
    int fps = 65;

    int screen_width = 1280;
    int screen_height = 720;
    string program_name = "graph editor";
    RenderWindow window(VideoMode(screen_width, screen_height), program_name);
    window.setFramerateLimit(fps);
    float scale = 500;
    Vector2f camera_pos(0, 0);
    vector<Vector2f> graph_points;
    vector<Vector2f> camera_points;
    Vector2i mouse_pos(0, 0);
    Vector2i prev_mouse_pos = mouse_pos;
    Vector2f vec(0.f, 0.f);
    int current_point = -1;
    bool right_last_state = false;
    int point_radius = 3;
    string current_command;

    bool parent_file_b = false;
    string parent_file;
    
    bool unsaved_changes = false;

    Font font;
    
    if (!font.loadFromFile("font.ttf")) {
        cout << "font not found!" << endl;
    }
    
    Text current_command_text;
    current_command_text.setFont(font);
    //current_command_text.setFillColor(Color(100, 100, 100));
    current_command_text.setPosition(Vector2f(50, screen_height - 40));

    graph_points.push_back(Vector2f(0, 0));
    graph_points.push_back(Vector2f(0.7, 0.3));

    while (window.isOpen())
    {
        float curr_screen_width = window.getSize().x;
        float curr_screen_height = window.getSize().y;

        // Сортируем точки графика по y координате
        std::sort(graph_points.begin(),
            graph_points.end(),
            [](const Vector2f& lhs, const Vector2f& rhs)
            {
                return lhs.x < rhs.x;
            });

        // Пересчитываем координаты точек графика в экранные координаты с учетом положения камеры и масштаба
        camera_points.clear();
        for (int i = 0; i < graph_points.size(); i++) {
            vec = (camera_pos - graph_points[i]) * scale;
            vec.x = -vec.x;
            vec.x += screen_width / 2;
            vec.y += screen_height / 2;
            camera_points.push_back(vec);
        }

        // Считаем шаг координатной сетки и все такое
        float max_y_coord = camera_pos.y + screen_height / (2 * scale);
        float min_y_coord = camera_pos.y - screen_height / (2 * scale);
        float delta_y = max_y_coord - min_y_coord;
        float h_lines_step = pow(10, floor(log10(delta_y))-1);
        int h_lines_count = (max_y_coord - min_y_coord) / h_lines_step + 1;
        while (h_lines_count > 20) {
            h_lines_step *= 2;
            h_lines_count = (max_y_coord - min_y_coord) / h_lines_step + 1;
        }
        float start_y_coord = ceil(min_y_coord / h_lines_step) * h_lines_step;

        // Считаем шаг координатной сетки и все такое
        float max_x_coord = camera_pos.x + screen_width / (2 * scale);
        float min_x_coord = camera_pos.x - screen_width / (2 * scale);
        float delta_x = max_x_coord - min_x_coord;
        float v_lines_step = pow(10, floor(log10(delta_x))-1);
        int v_lines_count = (max_x_coord - min_x_coord) / v_lines_step + 1;
        while (v_lines_count > 20) {
            v_lines_step *= 2;
            v_lines_count = (max_x_coord - min_x_coord) / v_lines_step + 1;
        }
        float start_x_coord = ceil(min_x_coord / v_lines_step) * v_lines_step;

        // Смотрим на какую точку наведена мышка и запоминаем ее номер
        mouse_pos = Mouse::getPosition(window);
        mouse_pos.x = mouse_pos.x * screen_width / curr_screen_width;
        mouse_pos.y = mouse_pos.y * screen_height / curr_screen_height;
        current_point = -1;
        for (int i = 0; i < size(camera_points); i++) {
            Vector2i vec = Vector2i(camera_points[i]) - mouse_pos;
            if (vector_sqare_length(vec) < 200) {
                //cout << vector_sqare_length(vec) << endl;
                current_point = i;
            }
        }
        // Если зажата ПКМ, то можем двигать камеру
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            camera_pos.x = camera_pos.x - (mouse_pos.x - prev_mouse_pos.x) / scale;
            camera_pos.y = camera_pos.y + (mouse_pos.y - prev_mouse_pos.y) / scale;
        }
        // Если нажали ЛКМ, то создаем новую точку
        if (Mouse::isButtonPressed(Mouse::Right) && right_last_state == false) {
            float new_point_x = camera_pos.x + (mouse_pos.x - screen_width / 2) / scale;
            float new_point_y = camera_pos.y - (mouse_pos.y - screen_height / 2) / scale;
            Vector2f new_point(new_point_x, new_point_y);
            graph_points.push_back(new_point);
            unsaved_changes = true;
        }
        right_last_state = Mouse::isButtonPressed(Mouse::Right);
        prev_mouse_pos = mouse_pos;

        // Обработка всяких событий
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseWheelMoved)
            {
                scale *= pow(1.05, event.mouseWheel.delta);//event.mouseWheel.delta равен 1 или -1 в зависимости от 
                //направления кручения колесика мыши
            }
            if (event.type == Event::KeyPressed) {
                if (Keyboard::isKeyPressed(Keyboard::Delete) && current_point > -1) {
                    graph_points.erase(graph_points.begin() + current_point);
                    unsaved_changes = true;
                    //cout << "deleted???" << endl;
                }
                if (Keyboard::isKeyPressed(Keyboard::Backspace)) {
                    if (current_command.size() > 0) {
                        current_command.pop_back();
                    }
                }
                if (Keyboard::isKeyPressed(Keyboard::LControl) && Keyboard::isKeyPressed(Keyboard::S)) {
                    if (parent_file_b == true) {
                        save_graph(graph_points, parent_file);
                        unsaved_changes = false;
                    }
                }
                if (Keyboard::isKeyPressed(Keyboard::Enter)) {
                    // Добавляем в конец строки пробел
                    if (current_command[current_command.size() - 1] != ' ') {
                        current_command += ' ';
                    }
                    // Удаляем из строки лишние пробелы (где их 2 и более подряд)
                    int i = 0;
                    while (i < current_command.size()-1) {
                        if (current_command[i] == ' ' && current_command[i + 1] == ' ') {
                            current_command.erase(i, 1);
                        }
                        else {
                            i++;
                        }
                    }
                    // Считываем саму команду и атрибуты
                    int space = current_command.find_first_of(' ');
                    string command = current_command.substr(0, space);
                    vector<string> attributes;
                    while (current_command.find_first_of(' ', space + 1) != string::npos) {
                        int substr_len = current_command.find_first_of(' ', space + 1) - space;
	                    string attribute = current_command.substr(space + 1, substr_len);
                        space = current_command.find_first_of(' ', space + 1);
                        attributes.push_back(attribute);
                    }

                    // При необходимости выводим то, что считала программа
                    /*
                    cout << "program get command: " << command << endl;
                    cout << "with following attributes: " << endl;
                    for (int i = 0; i < attributes.size(); i++) {
                        cout << "attribute #" << i << ": " << attributes[i] << endl;
                    }
                    */

                    // Здесь обрабатываем команды
                    if (command == "save") {
                        save_graph(graph_points, attributes[0]);
                        unsaved_changes = false;
                    }

                    if (command == "load") {
                        graph_points = load_graph(attributes[0]);
                        parent_file = attributes[0];
                        parent_file_b = true;
                        window.setTitle(program_name + " - " + parent_file);
                        unsaved_changes = false;
                    }

                    if (command == "clear") {
                        graph_points.clear();
                        unsaved_changes = true;
                    }

                    // Обнуляем переменные
                    current_command.clear();
                    attributes.clear();
                    
                }
            }
            else if (event.type == sf::Event::TextEntered)
            {
                // отсекаем не ASCII символы
                if (event.text.unicode < 127 && event.text.unicode > 31)
                {
                    //cout << "symbol entered: " << event.text.unicode << endl;
                    current_command += static_cast<char>(event.text.unicode);
                }
            }
        }

        window.clear();
        
        window.draw(current_command_text);
        // Рисуем сетку и циферки на ней по оси y
        float y_coord = start_y_coord;
        for (int i = 0; i < h_lines_count; i++) {
            float screen_y = (- y_coord + camera_pos.y) * scale + screen_height / 2;

            if (abs(y_coord) < h_lines_step/2) {
                draw_line(window, 0, screen_y, screen_width, screen_y, Color(255, 60, 0));
            }
            else {
                draw_line(window, 0, screen_y, screen_width, screen_y, Color(60, 120, 0));
            }
            
            Text text;
            text.setFont(font);
            String num = to_string(y_coord);
            num = delete_zeros_from_string(num);
            text.setString(num);
            text.setCharacterSize(14);
            text.setPosition(0, screen_y);
            window.draw(text);

            y_coord += h_lines_step;
        }
        // Рисуем сетку и циферки на ней по оси x
        float x_coord = start_x_coord;
        for (int i = 0; i < v_lines_count; i++) {
            float screen_x = (x_coord - camera_pos.x) * scale + screen_width / 2;
            if (abs(x_coord) < v_lines_step / 2) {
                draw_line(window, screen_x, 0, screen_x, screen_height, Color(255, 60, 0));
            }
            else {
                draw_line(window, screen_x, 0, screen_x, screen_height, Color(60, 120, 0));
            }

            Text text;
            text.setFont(font);
            String num = to_string(x_coord);
            num = delete_zeros_from_string(num);
            text.setString(num);
            text.setCharacterSize(14);
            text.setPosition(screen_x, 0);
            window.draw(text);

            x_coord += v_lines_step;
        }
        // Рисуем линии самого графика и кружки на месте точек
        for (int i = 1; i < camera_points.size(); i++) {
            draw_line(window, camera_points[i], camera_points[i-1]);
        }
        for (int i = 0; i < camera_points.size(); i++) {
            CircleShape circle;
            circle.setRadius(point_radius);
            circle.setOutlineColor(Color(20, 20, 20));
            circle.setOutlineThickness(2);
            circle.setPosition(camera_points[i].x - point_radius, camera_points[i].y - point_radius);
            window.draw(circle);
        }

        Text text;
        text.setFont(font);
        text.setString("you have unsaved changes");
        text.setCharacterSize(20);
        text.setPosition(50, screen_height - 65);
        if (unsaved_changes == true) {
            window.draw(text);
        }

        cursor_counter += 1;
        cursor_counter %= fps;
        if (cursor_counter < fps / 2) {
            current_command_text.setString(current_command + "|");
        }
        else {
            current_command_text.setString(current_command);
        }


        window.display();
        
    }

    return 0;
}
