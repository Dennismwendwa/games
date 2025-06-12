#include <SFML/Graphics.hpp>
#include <deque>
#include <iostream>
#include <random>
#include <string>
#include <SFML/Audio.hpp>


const int cellSize = 20;
const int width = 40;
const int height = 30;

const int windowWidth = cellSize * width;
const int windowHeight = cellSize * height;

enum class Direction {Up, Down, Left, Right};

sf::Vector2i directionToVector(Direction dir) {
    switch (dir) {
        case Direction::Up:
            return {0, -1};
        case Direction::Down:
            return {0, 1};
        case Direction::Left:
            return {-1, 0};
        case Direction::Right:
            return {1, 0};

    }
    return {0, 0};
}


bool isOutOfBounds(const sf::Vector2i& pos) {
    return pos.x < 0 || pos.y < 0 || pos.x >= width || pos.y >= height;
}

bool checkSelfCollision(const std::deque<sf::Vector2i>& snake) {
    const sf::Vector2i& head = snake.front();

    for (size_t k = 1; k < snake.size(); ++k) {
        if (snake[k] == head) return true;
    }
    return false;
}

sf::Vector2i getRandomPosition(const std::deque<sf::Vector2i>& snake, std::mt19937& rng) {
    std::uniform_int_distribution<int> xDist(0, width - 1);
    std::uniform_int_distribution<int>yDist(0, height - 1);

    sf::Vector2i food;
    bool valid = false;

    while (!valid) {
        food.x = xDist(rng);
        food.y = yDist(rng);
        valid = true;

        for (const auto& segment : snake) {
            if (segment == food) {
                valid == false;
                break;
            }
        }
    }
    return food;
}

Direction getRandomDirection(std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, 3);
    return static_cast<Direction>(dist(rng));
}

sf::Music bgMusic;

int main() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::string basePath = ASSET_PATH;

    sf::SoundBuffer eatBuffer, gameOverBuffer;
    if (!eatBuffer.loadFromFile(basePath + "/music/gameover.ogg") || !gameOverBuffer.loadFromFile(basePath + "/music/gameover.ogg")) {
        return -1;
    }
    sf::Sound eatSound(eatBuffer);
    sf::Sound gameOverSound(gameOverBuffer);

    // loading the background music
    static sf::Music bgMusic;
    if (!bgMusic.openFromFile(basePath + "/music/gameover.ogg")) {
        std::cerr << "Failed to load background music\n";
        return -1;
    }
    bgMusic.setLoop(true);
    bgMusic.play();
    std::cout << "Music status: " << bgMusic.getStatus() << "\n";
    int baseSpeed = 5;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Snake Game");
    window.setFramerateLimit(5);

    sf::Font font;
    font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 5);

    std::deque<sf::Vector2i> snake;
    snake.push_back({width / 2, height / 2});

    Direction dir = getRandomDirection(rng);
    bool grow = false;

    sf::Vector2i food = getRandomPosition(snake, rng);
    int score{};
    bool gameOver = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            int speed = baseSpeed + (score / 5);
            window.setFramerateLimit(speed);

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up && dir != Direction::Down) dir = Direction::Up;
                if (event.key.code == sf::Keyboard::Down && dir != Direction::Up) dir = Direction::Down;
                if (event.key.code == sf::Keyboard::Left && dir != Direction::Right) dir = Direction::Left;
                if (event.key.code == sf::Keyboard::Right && dir != Direction::Left) dir = Direction::Right;
            }

            if (event.type == sf::Event::KeyPressed && gameOver) {
                if (event.key.code == sf::Keyboard::R) {
                    snake.clear();
                    snake.push_back({width / 2, height / 3});
                    dir = Direction::Right;
                    food = getRandomPosition(snake, rng);
                    gameOver = false;
                }
            }
        }

        if (!gameOver) {
            sf::Vector2i head = snake.front();
            head += directionToVector(dir);

            if (isOutOfBounds(head) || checkSelfCollision(snake)) {
                gameOver = true;
                gameOverSound.play();
                continue;
            }

            snake.push_front(head);

            if (head == food) {
                food = getRandomPosition(snake, rng);
                score++;
                eatSound.play();
            } else {
                snake.pop_back();
            }
            scoreText.setString("Score: " + std::to_string(score));
        }


        window.clear(sf::Color::Black);

        // Drawing food
        sf::RectangleShape foodRect(sf::Vector2f(cellSize - 2, cellSize - 2));
        foodRect.setFillColor(sf::Color::Red);
        foodRect.setPosition(food.x * cellSize, food.y * cellSize);
        window.draw(foodRect);

        // Drawing snake
        for (const auto& segment : snake) {
            sf::RectangleShape rect(sf::Vector2f(cellSize - 2, cellSize - 2));
            rect.setFillColor(sf::Color::Green);
            rect.setPosition(segment.x * cellSize, segment.y * cellSize);
            window.draw(rect);
        }

        // Drawing score
        //std::cout << "Score: " << scoreText << std::endl;
        window.draw(scoreText);

        // Drawing game over
        if (gameOver) {
            sf::Font font;

            if (font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")) {
                sf::Text text("Game Over\nPress R to Restart", font, 24);
                text.setFillColor(sf::Color::White);
                text.setPosition(100, 200);
                window.draw(text);
                score = 0;
            }
        }
        //
        window.display();
    }

    return 0;
}