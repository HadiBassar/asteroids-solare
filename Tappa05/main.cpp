#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

// Un pianeta: la sua forma da disegnare e la sua velocita' (vettore 2D)
struct Pianeta {
    sf::CircleShape forma;
    sf::Vector2f velocita;
};

// Accelerazione di gravita' verso il Sole, valida per QUALSIASI corpo:
// la massa del corpo si semplifica, quindi non serve conoscerla.
sf::Vector2f accelerazioneGravita(sf::Vector2f pos, sf::Vector2f centro, float GM) {
    const sf::Vector2f d = centro - pos;
    const float r = std::sqrt(d.x * d.x + d.y * d.y);
    if (r < 1.f) return {0.f, 0.f};
    // Distanza minima per il calcolo del modulo: vicino al Sole la forza
    // non cresce all'infinito (evita lanci a velocita' assurde)
    const float rEff = std::max(r, 25.f);
    return d / r * (GM / (rEff * rEff));
}

// Crea un pianeta in orbita circolare: parte a "distanza" dal Sole, nella
// direzione data da "angolo", con velocita' perpendicolare al raggio.
Pianeta creaPianeta(sf::Vector2f centroSole, float distanza, float raggio,
                    sf::Color colore, float angolo, float GM) {
    Pianeta p;
    p.forma.setRadius(raggio);
    p.forma.setFillColor(colore);
    p.forma.setOrigin({raggio, raggio});
    const sf::Vector2f dir{std::cos(angolo), std::sin(angolo)};
    p.forma.setPosition(centroSole + dir * distanza);
    // Ruotare (x, y) di 90 gradi da' (-y, x): direzione perpendicolare
    p.velocita = sf::Vector2f{-dir.y, dir.x} * std::sqrt(GM / distanza);
    return p;
}

int main() {
    const float larghezza = 1200.f;
    const float altezza = 800.f;

    sf::RenderWindow window(sf::VideoMode({1200u, 800u}), "Asteroids nel sistema solare - Tappa 05");
    window.setFramerateLimit(60);

    // Il Sole
    const sf::Vector2f centroSole{larghezza / 2.f, altezza / 2.f};
    const float raggioSole = 40.f;
    sf::CircleShape sole(raggioSole);
    sole.setFillColor(sf::Color::Yellow);
    sole.setOrigin({raggioSole, raggioSole});
    sole.setPosition(centroSole);

    const float GM = 2'000'000.f;

    // I pianeti: distanza, raggio, colore, angolo iniziale (radianti)
    std::vector<Pianeta> pianeti;
    pianeti.push_back(creaPianeta(centroSole, 110.f, 8.f, sf::Color::Red, 0.f, GM));
    pianeti.push_back(creaPianeta(centroSole, 170.f, 12.f, sf::Color::Cyan, 2.f, GM));
    pianeti.push_back(creaPianeta(centroSole, 240.f, 10.f, sf::Color::Green, 4.f, GM));
    pianeti.push_back(creaPianeta(centroSole, 320.f, 14.f, sf::Color::Magenta, 1.f, GM));

    // La navicella
    sf::ConvexShape nave(3);
    nave.setPoint(0, {0.f, -20.f});
    nave.setPoint(1, {-12.f, 10.f});
    nave.setPoint(2, {12.f, 10.f});
    nave.setFillColor(sf::Color::White);

    const float velocitaRotazione = 180.f;
    const float accelerazione = 200.f;
    sf::Vector2f velocita{0.f, 0.f};

    // Rimette la navicella sotto il Sole, in orbita circolare (tasto R)
    auto resetNave = [&]() {
        const float distanzaNave = 360.f;
        nave.setPosition(centroSole + sf::Vector2f{0.f, distanzaNave});
        nave.setRotation(sf::degrees(0.f));
        velocita = {-std::sqrt(GM / distanzaNave), 0.f};
    };
    resetNave();

    sf::Clock orologio;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape)
                    window.close();
                else if (key->code == sf::Keyboard::Key::R)
                    resetNave();
            }
        }

        const float dt = std::min(orologio.restart().asSeconds(), 0.05f);

        // --- Navicella ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            nave.rotate(sf::degrees(-velocitaRotazione * dt));
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            nave.rotate(sf::degrees(velocitaRotazione * dt));
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            const float angolo = nave.getRotation().asRadians();
            const sf::Vector2f direzione{std::sin(angolo), -std::cos(angolo)};
            velocita += direzione * accelerazione * dt;
        }

        // NOVITA': la gravita' del Sole agisce anche sulla navicella
        velocita += accelerazioneGravita(nave.getPosition(), centroSole, GM) * dt;
        nave.move(velocita * dt);

        sf::Vector2f pos = nave.getPosition();
        if (pos.x < 0.f) pos.x += larghezza;
        if (pos.x > larghezza) pos.x -= larghezza;
        if (pos.y < 0.f) pos.y += altezza;
        if (pos.y > altezza) pos.y -= altezza;
        nave.setPosition(pos);

        // --- Pianeti: stesso calcolo per ognuno ---
        for (Pianeta& p : pianeti) {
            p.velocita += accelerazioneGravita(p.forma.getPosition(), centroSole, GM) * dt;
            p.forma.move(p.velocita * dt);
        }

        window.clear(sf::Color::Black);
        window.draw(sole);
        for (const Pianeta& p : pianeti)
            window.draw(p.forma);
        window.draw(nave);
        window.display();
    }
}