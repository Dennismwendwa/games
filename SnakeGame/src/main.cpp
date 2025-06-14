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

enum class Direction { Up, Down, Left, Right };
enum class GameState { MENU, PLAYING, PAUSED, GAME_OVER };

sf::Vector2i directionToVector(Direction dir) {
    switch (dir) {
        case Direction::Up: return {0, -1};
        case Direction::Down: return {0, 1};
        case Direction::Left: return {-1, 0};
        case Direction::Right: return {1, 0};
    }
    return {0, 0};
}

bool isOutOfBounds(const sf::Vector2i& pos) {
    return pos.x < 0 || pos.y < 0 || pos.x >= width || pos.y >= height;
}

bool checkSelfCollision(const std::deque<sf::Vector2i>& snake) {
    const sf::Vector2i& head = snake.front();
    for (size_t k = 1; k < snake.size(); ++k)
        if (snake[k] == head) return true;
    return false;
}

sf::Vector2i getRandomPosition(const std::deque<sf::Vector2i>& snake, std::mt19937& rng) {
    std::uniform_int_distribution<int> xDist(0, width - 1);
    std::uniform_int_distribution<int> yDist(0, height - 1);

    sf::Vector2i food;
    bool valid = false;
    while (!valid) {
        food = {xDist(rng), yDist(rng)};
        valid = true;
        for (const auto& segment : snake) {
            if (segment == food) {
                valid = false;
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

int main() {
    std::random_device rd;
    std::mt19937 rng(rd());

    std::string basePath = ASSET_PATH;

    sf::Vector2i bonusFood;
    bool bonusFoodVisible = false;
    int bonusFoodTimer = 0;
    const int bonusFoodDuration = 100;
    const int bonusFoodCooldown = 200;
    int bonusCooldownTimer = bonusFoodCooldown;

    sf::Texture bonusFoodTex;
    sf::Sprite bonusFoodSprite;

    sf::SoundBuffer eatBuffer, gameOverBuffer;
    if (!eatBuffer.loadFromFile(basePath + "/music/eat.ogg") ||
        !gameOverBuffer.loadFromFile(basePath + "/music/gameover.ogg")) {
        return -1;
    }

    sf::Sound eatSound(eatBuffer);
    sf::Sound gameOverSound(gameOverBuffer);

    sf::Music bgMusic;
    if (!bgMusic.openFromFile(basePath + "/music/background.wav")) {
        std::cerr << "Failed to load background music\n";
        return -1;
    }
    bgMusic.setLoop(true);

    sf::Texture backgroundTex, headTex, bodyTex, foodTex;

    if (!backgroundTex.loadFromFile(basePath + "/sprites/background.jpg") ||
        !headTex.loadFromFile(basePath + "/sprites/snake_head.png") ||
        !bodyTex.loadFromFile(basePath + "/sprites/snake_body.png") ||
        !foodTex.loadFromFile(basePath + "/sprites/apple.jpg")
        ) {
        std::cerr << "Failed to load one or more textures!\n";
        return -1;
    }

    if (!bonusFoodTex.loadFromFile(basePath + "/sprites/bonus.png")) {
        std::cerr << "Failed to load bonus food sprite!\n";
        return -1;
    }
    bonusFoodSprite.setTexture(bonusFoodTex);
    bonusFoodSprite.setScale(
        static_cast<float>(cellSize) / bonusFoodTex.getSize().x,
        static_cast<float>(cellSize) / bonusFoodTex.getSize().y
    );

    sf::Sprite bgTile(backgroundTex);
    bgTile.setScale(
        static_cast<float>(cellSize) / backgroundTex.getSize().x,
        static_cast<float>(cellSize) / backgroundTex.getSize().y
    );

    sf::Sprite headSprite(headTex);
    headSprite.setScale(
        static_cast<float>(cellSize) / headTex.getSize().x,
        static_cast<float>(cellSize) / headTex.getSize().y
    );

    sf::Sprite bodySprite(bodyTex);
    bodySprite.setScale(
        static_cast<float>(cellSize) / bodyTex.getSize().x,
        static_cast<float>(cellSize) / bodyTex.getSize().y
    );

    sf::Sprite foodSprite(foodTex);
    foodSprite.setScale(
        static_cast<float>(cellSize) / foodTex.getSize().x,
        static_cast<float>(cellSize) / foodTex.getSize().y
    );

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

    GameState state = GameState::MENU;
    std::deque<sf::Vector2i> snake;
    Direction dir = Direction::Right;
    sf::Vector2i food;
    int score = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (state == GameState::MENU) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Enter) {
                        state = GameState::PLAYING;
                        score = 0;
                        snake.clear();
                        snake.push_back({width / 2, height / 2});
                        dir = getRandomDirection(rng);
                        food = getRandomPosition(snake, rng);
                        bgMusic.play();
                    }
                    if (event.key.code == sf::Keyboard::Escape) window.close();
                }
            } else if (state == GameState::PLAYING) {
                int speed = baseSpeed + (score / 5);
                window.setFramerateLimit(speed);

                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Up && dir != Direction::Down) dir = Direction::Up;
                    if (event.key.code == sf::Keyboard::Down && dir != Direction::Up) dir = Direction::Down;
                    if (event.key.code == sf::Keyboard::Left && dir != Direction::Right) dir = Direction::Left;
                    if (event.key.code == sf::Keyboard::Right && dir != Direction::Left) dir = Direction::Right;
                    if (event.key.code == sf::Keyboard::P) state = GameState::PAUSED;
                }
            } else if (state == GameState::PAUSED) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P)
                    state = GameState::PLAYING;
            } else if (state == GameState::GAME_OVER) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::R) {
                        // Restart game
                        score = 0;
                        snake.clear();
                        snake.push_back({width / 2, height / 2});
                        dir = getRandomDirection(rng);
                        food = getRandomPosition(snake, rng);
                        state = GameState::PLAYING;
                        bgMusic.play();
                    } else if (event.key.code == sf::Keyboard::M) {
                        state = GameState::MENU;
                        //bgMusic.play();
                    }
                }
            }
        }

        if (state == GameState::PLAYING) {
            sf::Vector2i head = snake.front();
            head += directionToVector(dir);

            if (isOutOfBounds(head) || checkSelfCollision(snake)) {
                state = GameState::GAME_OVER;
                bgMusic.stop();
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

            //bonus food spawning
            if (!bonusFoodVisible) {
                bonusCooldownTimer--;
                if (bonusCooldownTimer <= 0) {
                    bonusFood = getRandomPosition(snake, rng);
                    bonusFoodVisible = true;
                    bonusFoodTimer = bonusFoodDuration;
                    bonusCooldownTimer = bonusFoodCooldown;
                }
            } else {
                bonusFoodTimer--;
                if (bonusFoodTimer <= 0) {
                    bonusFoodVisible = false;
                }
            }

            // if bonus food was eaten
            if (bonusFoodVisible && head == bonusFood) {
                score += 5; // Give extra points
                bonusFoodVisible = false;
                eatSound.play();
            }

            scoreText.setString("Score: " + std::to_string(score));
        }

        // Draw background grid
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                bgTile.setPosition(x * cellSize, y * cellSize);
                window.draw(bgTile);
            }
        }

        // Draw snake
        for (size_t i = 0; i < snake.size(); ++i) {
            sf::Vector2i pos = snake[i];
            sf::Sprite& sprite = (i == 0) ? headSprite : bodySprite;
            sprite.setPosition(pos.x * cellSize, pos.y * cellSize);
        }

        if (state == GameState::MENU) {
            sf::Text title("SNAKE GAME", font, 48);
            title.setFillColor(sf::Color::Green);
            title.setPosition(100, 100);

            sf::Text prompt("Press Enter to Start\nPress Esc to Exit", font, 24);
            prompt.setFillColor(sf::Color::White);
            prompt.setPosition(100, 200);

            window.draw(title);
            window.draw(prompt);
        } else if (state == GameState::PLAYING || state == GameState::PAUSED) {
            // Draw food
            foodSprite.setPosition(food.x * cellSize, food.y * cellSize);
            window.draw(foodSprite);

            // Draw bonus food
            if (bonusFoodVisible) {
                bonusFoodSprite.setPosition(bonusFood.x * cellSize, bonusFood.y * cellSize);
                window.draw(bonusFoodSprite);
            }

            bool isHead = true;
            for (const auto& segment : snake) {
                if (isHead) {
                    headSprite.setPosition(segment.x * cellSize, segment.y * cellSize);
                    headSprite.setRotation(0);

                    switch (dir) {
                        case Direction::Up:
                            headSprite.setRotation(180);
                            break;
                        case Direction::Right:
                            headSprite.setRotation(270);
                            break;
                        case Direction::Down:
                            headSprite.setRotation(0);
                            break;
                        case Direction::Left:
                            headSprite.setRotation(90);
                            break;
                    }

                    headSprite.setOrigin(
                        headTex.getSize().x / 2.0f,
                        headTex.getSize().y / 2.0f
                    );

                    headSprite.setPosition(
                        segment.x * cellSize + cellSize / 2.0f,
                        segment.y * cellSize + cellSize / 2.0f
                    );

                    window.draw(headSprite);
                    isHead = false;
                } else {
                    bodySprite.setPosition(segment.x * cellSize, segment.y * cellSize);
                    window.draw(bodySprite);
                }
            }

            window.draw(scoreText);

            if (bonusFoodVisible) {
                sf::Text bonusTimerText("Bonus: " + std::to_string(bonusFoodTimer / 5), font, 20);
                bonusTimerText.setFillColor(sf::Color::Yellow);
                bonusTimerText.setPosition(10, 35);
                window.draw(bonusTimerText);
            }

            if (state == GameState::PAUSED) {
                sf::Text pausedText("PAUSED\nPress P to Resume", font, 24);
                pausedText.setFillColor(sf::Color::White);
                pausedText.setPosition(100, 200);
                window.draw(pausedText);
            }
        } else if (state == GameState::GAME_OVER) {
            sf::Text gameOverText("Game Over\nPress R to Restart\nPress M for Main Menu", font, 24);
            gameOverText.setFillColor(sf::Color::White);
            gameOverText.setPosition(100, 200);
            window.draw(gameOverText);
        }

        window.display();
    }

    return 0;
}
