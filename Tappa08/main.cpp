#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

constexpr float PI = 3.14159265f;

struct Pianeta {
    sf::CircleShape forma;
    sf::Vector2f velocita;
};

struct Proiettile {
    sf::CircleShape forma;
    sf::Vector2f velocita;
    float vita;
};

// NOVITA': un asteroide e' un poligono definito in coordinate LOCALI
// (attorno a (0,0)), piu' una posizione e una rotazione nel mondo.
struct Asteroide {
    sf::VertexArray forma;
    sf::Vector2f posizione;
    sf::Vector2f velocita;
    float rotazione;     // gradi
    float velRotazione;  // gradi al secondo
    float raggio;        // raggio usato per le collisioni
    bool viva = true;
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

// Collisione cerchio-cerchio: i cerchi si toccano se la distanza tra i centri
// e' minore della somma dei raggi. Si confrontano i QUADRATI per evitare sqrt.
bool collisione(sf::Vector2f a, float ra, sf::Vector2f b, float rb) {
    const sf::Vector2f d = a - b;
    const float somma = ra + rb;
    return d.x * d.x + d.y * d.y < somma * somma;
}

// Poligono irregolare a 10 vertici: ogni raggio e' ridotto di un fattore casuale.
// Si disegna come "ventaglio di triangoli" con il centro come primo vertice.
sf::VertexArray creaPoligono(float raggio, std::mt19937& gen) {
    std::uniform_real_distribution<float> variazione(0.75f, 1.f);
    const int n = 10;
    const sf::Color centro(90, 90, 90);
    const sf::Color bordo(170, 170, 170);

    std::vector<sf::Vector2f> punti;
    for (int i = 0; i < n; ++i) {
        const float angolo = 2.f * PI * static_cast<float>(i) / static_cast<float>(n);
        const float r = raggio * variazione(gen);
        punti.push_back({std::cos(angolo) * r, std::sin(angolo) * r});
    }

    sf::VertexArray v(sf::PrimitiveType::TriangleFan);
    v.append(sf::Vertex{{0.f, 0.f}, centro});
    for (const sf::Vector2f& p : punti)
        v.append(sf::Vertex{p, bordo});
    v.append(sf::Vertex{punti[0], bordo}); // chiude il poligono
    return v;
}

int main() {
    const float larghezza = 1200.f;
    const float altezza = 800.f;

    sf::RenderWindow window(sf::VideoMode({1200u, 800u}), "Asteroids nel sistema solare - Tappa 08");
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

    // Stelle
    std::mt19937 gen(42);
    sf::VertexArray stelle(sf::PrimitiveType::Triangles);
    {
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
    }

    // La navicella
    sf::ConvexShape nave(3);
    nave.setPoint(0, {0.f, -20.f});
    nave.setPoint(1, {-12.f, 10.f});
    nave.setPoint(2, {12.f, 10.f});
    nave.setFillColor(sf::Color::White);
    const float raggioNave = 14.f; // cerchio che approssima la navicella

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

    // Proiettili
    std::vector<Proiettile> proiettili;
    const float velocitaProiettile = 500.f;
    const float durataProiettile = 2.f;
    const float intervalloSparo = 0.25f;
    const float raggioProiettile = 3.f;
    float ricarica = 0.f;

    // NOVITA': gli asteroidi
    auto casuale = [&](float a, float b) {
        return std::uniform_real_distribution<float>(a, b)(gen);
    };

    // Crea un asteroide in orbita attorno al Sole, lontano dalla navicella
    auto creaAsteroide = [&]() {
        Asteroide a;
        a.raggio = casuale(15.f, 40.f);
        a.forma = creaPoligono(a.raggio, gen);

        float distanza, angolo;
        sf::Vector2f dir;
        do {
            distanza = casuale(380.f, 700.f);
            angolo = casuale(0.f, 2.f * PI);
            dir = {std::cos(angolo), std::sin(angolo)};
            a.posizione = centroSole + dir * distanza;
        } while (collisione(a.posizione, 0.f, nave.getPosition(), 150.f));

        // Velocita' perpendicolare al raggio, vicina a quella circolare,
        // con verso orario o antiorario a caso
        const float verso = (gen() % 2 == 0) ? 1.f : -1.f;
        a.velocita = sf::Vector2f{-dir.y, dir.x} * verso *
                     std::sqrt(GM / distanza) * casuale(0.85f, 1.05f);
        a.rotazione = casuale(0.f, 360.f);
        a.velRotazione = casuale(-60.f, 60.f);
        return a;
    };

    const int numAsteroidi = 20;
    std::vector<Asteroide> asteroidi;
    for (int i = 0; i < numAsteroidi; ++i)
        asteroidi.push_back(creaAsteroide());

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

        const float angoloNave = nave.getRotation().asRadians();
        const sf::Vector2f direzione{std::sin(angoloNave), -std::cos(angoloNave)};

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            velocita += direzione * accelerazione * dt;

        velocita += accelerazioneGravita(nave.getPosition(), centroSole, GM) * dt;
        nave.move(velocita * dt);

        // --- Sparo ---
        ricarica -= dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && ricarica <= 0.f) {
            Proiettile p;
            p.forma.setRadius(raggioProiettile);
            p.forma.setFillColor(sf::Color(255, 120, 40));
            p.forma.setOrigin({raggioProiettile, raggioProiettile});
            p.forma.setPosition(nave.getPosition() + direzione * 20.f);
            p.velocita = velocita + direzione * velocitaProiettile;
            p.vita = durataProiettile;
            proiettili.push_back(p);
            ricarica = intervalloSparo;
        }

        // --- Aggiornamento proiettili ---
        for (Proiettile& p : proiettili) {
            p.velocita += accelerazioneGravita(p.forma.getPosition(), centroSole, GM) * dt;
            p.forma.move(p.velocita * dt);
            p.vita -= dt;
        }

        // --- Aggiornamento asteroidi ---
        for (Asteroide& a : asteroidi) {
            a.velocita += accelerazioneGravita(a.posizione, centroSole, GM) * dt;
            a.posizione += a.velocita * dt;
            a.rotazione += a.velRotazione * dt;
            // Un asteroide che cade nel Sole viene distrutto
            if (collisione(a.posizione, a.raggio * 0.9f, centroSole, raggioSole))
                a.viva = false;
        }

        // --- Pianeti ---
        for (Pianeta& p : pianeti) {
            p.velocita += accelerazioneGravita(p.forma.getPosition(), centroSole, GM) * dt;
            p.forma.move(p.velocita * dt);
        }

        // --- Collisioni ---
        // Proiettile contro asteroide: spariscono entrambi
        for (Proiettile& p : proiettili) {
            for (Asteroide& a : asteroidi) {
                if (a.viva && collisione(p.forma.getPosition(), raggioProiettile,
                                         a.posizione, a.raggio * 0.9f)) {
                    a.viva = false;
                    p.vita = 0.f; // sara' rimosso con gli altri scaduti
                    break;        // un proiettile colpisce al massimo un asteroide
                }
            }
        }
        // Navicella contro asteroide: la navicella ricomincia, l'asteroide si distrugge
        for (Asteroide& a : asteroidi) {
            if (a.viva && collisione(nave.getPosition(), raggioNave,
                                     a.posizione, a.raggio * 0.9f)) {
                a.viva = false;
                resetNave();
            }
        }
        // Navicella contro Sole
        if (collisione(nave.getPosition(), raggioNave, centroSole, raggioSole))
            resetNave();

        // --- Rimozione degli elementi distrutti ---
        proiettili.erase(
            std::remove_if(proiettili.begin(), proiettili.end(),
                           [](const Proiettile& p) { return p.vita <= 0.f; }),
            proiettili.end());
        asteroidi.erase(
            std::remove_if(asteroidi.begin(), asteroidi.end(),
                           [](const Asteroide& a) { return !a.viva; }),
            asteroidi.end());

        // Se li hai distrutti tutti, ne arrivano di nuovi
        if (asteroidi.empty())
            for (int i = 0; i < numAsteroidi; ++i)
                asteroidi.push_back(creaAsteroide());

        // --- Camera ---
        view.setCenter(nave.getPosition());
        view.setSize(dimensioneBase * zoom);
        window.setView(view);

        // --- Disegno ---
        window.clear(sf::Color::Black);
        window.draw(stelle);
        window.draw(sole);
        for (const Pianeta& p : pianeti)
            window.draw(p.forma);
        for (const Asteroide& a : asteroidi) {
            // Trasformazione modello: prima ruota (attorno all'origine locale),
            // poi trasla nella posizione del mondo
            sf::Transform t;
            t.translate(a.posizione);
            t.rotate(sf::degrees(a.rotazione));
            sf::RenderStates stati;
            stati.transform = t;
            window.draw(a.forma, stati);
        }
        for (const Proiettile& p : proiettili)
            window.draw(p.forma);
        window.draw(nave);
        window.display();
    }
}