#ifndefined SNAKE_HPP
#define SNAKE_HPP

#include <deque>
#include <SFML/Graphics.hpp>

enum class Direction {
    Up, Down, Left, Right
};

class Snake {
    public:
        Snake(int gridWidth, int gridHeight);

#endif