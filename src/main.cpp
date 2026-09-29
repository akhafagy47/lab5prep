#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;

// Number of frames it takes the circle to cross the screen once.
const int FRAMES_PER_ANIMATION = 60;
const float PI = 3.14159265f;

// Number of frames elapsed since the program started.
int elapsedFrames = 0;

// global tween function
std::function<float(float, float, float)> tween = [](float a, float b, float t) {
    return (1 - t) * a + t * b;
};

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        }

        // Keys 1-9 replace the tween function. Each easing maps t in [0, 1]
        // to an eased value e, which is then used to interpolate from a to b.
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            switch (keyPressed->code) {
                case sf::Keyboard::Key::Num1:  // easeInQuad
                    tween = [](float a, float b, float t) {
                        float e = t * t;
                        return a + (b - a) * e;
                    };
                    break;
                case sf::Keyboard::Key::Num2:  // easeOutQuad
                    tween = [](float a, float b, float t) {
                        float e = 1 - (1 - t) * (1 - t);
                        return a + (b - a) * e;
                    };
                    break;
                case sf::Keyboard::Key::Num3:  // easeInOutCubic
                    tween = [](float a, float b, float t) {
                        float e = t < 0.5f ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3.0f) / 2;
                        return a + (b - a) * e;
                    };
                    break;
                case sf::Keyboard::Key::Num4:  // easeInOutSine
                    tween = [](float a, float b, float t) {
                        float e = -(std::cos(PI * t) - 1) / 2;
                        return a + (b - a) * e;
                    };
                    break;
                case sf::Keyboard::Key::Num5:  // easeOutExpo
                    tween = [](float a, float b, float t) {
                        float e = t >= 1 ? 1 : 1 - std::pow(2.0f, -10 * t);
                        return a + (b - a) * e;
                    };
                    break;
                case sf::Keyboard::Key::Num6:  // easeInOutCirc
                    tween = [](float a, float b, float t) {
                        float e = t < 0.5f ? (1 - std::sqrt(1 - std::pow(2 * t, 2.0f))) / 2
                                           : (std::sqrt(1 - std::pow(-2 * t + 2, 2.0f)) + 1) / 2;
                        return a + (b - a) * e;
                    };
                    break;
                case sf::Keyboard::Key::Num7:  // easeInBack
                    tween = [](float a, float b, float t) {
                        const float c1 = 1.70158f;
                        const float c3 = c1 + 1;
                        float e = c3 * t * t * t - c1 * t * t;
                        return a + (b - a) * e;
                    };
                    break;
                case sf::Keyboard::Key::Num8:  // easeOutElastic
                    tween = [](float a, float b, float t) {
                        const float c4 = (2 * PI) / 3;
                        float e = t <= 0   ? 0
                                  : t >= 1 ? 1
                                           : std::pow(2.0f, -10 * t) *
                                                     std::sin((t * 10 - 0.75f) * c4) +
                                                 1;
                        return a + (b - a) * e;
                    };
                    break;
                case sf::Keyboard::Key::Num9:  // easeOutBounce
                    tween = [](float a, float b, float t) {
                        const float n1 = 7.5625f;
                        const float d1 = 2.75f;
                        float e;
                        if (t < 1 / d1) {
                            e = n1 * t * t;
                        } else if (t < 2 / d1) {
                            t -= 1.5f / d1;
                            e = n1 * t * t + 0.75f;
                        } else if (t < 2.5f / d1) {
                            t -= 2.25f / d1;
                            e = n1 * t * t + 0.9375f;
                        } else {
                            t -= 2.625f / d1;
                            e = n1 * t * t + 0.984375f;
                        }
                        return a + (b - a) * e;
                    };
                    break;
                default:
                    break;
            }
        }
    }
}

void render(sf::RenderWindow& window) {
    // Clear with blue background (sky)
    window.clear(sf::Color::Black);

    // Animation time in [0, 1], restarting every FRAMES_PER_ANIMATION frames.
    float t = static_cast<float>(elapsedFrames % (FRAMES_PER_ANIMATION + 1)) /
              FRAMES_PER_ANIMATION;
    elapsedFrames++;

    // Circle that moves from the left side to the right side of the screen,
    // about 1/3 of the way down.
    const float radius = 25.f;
    sf::CircleShape circle(radius);
    circle.setOrigin({radius, radius});
    circle.setFillColor(sf::Color::Cyan);
    circle.setPosition({tween(radius, WINDOW_WIDTH - radius, t), WINDOW_HEIGHT / 3.f});
    window.draw(circle);

    // Plot of the tween function at the bottom of the window. The graph's
    // bounds are [0, 1] on both axes; (0, 0) is at the bottom-left.
    const float graphLeft = 100.f;
    const float graphBottom = WINDOW_HEIGHT - 60.f;
    const float graphWidth = WINDOW_WIDTH - 2 * graphLeft;
    const float graphHeight = 250.f;
    auto toScreen = [&](float x, float y) {
        return sf::Vector2f{graphLeft + x * graphWidth, graphBottom - y * graphHeight};
    };

    // Axes, plus faint lines marking x = 1 and y = 1.
    sf::VertexArray axes(sf::PrimitiveType::Lines);
    const sf::Color axisColor = sf::Color::White;
    const sf::Color boundColor(80, 80, 80);
    axes.append({toScreen(0, 0), axisColor});
    axes.append({toScreen(1, 0), axisColor});
    axes.append({toScreen(0, 0), axisColor});
    axes.append({toScreen(0, 1), axisColor});
    axes.append({toScreen(1, 0), boundColor});
    axes.append({toScreen(1, 1), boundColor});
    axes.append({toScreen(0, 1), boundColor});
    axes.append({toScreen(1, 1), boundColor});
    window.draw(axes);

    // Sample the tween function from 0 to 1.
    const int samples = 200;
    sf::VertexArray curve(sf::PrimitiveType::LineStrip);
    for (int i = 0; i <= samples; i++) {
        float x = static_cast<float>(i) / samples;
        curve.append({toScreen(x, tween(0, 1, x)), sf::Color::Yellow});
    }
    window.draw(curve);

    // Dot showing the current position of the animation on the curve.
    const float dotRadius = 6.f;
    sf::CircleShape dot(dotRadius);
    dot.setOrigin({dotRadius, dotRadius});
    dot.setFillColor(sf::Color::Red);
    dot.setPosition(toScreen(t, tween(0, 1, t)));
    window.draw(dot);

    window.display();
}

int main() {
    sf::RenderWindow window;

    try {
        // Initialize window
        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Tween");
        window.setFramerateLimit(FPS_LIMIT);
        // Prevent key repeats.
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;
        // Main game loop
        while (window.isOpen()) {
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
