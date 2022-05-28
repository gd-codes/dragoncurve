
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>
#include <cstdlib>
#include <vector>
#include <mutex>
#include <thread>

#define sqrt3 1.7320508f
#define sin60 0.8660254f
#define cos60 0.5f

// Global constants
const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 1600;
const sf::Time GUI_REFRESH_RATE = sf::seconds(1 / 20.0f);
const sf::Vector2f windoworigin(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
const sf::Vector2f SCALE_FACTOR_0(1.25f, 1.25f);
const sf::Vector2f SCALE_FACTOR_1(0.80f, 0.80f);
int MAXORDER = 12; /* WARNING : Risk of segmentation fault for large values */
const int STEP = 5;
const sf::Color DARKGREY(32,32,32);

const std::vector<sf::Vector2f> dirs {
    sf::Vector2f(0, STEP),
    sf::Vector2f(STEP*sin60, STEP*cos60),
    sf::Vector2f(STEP*sin60, -STEP*cos60),
    sf::Vector2f(0, -STEP),
    sf::Vector2f(-STEP*sin60, -STEP*cos60),
    sf::Vector2f(-STEP*sin60, STEP*cos60),
};

// Coordinates of points to be drawn in window
// In global scope as it is used by both threads, builder and main
sf::VertexArray path(sf::LineStrip, 2);

std::mutex access_path_array;


sf::Vector2f turn(const bool& lr, int& dir, const sf::Vector2f& v) {
    sf::Vector2f a(v);
    switch (dir) {
        case 0: dir = lr ? 4 : 2; break;
        case 1: dir = lr ? 5 : 3; break;
        case 2: dir = lr ? 0 : 4; break;
        case 3: dir = lr ? 1 : 5; break;
        case 4: dir = lr ? 2 : 0; break;
        case 5: dir = lr ? 3 : 1; break;
    }
    a += dirs[dir];
    return a;
}


// Thread-1 : Generate the fractal
// Runs at unlimited rate
void builderthread() {
    int order = 0;
    int dir = 0;
    int tlen = 0, stage = 0;
    std::vector<bool> turns;
    std::vector<bool>::reverse_iterator iter_r = turns.rbegin();
    std::vector<bool>::iterator iter_f = turns.begin();
    std::vector<bool>::reverse_iterator rbegin = turns.rbegin();
    std::vector<bool>::iterator fbegin = turns.begin();
    std::vector<bool>::reverse_iterator rend = turns.rend();
    std::vector<bool>::iterator fend = turns.end();
    // Create the next segment of the fractal
    while (order < MAXORDER) {
        if (stage==0) {
            if (iter_r==rbegin) {
                turns.push_back(true);
                sf::Vertex v(turn(true, dir, path[path.getVertexCount()-1].position));
                path.append(v);
            }
            if (iter_r == rend) {
                stage=1;
            } else {
                turns.push_back(! *iter_r);
                sf::Vertex v(turn(! *iter_r, dir, path[path.getVertexCount()-1].position));
                path.append(v);
                iter_r++;
                tlen++;
            }
        } else {
            if (iter_f==fbegin) {
                turns.push_back(false);
                sf::Vertex v(turn(false, dir, path[path.getVertexCount()-1].position));
                path.append(v);
            }
            if (tlen == 0) {
                stage=0;
                turns.reserve(3*turns.size() + 2);
                iter_r = turns.rbegin(); iter_f = turns.begin();
                rbegin = turns.rbegin(); fbegin = turns.begin();
                rend = turns.rend(); fend = turns.end();
                order++;
                // turns.push_back(true);
            } else {
                turns.push_back(*iter_f);
                sf::Vertex v(turn(*iter_f, dir, path[path.getVertexCount()-1].position));
                path.append(v);
                iter_f++;
                tlen--;
            }
        }
    }
}


// Thread-0 : Update the window
// Refreshed according to GUI_REFRESH_RATE
int main(const int argc, const char* argv[]) {

    if (argc > 1) {
        int a = std::atoi(argv[1]);
        if (a>0 && a<20)
            MAXORDER=a;
    }

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
                            "Dragoncurve");

    path[0].position = sf::Vector2f(0,0);
    path[1].position = dirs[0];
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
                    center += (newPos - cursorPos) / float(scale);
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