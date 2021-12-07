
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>


const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 1000;
const sf::Vector2f SCALE_FACTOR_0(1.1f, 1.1f);
const sf::Vector2f SCALE_FACTOR_1(0.9f, 0.9f);
const int MAXORDER = 25;
const int STEP = 5;
const sf::Color DARKGREY(32,32,32);


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


int main() {

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
                            "Dragoncurve");
    // Record the structure of the fractal as it is being generated
    sf::VertexArray path(sf::LineStrip, 2);
    path[0].position = sf::Vector2f(0,0);
    path[1].position = sf::Vector2f(STEP,0);
    std::vector<bool> turns;
    std::vector<bool>::reverse_iterator iter = turns.rend();
    std::vector<bool>::reverse_iterator rend = turns.rend();
    
    // For interactiveness of display
    sf::Transform transform;
    transform.translate(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    sf::Vector2f cursorPos, center(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    bool isClicking = false;
    int order = 0;
    char dir='E';

    while (window.isOpen()) {

        // Create the next segment of the fractal
        if (order < MAXORDER) {
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

        // Event handling
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed) 
                isClicking = true;
            else if (event.type == sf::Event::MouseButtonReleased) 
                isClicking = false;
            else if (event.type == sf::Event::MouseMoved) {
                sf::Vector2f newPos(event.mouseMove.x, event.mouseMove.y);
                if (isClicking) {
                    transform.translate(newPos - cursorPos);
                    center += newPos - cursorPos;
                }
                cursorPos = newPos;
            }
            else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0)
                    transform.scale(SCALE_FACTOR_0, center);
                else if (event.mouseWheelScroll.delta < 0)
                    transform.scale(SCALE_FACTOR_1, center);
            }
        }

        // Update GUI
        window.clear(DARKGREY);
        window.draw(path, transform);
        window.display();
    }

    return 0;
}