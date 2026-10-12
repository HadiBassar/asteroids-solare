#include <SFML/Graphics.hpp>
#include <optional>

int main() {
    const unsigned int larghezza = 1200;
    const unsigned int altezza = 800;

    sf::RenderWindow window(sf::VideoMode({larghezza, altezza}), "Asteroids nel sistema solare - Tappa 01");
    window.setFramerateLimit(60);

    // Il Sole: cerchio con l'origine al centro, posizionato al centro della finestra
    const float raggio = 40.f;
    sf::CircleShape sole(raggio);
    sole.setFillColor(sf::Color::Yellow);
    sole.setOrigin({raggio, raggio});
    sole.setPosition({larghezza / 2.f, altezza / 2.f});

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape)
                    window.close();
            }
        }

        window.clear(sf::Color::Black);
        window.draw(sole);
        window.display();
    }
}