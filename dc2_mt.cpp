
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>


// Global constants
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 1000;
const sf::Vector2f windoworigin(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
const sf::Time GUI_REFRESH_RATE = sf::seconds(1 / 20.0f);
const sf::Vector2f SCALE_FACTOR_0(1.25f, 1.25f);
const sf::Vector2f SCALE_FACTOR_1(0.80f, 0.80f);
const int MAXORDER = 20; /* WARNING : Risk of segmentation fault for large values */
const int STEP = 5;
const sf::Color DARKGREY(32,32,32);

// Coordinates of points to be drawn in window
// In global scope as it is used by both threads, builder and main
sf::VertexArray path(sf::LineStrip, 2);

std::mutex access_path_array;


sf::Vector2f turn(const bool& lr, char& dir, const sf::Vector2f& v) {
    sf::Vector2f a(v);
    const int sign = (lr)? 1 : -1;
    switch (dir) {
        case 'N': a += sf::Vector2f(-sign*STEP, 0); dir = lr?'W':'E'; break;
        case 'E': a += sf::Vector2f(0, sign*STEP); dir = lr?'N':'S'; break;
        case 'W': a += sf::Vector2f(0, -sign*STEP); dir = lr?'S':'N'; break;
        case 'S': a += sf::Vector2f(sign*STEP, 0); dir = lr?'E':'W'; break;
    }
    return a;
}


// Thread-1 : Generate the fractal
// Runs at unlimited rate
void builderthread() {
    int order = 0;
    char dir='E';
    std::vector<bool> turns;
    std::vector<bool>::reverse_iterator iter = turns.rend();
    std::vector<bool>::reverse_iterator rend = turns.rend();
    // Create the next segment of the fractal
    while (order < MAXORDER) {
        if (iter == rend) {
            turns.reserve(2*turns.size() + 1);
            iter = turns.rbegin();
            rend = turns.rend();
            turns.push_back(true);
            sf::Vertex v(turn(true, dir, 
                            path[path.getVertexCount()-1].position));
            path.append(v);
            order++;
        } else {
            turns.push_back(! *iter);
            sf::Vertex v(turn(! *iter, dir, 
                            path[path.getVertexCount()-1].position));
            path.append(v);
            iter++;
        }
    }
}


// Thread-0 : Update the window
// Refreshed according to GUI_REFRESH_RATE
int main() {

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
                            "Dragoncurve");

    path[0].position = sf::Vector2f(0,0);
    path[1].position = sf::Vector2f(STEP,0);
    // For interactiveness of display
    sf::Transform transform;
    sf::Vector2f cursorPos, 
                 center(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    transform.translate(windoworigin);
    bool isClicking = false;
    double scale = 1.0;
    
    // Begin creating the fractal
    std::thread thr1(builderthread);


    while (window.isOpen()) {

        // Event handling
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                thr1.join();
                return 0;
            } else if (event.type == sf::Event::MouseButtonPressed) 
                isClicking = true;
            else if (event.type == sf::Event::MouseButtonReleased) 
                isClicking = false;
            else if (event.type == sf::Event::MouseMoved) {
                sf::Vector2f newPos(event.mouseMove.x, event.mouseMove.y);
                if (isClicking) {
                    transform.translate((newPos - cursorPos) / float(scale));
                    center += (newPos - cursorPos);
                }
                cursorPos = newPos;
            }
            else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) {
                    transform.scale(SCALE_FACTOR_0, cursorPos-center);
                    scale *= SCALE_FACTOR_0.x;
                } else if (event.mouseWheelScroll.delta < 0) {
                    transform.scale(SCALE_FACTOR_1, cursorPos-center);
                    scale *= SCALE_FACTOR_1.x;
                }
            }
        }

        // Update GUI
        window.clear(DARKGREY);
        access_path_array.lock();
        window.draw(path, transform);
            sf::Vertex point((cursorPos-center+windoworigin), sf::Color::Magenta);
            window.draw(&point, 1, sf::Points);
        access_path_array.unlock();
        window.display();
        sf::sleep(GUI_REFRESH_RATE);
    }

    return 0;
}