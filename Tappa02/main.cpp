#include <SFML/Graphics.hpp>
#include <optional>

int main() {
    const unsigned int larghezza = 1200;
    const unsigned int altezza = 800;

    sf::RenderWindow window(sf::VideoMode({larghezza, altezza}), "Asteroids nel sistema solare - Tappa 02");
    window.setFramerateLimit(60);

    // Il Sole, come nella Tappa 01
    const float raggio = 40.f;
    sf::CircleShape sole(raggio);
    sole.setFillColor(sf::Color::Yellow);
    sole.setOrigin({raggio, raggio});
    sole.setPosition({larghezza / 2.f, altezza / 2.f});

    // La navicella: triangolo con la punta verso l'alto (asse y negativo).
    // I punti sono definiti attorno all'origine (0,0), cosi' la rotazione
    // avviene attorno al centro della navicella.
    sf::ConvexShape nave(3);
    nave.setPoint(0, {0.f, -20.f});   // punta
    nave.setPoint(1, {-12.f, 10.f});  // base sinistra
    nave.setPoint(2, {12.f, 10.f});   // base destra
    nave.setFillColor(sf::Color::White);
    nave.setPosition({larghezza / 2.f + 250.f, altezza / 2.f});

    const float velocitaRotazione = 180.f; // gradi al secondo

    sf::Clock orologio;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape)
                    window.close();
            }
        }

        // Delta time: secondi trascorsi dal frame precedente
        const float dt = orologio.restart().asSeconds();

        // Rotazione con le frecce
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            nave.rotate(sf::degrees(-velocitaRotazione * dt));
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            nave.rotate(sf::degrees(velocitaRotazione * dt));

        window.clear(sf::Color::Black);
        window.draw(sole);
        window.draw(nave);
        window.display();
    }
}