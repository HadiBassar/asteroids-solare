#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>

constexpr float PI = 3.14159265f;

struct Pianeta {
    sf::CircleShape forma;
    sf::Vector2f velocita;
    std::deque<sf::Vector2f> scia; // NOVITA': ultime posizioni del pianeta
};

struct Proiettile {
    sf::CircleShape forma;
    sf::Vector2f velocita;
    float vita;
};

struct Asteroide {
    sf::VertexArray forma;
    sf::Vector2f posizione;
    sf::Vector2f velocita;
    float rotazione;
    float velRotazione;
    float raggio;
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

bool collisione(sf::Vector2f a, float ra, sf::Vector2f b, float rb) {
    const sf::Vector2f d = a - b;
    const float somma = ra + rb;
    return d.x * d.x + d.y * d.y < somma * somma;
}

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
    v.append(sf::Vertex{punti[0], bordo});
    return v;
}

Asteroide creaFrammento(const Asteroide& madre, float segno, std::mt19937& gen) {
    Asteroide f;
    f.raggio = madre.raggio * 0.6f;
    f.forma = creaPoligono(f.raggio, gen);

    const sf::Vector2f v = madre.velocita;
    const float modulo = std::sqrt(v.x * v.x + v.y * v.y);
    sf::Vector2f perp{1.f, 0.f};
    if (modulo > 0.001f)
        perp = sf::Vector2f{-v.y, v.x} / modulo;

    f.posizione = madre.posizione + perp * segno * f.raggio * 0.5f;
    f.velocita = v + perp * segno * 40.f;
    f.rotazione = madre.rotazione;
    f.velRotazione = -madre.velRotazione * 1.5f;
    return f;
}

bool caricaFont(sf::Font& font, const std::string& nome) {
    const std::vector<std::string> prefissi = {"risorse/", "../risorse/", "../../risorse/"};
    for (const std::string& p : prefissi)
        if (font.openFromFile(p + nome))
            return true;
    return false;
}

// NOVITA': aggiunge un punto alla scia e butta via i piu' vecchi
void aggiungiPunto(std::deque<sf::Vector2f>& scia, sf::Vector2f p, std::size_t massimo) {
    scia.push_back(p);
    while (scia.size() > massimo)
        scia.pop_front();
}

// NOVITA': disegna la scia come una linea spezzata. Il vertice piu' vecchio e'
// quasi trasparente, il piu' recente e' opaco: SFML interpola il colore (e
// l'alpha) lungo ogni segmento, quindi la coda sfuma.
void disegnaScia(sf::RenderTarget& target, const std::deque<sf::Vector2f>& scia, sf::Color colore) {
    if (scia.size() < 2) return;
    sf::VertexArray v(sf::PrimitiveType::LineStrip);
    const std::size_t n = scia.size();
    for (std::size_t i = 0; i < n; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(n - 1); // 0 = vecchio, 1 = recente
        sf::Color c = colore;
        c.a = static_cast<std::uint8_t>(255.f * t * t);
        v.append(sf::Vertex{scia[i], c});
    }
    target.draw(v);
}

int main() {
    const float larghezza = 1200.f;
    const float altezza = 800.f;

    sf::RenderWindow window(sf::VideoMode({1200u, 800u}), "Asteroids nel sistema solare - Tappa 10");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!caricaFont(font, "Font/Arial.ttf")) {
        std::cerr << "Font non trovato: controlla che esista risorse/Font/Arial.ttf\n"
                  << "e lancia l'eseguibile dalla radice del progetto o dalla cartella build.\n";
        return 1;
    }

    // Il Sole
    const sf::Vector2f centroSole{larghezza / 2.f, altezza / 2.f};
    const float raggioSole = 70.f;
    sf::CircleShape sole(raggioSole);
    sole.setFillColor(sf::Color::Yellow);
    sole.setOrigin({raggioSole, raggioSole});
    sole.setPosition(centroSole);

    const float GM = 2'000'000.f;

    // I pianeti
    std::vector<Pianeta> pianeti;
    pianeti.push_back(creaPianeta(centroSole, 110.f, 14.f, sf::Color::Red, 0.f, GM));
    pianeti.push_back(creaPianeta(centroSole, 170.f, 22.f, sf::Color::Cyan, 2.f, GM));
    pianeti.push_back(creaPianeta(centroSole, 240.f, 18.f, sf::Color::Green, 4.f, GM));
    pianeti.push_back(creaPianeta(centroSole, 320.f, 26.f, sf::Color::Magenta, 1.f, GM));

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
    const float raggioNave = 14.f;

    const float velocitaRotazione = 180.f;
    const float accelerazione = 200.f;
    sf::Vector2f velocita{0.f, 0.f};

    // Stato di gioco
    int punteggio = 0;
    int vite = 3;
    bool gameOver = false;
    float invulnerabile = 0.f;

    // NOVITA': scie, pausa
    std::deque<sf::Vector2f> sciaNave;
    bool scieAttive = true;
    bool inPausa = false;
    const float intervalloScia = 0.05f; // un punto ogni 50 ms
    float timerScia = 0.f;

    auto resetNave = [&]() {
        const float distanzaNave = 360.f;
        nave.setPosition(centroSole + sf::Vector2f{0.f, distanzaNave});
        nave.setRotation(sf::degrees(0.f));
        velocita = {-std::sqrt(GM / distanzaNave), 0.f};
        invulnerabile = 2.f;
        sciaNave.clear(); // la nave "salta": la vecchia scia non avrebbe senso
    };
    resetNave();

    auto perdiVita = [&]() {
        vite--;
        if (vite <= 0)
            gameOver = true;
        else
            resetNave();
    };

    // Proiettili
    std::vector<Proiettile> proiettili;
    const float velocitaProiettile = 500.f;
    const float durataProiettile = 2.f;
    const float intervalloSparo = 0.25f;
    const float raggioProiettile = 3.f;
    float ricarica = 0.f;

    // Asteroidi
    auto casuale = [&](float a, float b) {
        return std::uniform_real_distribution<float>(a, b)(gen);
    };

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

    auto riavvia = [&]() {
        punteggio = 0;
        vite = 3;
        gameOver = false;
        proiettili.clear();
        asteroidi.clear();
        resetNave();
        for (int i = 0; i < numAsteroidi; ++i)
            asteroidi.push_back(creaAsteroide());
    };

    // Testi
    sf::Text hud(font, "", 24);
    hud.setFillColor(sf::Color::White);
    hud.setPosition({15.f, 10.f});

    // NOVITA': riga di aiuto in basso a sinistra
    sf::Text aiuto(font,
        "Frecce: ruota e spingi | Spazio: spara | R: reinizia orbita | T: scie | P: pausa | Rotella: zoom",
        16);
    aiuto.setFillColor(sf::Color(160, 160, 160));
    aiuto.setPosition({15.f, altezza - 30.f});

    sf::Text scrittaFine(font, "GAME OVER\nInvio per ricominciare", 48);
    scrittaFine.setFillColor(sf::Color::Red);
    {
        const sf::FloatRect b = scrittaFine.getLocalBounds();
        scrittaFine.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
        scrittaFine.setPosition({larghezza / 2.f, altezza / 2.f});
    }

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
                else if (key->code == sf::Keyboard::Key::R && !gameOver)
                    resetNave();
                else if (key->code == sf::Keyboard::Key::Enter && gameOver)
                    riavvia();
                else if (key->code == sf::Keyboard::Key::T)
                    scieAttive = !scieAttive;
                else if (key->code == sf::Keyboard::Key::P)
                    inPausa = !inPausa;
            }
            else if (const auto* rotella = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (rotella->delta > 0.f) zoom *= 0.9f;
                else if (rotella->delta < 0.f) zoom *= 1.1f;
                zoom = std::clamp(zoom, 0.3f, 6.f);
            }
        }

        float dt = std::min(orologio.restart().asSeconds(), 0.05f);
        if (inPausa) dt = 0.f; // in pausa il tempo di simulazione non avanza
        invulnerabile = std::max(0.f, invulnerabile - dt);

        // --- Navicella ---
        if (!gameOver) {
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

            // Sparo (non in pausa)
            ricarica -= dt;
            if (!inPausa && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && ricarica <= 0.f) {
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
        }

        // --- Proiettili ---
        for (Proiettile& p : proiettili) {
            p.velocita += accelerazioneGravita(p.forma.getPosition(), centroSole, GM) * dt;
            p.forma.move(p.velocita * dt);
            p.vita -= dt;
        }

        // --- Asteroidi ---
        for (Asteroide& a : asteroidi) {
            a.velocita += accelerazioneGravita(a.posizione, centroSole, GM) * dt;
            a.posizione += a.velocita * dt;
            a.rotazione += a.velRotazione * dt;
            if (collisione(a.posizione, a.raggio * 0.9f, centroSole, raggioSole))
                a.viva = false;
        }

        // --- Pianeti ---
        for (Pianeta& p : pianeti) {
            p.velocita += accelerazioneGravita(p.forma.getPosition(), centroSole, GM) * dt;
            p.forma.move(p.velocita * dt);
        }

        // --- Campionamento delle scie (ogni 50 ms di tempo di simulazione) ---
        timerScia += dt;
        if (timerScia >= intervalloScia) {
            timerScia = 0.f;
            if (!gameOver)
                aggiungiPunto(sciaNave, nave.getPosition(), 120);
            for (Pianeta& p : pianeti)
                aggiungiPunto(p.scia, p.forma.getPosition(), 200);
        }

        // --- Collisioni ---
        std::vector<Asteroide> nuovi;

        for (Proiettile& p : proiettili) {
            for (Asteroide& a : asteroidi) {
                if (a.viva && collisione(p.forma.getPosition(), raggioProiettile,
                                         a.posizione, a.raggio * 0.9f)) {
                    a.viva = false;
                    p.vita = 0.f;
                    if (a.raggio >= 30.f) punteggio += 20;
                    else if (a.raggio >= 20.f) punteggio += 50;
                    else punteggio += 100;
                    if (a.raggio >= 22.f) {
                        nuovi.push_back(creaFrammento(a, 1.f, gen));
                        nuovi.push_back(creaFrammento(a, -1.f, gen));
                    }
                    break;
                }
            }
        }

        if (!gameOver && invulnerabile <= 0.f) {
            for (Asteroide& a : asteroidi) {
                if (a.viva && collisione(nave.getPosition(), raggioNave,
                                         a.posizione, a.raggio * 0.9f)) {
                    a.viva = false;
                    perdiVita();
                    break;
                }
            }
            if (!gameOver && collisione(nave.getPosition(), raggioNave, centroSole, raggioSole))
                perdiVita();
        }

        // --- Rimozione e aggiunta ---
        proiettili.erase(
            std::remove_if(proiettili.begin(), proiettili.end(),
                           [](const Proiettile& p) { return p.vita <= 0.f; }),
            proiettili.end());
        asteroidi.erase(
            std::remove_if(asteroidi.begin(), asteroidi.end(),
                           [](const Asteroide& a) { return !a.viva; }),
            asteroidi.end());
        for (Asteroide& f : nuovi)
            asteroidi.push_back(std::move(f));

        if (asteroidi.empty())
            for (int i = 0; i < numAsteroidi; ++i)
                asteroidi.push_back(creaAsteroide());

        // --- Camera ---
        view.setCenter(nave.getPosition());
        view.setSize(dimensioneBase * zoom);
        window.setView(view);

        // --- Disegno del mondo ---
        window.clear(sf::Color::Black);
        window.draw(stelle);

        // Le scie vanno dietro agli oggetti, quindi si disegnano per prime
        if (scieAttive) {
            for (const Pianeta& p : pianeti)
                disegnaScia(window, p.scia, p.forma.getFillColor());
            disegnaScia(window, sciaNave, sf::Color(120, 200, 255));
        }

        window.draw(sole);
        for (const Pianeta& p : pianeti)
            window.draw(p.forma);
        for (const Asteroide& a : asteroidi) {
            sf::Transform t;
            t.translate(a.posizione);
            t.rotate(sf::degrees(a.rotazione));
            sf::RenderStates stati;
            stati.transform = t;
            window.draw(a.forma, stati);
        }
        for (const Proiettile& p : proiettili)
            window.draw(p.forma);
        if (!gameOver && (invulnerabile <= 0.f || std::fmod(invulnerabile, 0.2f) < 0.1f))
            window.draw(nave);

        // --- Disegno dell'interfaccia (vista di default) ---
        window.setView(window.getDefaultView());
        std::string testo = "Punteggio: " + std::to_string(punteggio) +
                            "    Vite: " + std::to_string(vite);
        if (inPausa) testo += "    [PAUSA]";
        hud.setString(testo);
        window.draw(hud);
        window.draw(aiuto);
        if (gameOver)
            window.draw(scrittaFine);

        window.display();
    }
}