#include <iostream>
#include <string>
#include <cmath>
#include <future>
#include <thread>
#include <SFML/Graphics.hpp>
#include "3dMath/3dMath.h"

m3d::vec3d refraction(m3d::vec3d q1, m3d::vec3d n, double n1, double n2) {
    double dot = m3d::dot(q1, n);
    m3d::vec3d q2 = q1 + (sqrt((n2 * n2 - n1 * n1) / (dot * dot) + 1) - 1) * dot * n;
    return q2;
}

m3d::vec3d intersection(m3d::vec3d s, m3d::vec3d q, m3d::vec3d c, double r) { // start, dir, sphere center, radius
    auto a = s - c;

    double det = sqrt(m3d::dot(q, a) * m3d::dot(q, a) - m3d::norm(q) * (m3d::norm(a) - r * r));

    double root1 = (-m3d::dot(q, a) + det) / m3d::norm(q);
    double root2 = (-m3d::dot(q, a) - det) / m3d::norm(q);

    if (root1 < 0 && root2 < 0)
        throw("no intersection!");
    else if (root1 > 0 && root2 < 0)
        return s + q * root1;
    else if (root1 < 0 && root2 > 0)
        return s + q * root2;
    else
        return s + q * fmin(root1, root2);
}

struct Lens {
    const double n = 1.5;

    double r1, r2, r;
    m3d::vec3d c1, c2;

    Lens(double _r1, double _r2, double _r) {
        r1 = _r1;
        r2 = _r2;
        r = _r;

        c1 = { 0, 0, sqrt(r1 * r1 - r * r) };
        if (r1 < 0)
            c1 = -1.0 * c1;

        c2 = { 0, 0, -sqrt(r2 * r2 - r * r) };
        if (r2 < 0)
            c2 = -1.0 * c2;
    }

    double getFocus() {
        return 1.0 / (1 / r1 + 1 / r2) / (n - 1.0);
    }

    std::pair<m3d::vec3d, m3d::vec3d> refract(m3d::vec3d start, m3d::vec3d dir) {
        try {
            m3d::vec3d intersection1 = intersection(start, dir, c1, r1);
            m3d::vec3d n1 = m3d::normalize(intersection1 - c1);

            m3d::vec3d q = refraction(dir, n1, 1.0, n);
            m3d::vec3d intersection2 = intersection(intersection1, q, c2, r2);
            m3d::vec3d n2 = m3d::normalize(intersection2 - c2);
            return std::pair<m3d::vec3d, m3d::vec3d>(refraction(q, n2, n, 1.0), intersection2);
        }
        catch (...) {
            return std::pair<m3d::vec3d, m3d::vec3d>({ 0.0, 0.0, 1.0 }, { 1000.0, 1000.0, 0.0 });
        }
    }

};

int main()
{
    double x = 8.0, y = 8.0;
    unsigned pixelX = 1000, pixelY = 1000;

    sf::Image image;
    std::vector<uint8_t> img(pixelX * pixelY * 4, 0);
    image.create(sf::Vector2u(pixelX, pixelY), img.data());
    img.clear();

    auto clearImage = [&]() {
        for (unsigned i = 0; i < pixelX * pixelY; i++)
            image.setPixel(sf::Vector2u(i / pixelX, i % pixelX), sf::Color(0, 0, 0, 255));
    };

    auto addToPixel = [&](m3d::vec3d pos) {
        pos += {x, y, 0.0};
        sf::Vector2u imgpos;
        imgpos.x = unsigned(pos.x / 2.0 / x * double(pixelX));
        imgpos.y = unsigned(pos.y / 2.0 / y * double(pixelY));

        if (imgpos.x < image.getSize().x && imgpos.y < image.getSize().y)
            if (image.getPixel(imgpos).r < 255)
                image.setPixel(imgpos, sf::Color(1, 1, 1, 0) + image.getPixel(imgpos));
    };

    Lens lens(10.0, 15.0, 4.0);
    std::cout << "Focus: " << lens.getFocus() << "\n";
    m3d::vec3d object = { 0.0, 0.0, -2.0 * lens.getFocus() };
    double Fwatch = 2.0 * lens.getFocus();

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1000, 1000)), "My window");
    window.setFramerateLimit(60);

    const double c = 0.01;
    unsigned rayCount = 0;
    sf::Clock clock;

    unsigned num_threads = std::thread::hardware_concurrency();
    double phiPerThread = 2.0 * 3.1415926535 / double(num_threads);

    double rStep = lens.r / 400.0, phiConst = 0.0002;
    unsigned raysPerThread = 0;
    for (double r = 0.0; r < lens.r; r += rStep) {
        double phiStep = phiConst / (r + rStep) / rStep;
        for (double phi = 0.0; phi < phiPerThread * 1.0; phi += phiStep) {
            raysPerThread++; // very dumb way to calculate raysPerThread
        }
    }

    std::cout << "threads: " << num_threads << "\n";
    std::cout << "Rays calculated: " << raysPerThread * num_threads << "\n";

    auto recalc = [&]() {
        clock.restart();
        clearImage();

        std::vector<std::future<m3d::vec3d*>> futures;

        auto simulateRay = [&](double phiMin, double phiMax) {
            unsigned counter = 0;
            m3d::vec3d* results = new m3d::vec3d[raysPerThread];
            for (double r = 0.0; r < lens.r; r += rStep) {
                double phiStep = phiConst / (r + rStep) / rStep;
                for (double phi = phiMin; phi < phiMax; phi += phiStep) {
                    auto newdir = lens.refract(object, m3d::normalize(m3d::vec3d(r * cos(phi), r * sin(phi), 0.0) - object));
                    newdir.first *= Fwatch / newdir.first.z;
                    if (counter < raysPerThread)
                        results[counter] = newdir.first + newdir.second;
                    counter++;
                }
            }
            return results;
        };

        for (unsigned i = 0; i < num_threads; i++) {
            futures.push_back(std::async(std::launch::async, simulateRay, phiPerThread * double(i), phiPerThread * double(i + 1)));
        }

        for (unsigned j = 0; j < futures.size(); j++) {
            futures[j].wait();
            auto res = futures[j].get();
            for (unsigned i = 0; i < raysPerThread; i++) {
                addToPixel(res[i]);
            }
            delete[] res;
        }


        window.clear(sf::Color(50, 0, 0, 255));
        sf::Texture texture;
        texture.loadFromImage(image);
        auto sprite = sf::Sprite(texture);
        sprite.setScale(sf::Vector2f(1.f, 1.f));
        window.draw(sprite);
        window.display();
        std::cout << "Milliseconds to render: " << clock.getElapsedTime().asMilliseconds() << "\n";
        rayCount = 0;
    };

    recalc();
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Right) {
                    Fwatch += 0.15;
                    std::cout << "Fwatch / lens.getFocus(): " << Fwatch / lens.getFocus() << "\n";
                    recalc();
                }
                if (event.key.code == sf::Keyboard::Left) {
                    Fwatch -= 0.15;
                    std::cout << "Fwatch / lens.getFocus(): " << Fwatch / lens.getFocus() << "\n";
                    recalc();
                }
            }
        }
    }

    image.saveToFile("1.tga");
    return 0;
}
