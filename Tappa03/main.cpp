#include <SFML/Graphics.hpp>
#include <cmath>
#include <optional>

int main() {
    const float larghezza = 1200.f;
    const float altezza = 800.f;

    sf::RenderWindow window(sf::VideoMode({1200u, 800u}), "Asteroids nel sistema solare - Tappa 03");
    window.setFramerateLimit(60);

    // Il Sole
    const float raggio = 40.f;
    sf::CircleShape sole(raggio);
    sole.setFillColor(sf::Color::Yellow);
    sole.setOrigin({raggio, raggio});
    sole.setPosition({larghezza / 2.f, altezza / 2.f});

    // La navicella (punta verso l'alto a rotazione 0)
    sf::ConvexShape nave(3);
    nave.setPoint(0, {0.f, -20.f});
    nave.setPoint(1, {-12.f, 10.f});
    nave.setPoint(2, {12.f, 10.f});
    nave.setFillColor(sf::Color::White);
    nave.setPosition({larghezza / 2.f + 250.f, altezza / 2.f});

    const float velocitaRotazione = 180.f; // gradi al secondo
    const float accelerazione = 200.f;     // pixel al secondo^2

    // Stato della navicella: la velocita' e' un vettore 2D (pixel al secondo)
    sf::Vector2f velocita{0.f, 0.f};

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

        const float dt = orologio.restart().asSeconds();

        // Rotazione con le frecce sinistra/destra
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            nave.rotate(sf::degrees(-velocitaRotazione * dt));
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            nave.rotate(sf::degrees(velocitaRotazione * dt));

        // Spinta con la freccia su: aggiunge accelerazione nella direzione di puntamento.
        // A rotazione 0 la nave guarda verso l'alto, cioe' verso (0, -1).
        // Ruotando di un angolo a, la direzione diventa (sin a, -cos a).
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            const float angolo = nave.getRotation().asRadians();
            const sf::Vector2f direzione{std::sin(angolo), -std::cos(angolo)};
            velocita += direzione * accelerazione * dt;
        }

        // Integrazione di Eulero: nuova posizione = vecchia + velocita * dt
        nave.move(velocita * dt);

        // Se la nave esce da un bordo, rientra dal bordo opposto
        sf::Vector2f pos = nave.getPosition();
        if (pos.x < 0.f) pos.x += larghezza;
        if (pos.x > larghezza) pos.x -= larghezza;
        if (pos.y < 0.f) pos.y += altezza;
        if (pos.y > altezza) pos.y -= altezza;
        nave.setPosition(pos);

        window.clear(sf::Color::Black);
        window.draw(sole);
        window.draw(nave);
        window.display();
    }
}