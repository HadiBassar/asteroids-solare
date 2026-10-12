#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

struct Pianeta {
    sf::CircleShape forma;
    sf::Vector2f velocita;
};

// NOVITA': un proiettile ha forma, velocita' e un tempo di vita residuo
struct Proiettile {
    sf::CircleShape forma;
    sf::Vector2f velocita;
    float vita; // secondi che mancano alla scomparsa
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

    sf::RenderWindow window(sf::VideoMode({1200u, 800u}), "Asteroids nel sistema solare - Tappa 07");
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

    // Stelle: quadratini (2 triangoli) in coordinate del mondo
    sf::VertexArray stelle(sf::PrimitiveType::Triangles);
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> posDistr(-4000.f, 4000.f);
    std::uniform_real_distribution<float> dimDistr(2.f, 5.f);
    std::uniform_int_distribution<int> lumDistr(140, 255);
    for (int i = 0; i < 2000; ++i) {
        const float x = posDistr(gen);
        const float y = posDistr(gen);
        const float s = dimDistr(gen);
        const std::uint8_t l = static_cast<std::uint8_t>(lumDistr(gen));
        const sf::Color c(l, l, l);
        stelle.append(sf::Vertex{{x - s, y - s}, c});
        stelle.append(sf::Vertex{{x + s, y - s}, c});
        stelle.append(sf::Vertex{{x + s, y + s}, c});
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

    // Parametri dei proiettili
    std::vector<Proiettile> proiettili;
    const float velocitaProiettile = 500.f; // rispetto alla navicella
    const float durataProiettile = 2.f;     // secondi
    const float intervalloSparo = 0.25f;    // secondi tra due spari
    float ricarica = 0.f;                   // tempo che manca al prossimo sparo

    // Camera
    const sf::Vector2f dimensioneBase{larghezza, altezza};
    float zoom = 1.f;
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

        // Direzione verso cui punta la navicella (calcolata una volta per frame)
        const float angolo = nave.getRotation().asRadians();
        const sf::Vector2f direzione{std::sin(angolo), -std::cos(angolo)};

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            velocita += direzione * accelerazione * dt;

        velocita += accelerazioneGravita(nave.getPosition(), centroSole, GM) * dt;
        nave.move(velocita * dt);

        // --- Sparo ---
        ricarica -= dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && ricarica <= 0.f) {
            Proiettile p;
            p.forma.setRadius(3.f);
            p.forma.setFillColor(sf::Color(255, 120, 40));
            p.forma.setOrigin({3.f, 3.f});
            // Nasce sulla punta della navicella (20 unita' davanti al centro)
            p.forma.setPosition(nave.getPosition() + direzione * 20.f);
            // La velocita' e' quella della navicella + quella dello sparo
            p.velocita = velocita + direzione * velocitaProiettile;
            p.vita = durataProiettile;
            proiettili.push_back(p);
            ricarica = intervalloSparo;
        }

        // --- Proiettili: gravita', movimento, invecchiamento ---
        for (Proiettile& p : proiettili) {
            p.velocita += accelerazioneGravita(p.forma.getPosition(), centroSole, GM) * dt;
            p.forma.move(p.velocita * dt);
            p.vita -= dt;
        }
        // Rimuove i proiettili scaduti (idioma erase-remove)
        proiettili.erase(
            std::remove_if(proiettili.begin(), proiettili.end(),
                           [](const Proiettile& p) { return p.vita <= 0.f; }),
            proiettili.end());

        // --- Pianeti ---
        for (Pianeta& p : pianeti) {
            p.velocita += accelerazioneGravita(p.forma.getPosition(), centroSole, GM) * dt;
            p.forma.move(p.velocita * dt);
        }

        // --- Camera ---
        view.setCenter(nave.getPosition());
        view.setSize(dimensioneBase * zoom);
        window.setView(view);

        window.clear(sf::Color::Black);
        window.draw(stelle);
        window.draw(sole);
        for (const Pianeta& p : pianeti)
            window.draw(p.forma);
        for (const Proiettile& p : proiettili)
            window.draw(p.forma);
        window.draw(nave);
        window.display();
    }
}