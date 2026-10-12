#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <optional>
#include <random>
#include <vector>
#include<cstdint>

struct Pianeta {
    sf::CircleShape forma;
    sf::Vector2f velocita;
};

sf::Vector2f accelerazioneGravita(sf::Vector2f pos, sf::Vector2f centro, float GM) {
    const sf::Vector2f d = centro - pos;
    const float r = std::sqrt(d.x * d.x + d.y * d.y);
    if (r < 1.f) return {0.f, 0.f};
    const float rEff = std::max(r, 25.f);
    return d / r * (GM / (rEff * rEff));
}

Pianeta creaPianeta(sf::Vector2f centroSole, float distanza, float raggio,
                    sf::Color colore, float angolo, float GM) {
    Pianeta p;
    p.forma.setRadius(raggio);
    p.forma.setFillColor(colore);
    p.forma.setOrigin({raggio, raggio});
    const sf::Vector2f dir{std::cos(angolo), std::sin(angolo)};
    p.forma.setPosition(centroSole + dir * distanza);
    p.velocita = sf::Vector2f{-dir.y, dir.x} * std::sqrt(GM / distanza);
    return p;
}

int main() {
    const float larghezza = 1200.f;
    const float altezza = 800.f;

    sf::RenderWindow window(sf::VideoMode({1200u, 800u}), "Asteroids nel sistema solare - Tappa 06");
    window.setFramerateLimit(60);

    // Il Sole
    const sf::Vector2f centroSole{larghezza / 2.f, altezza / 2.f};
    const float raggioSole = 40.f;
    sf::CircleShape sole(raggioSole);
    sole.setFillColor(sf::Color::Yellow);
    sole.setOrigin({raggioSole, raggioSole});
    sole.setPosition(centroSole);

    const float GM = 2'000'000.f;

    // I pianeti
    std::vector<Pianeta> pianeti;
    pianeti.push_back(creaPianeta(centroSole, 110.f, 8.f, sf::Color::Red, 0.f, GM));
    pianeti.push_back(creaPianeta(centroSole, 170.f, 12.f, sf::Color::Cyan, 2.f, GM));
    pianeti.push_back(creaPianeta(centroSole, 240.f, 10.f, sf::Color::Green, 4.f, GM));
    pianeti.push_back(creaPianeta(centroSole, 320.f, 14.f, sf::Color::Magenta, 1.f, GM));

      // Ogni stella e' un quadratino (2 triangoli) di lato 4-10 unita' del mondo,
    // con luminosita' casuale. Essendo in coordinate del mondo, con lo zoom
    // si ingrandiscono e rimpiccioliscono insieme al resto.
    sf::VertexArray stelle(sf::PrimitiveType::Triangles);
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> posDistr(-4000.f, 4000.f);
    std::uniform_real_distribution<float> dimDistr(3.f, 8.f);
    std::uniform_int_distribution<int> lumDistr(140, 255);
    for (int i = 0; i < 2000; ++i) {
        const float x = posDistr(gen);
        const float y = posDistr(gen);
        const float s = dimDistr(gen); // meta' del lato
        const std::uint8_t l = static_cast<std::uint8_t>(lumDistr(gen));
        const sf::Color c(l, l, l);

        // primo triangolo
        stelle.append(sf::Vertex{{x - s, y - s}, c});
        stelle.append(sf::Vertex{{x + s, y - s}, c});
        stelle.append(sf::Vertex{{x + s, y + s}, c});
        // secondo triangolo
        stelle.append(sf::Vertex{{x - s, y - s}, c});
        stelle.append(sf::Vertex{{x + s, y + s}, c});
        stelle.append(sf::Vertex{{x - s, y + s}, c});
    }
    

    // La navicella
    sf::ConvexShape nave(3);
    nave.setPoint(0, {0.f, -20.f});
    nave.setPoint(1, {-12.f, 10.f});
    nave.setPoint(2, {12.f, 10.f});
    nave.setFillColor(sf::Color::White);

    const float velocitaRotazione = 180.f;
    const float accelerazione = 200.f;
    sf::Vector2f velocita{0.f, 0.f};

    auto resetNave = [&]() {
        const float distanzaNave = 360.f;
        nave.setPosition(centroSole + sf::Vector2f{0.f, distanzaNave});
        nave.setRotation(sf::degrees(0.f));
        velocita = {-std::sqrt(GM / distanzaNave), 0.f};
    };
    resetNave();

    // NOVITA': la camera. Una View ha un centro e una dimensione (in unita' del mondo).
    // Cio' che sta dentro il rettangolo della View viene mostrato nella finestra.
    const sf::Vector2f dimensioneBase{larghezza, altezza};
    float zoom = 1.f; // 1 = normale, < 1 = ingrandisce, > 1 = allontana
    sf::View view(nave.getPosition(), dimensioneBase);

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
            else if (const auto* rotella = event->getIf<sf::Event::MouseWheelScrolled>()) {
                // Rotella in avanti: avvicina. Indietro: allontana.
                if (rotella->delta > 0.f) zoom *= 0.9f;
                else if (rotella->delta < 0.f) zoom *= 1.1f;
                zoom = std::clamp(zoom, 0.3f, 6.f);
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
        velocita += accelerazioneGravita(nave.getPosition(), centroSole, GM) * dt;
        nave.move(velocita * dt);
        // (il wrap-around ai bordi non c'e' piu': il mondo e' illimitato)

        // --- Pianeti ---
        for (Pianeta& p : pianeti) {
            p.velocita += accelerazioneGravita(p.forma.getPosition(), centroSole, GM) * dt;
            p.forma.move(p.velocita * dt);
        }

        // --- Camera: segue la navicella, la dimensione dipende dallo zoom ---
        view.setCenter(nave.getPosition());
        view.setSize(dimensioneBase * zoom);
        window.setView(view);

        window.clear(sf::Color::Black);
        window.draw(stelle);
        window.draw(sole);
        for (const Pianeta& p : pianeti)
            window.draw(p.forma);
        window.draw(nave);
        window.display();
    }
}