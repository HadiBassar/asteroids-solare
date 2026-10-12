#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <optional>

// Accelerazione di gravita' che il Sole (nel punto "centro") esercita su un
// corpo che si trova in "pos": direzione verso il Sole, modulo GM / r^2.
sf::Vector2f accelerazioneGravita(sf::Vector2f pos, sf::Vector2f centro, float GM) {
    const sf::Vector2f d = centro - pos;
    const float r2 = d.x * d.x + d.y * d.y;
    const float r = std::sqrt(r2);
    if (r < 1.f) return {0.f, 0.f}; // evita la divisione per zero
    return d / r * (GM / r2);       // vettore unitario verso il Sole * GM / r^2
}

int main() {
    const float larghezza = 1200.f;
    const float altezza = 800.f;

    sf::RenderWindow window(sf::VideoMode({1200u, 800u}), "Asteroids nel sistema solare - Tappa 04");
    window.setFramerateLimit(60);

    // Il Sole, fermo al centro
    const sf::Vector2f centroSole{larghezza / 2.f, altezza / 2.f};
    const float raggioSole = 40.f;
    sf::CircleShape sole(raggioSole);
    sole.setFillColor(sf::Color::Yellow);
    sole.setOrigin({raggioSole, raggioSole});
    sole.setPosition(centroSole);

    // Costante gravitazionale del Sole (G * massa), scelta a mano per avere
    // un'orbita ben visibile sullo schermo
    const float GM = 2'000'000.f;

    // Il pianeta: parte a destra del Sole, a distanza 200
    const float raggioPianeta = 12.f;
    sf::CircleShape pianeta(raggioPianeta);
    pianeta.setFillColor(sf::Color::Cyan);
    pianeta.setOrigin({raggioPianeta, raggioPianeta});
    const float distanza = 200.f;
    pianeta.setPosition(centroSole + sf::Vector2f{distanza, 0.f});

    // Velocita' iniziale perpendicolare alla direzione verso il Sole.
    // Per un'orbita circolare serve v = sqrt(GM / r).
    sf::Vector2f velocitaPianeta{0.f, std::sqrt(GM / distanza)};

    // La navicella, come nella Tappa 03 (per ora la gravita' non la influenza)
    sf::ConvexShape nave(3);
    nave.setPoint(0, {0.f, -20.f});
    nave.setPoint(1, {-12.f, 10.f});
    nave.setPoint(2, {12.f, 10.f});
    nave.setFillColor(sf::Color::White);
    nave.setPosition({larghezza / 2.f, altezza / 2.f + 300.f});

    const float velocitaRotazione = 180.f;
    const float accelerazione = 200.f;
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

        // Delta time, limitato a 50 ms: se la finestra si blocca un attimo
        // (per esempio trascinandola), la simulazione non fa un salto enorme
        const float dt = std::min(orologio.restart().asSeconds(), 0.05f);

        // --- Navicella (invariata) ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            nave.rotate(sf::degrees(-velocitaRotazione * dt));
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            nave.rotate(sf::degrees(velocitaRotazione * dt));
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            const float angolo = nave.getRotation().asRadians();
            const sf::Vector2f direzione{std::sin(angolo), -std::cos(angolo)};
            velocita += direzione * accelerazione * dt;
        }
        nave.move(velocita * dt);

        sf::Vector2f pos = nave.getPosition();
        if (pos.x < 0.f) pos.x += larghezza;
        if (pos.x > larghezza) pos.x -= larghezza;
        if (pos.y < 0.f) pos.y += altezza;
        if (pos.y > altezza) pos.y -= altezza;
        nave.setPosition(pos);

        // --- Pianeta: gravita' ---
        // Prima si aggiorna la velocita' con l'accelerazione, poi la posizione
        // con la nuova velocita' (Eulero semi-implicito)
        const sf::Vector2f acc = accelerazioneGravita(pianeta.getPosition(), centroSole, GM);
        velocitaPianeta += acc * dt;
        pianeta.move(velocitaPianeta * dt);

        window.clear(sf::Color::Black);
        window.draw(sole);
        window.draw(pianeta);
        window.draw(nave);
        window.display();
    }
}