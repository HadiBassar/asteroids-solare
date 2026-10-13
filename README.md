Compilation:

```
cmake -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

`Release` can be changed to `Debug` if necessary.
Delta time: dt è il tempo trascorso dall’ultimo frame. Muovere con velocità \* dt fa girare la navicella alla stessa velocità (180 gradi al secondo) su un PC lento o veloce. Se ruotassi di un valore fisso per frame, dipenderebbe dal framerate.

L’origine e la rotazione: i punti del triangolo sono attorno a (0,0), quindi la rotazione avviene attorno al centro della navicella. Se i punti fossero lontani dall’origine, la navicella girerebbe come su un perno esterno. È lo stesso concetto della slide 02 (setOrigin, trasformazioni).

La punta verso l’alto: con rotazione 0 la navicella guarda su. Nella prossima tappa ci servirà per calcolare la direzione della spinta con seno e coseno.

Vettore velocità: la velocità non è un numero ma un vettore (x, y). Ogni frame le sommi accelerazione \* dt nella direzione della spinta. Nessuna spinta significa velocità costante, cioè inerzia.

Angolo in direzione: con rotazione a, il vettore di puntamento è (sin a, -cos a). A 0 gradi è (0, -1), cioè verso l’alto (nello schermo la y cresce verso il basso). A 90 gradi diventa (1, 0), verso destra.

Integrazione di Eulero: posizione += velocita _ dt e velocita += accelerazione _ dt. È il metodo più semplice per simulare il moto. Nella Tappa 04 lo userai per la gravità.

Delta time: anche qui tutto è moltiplicato per dt, quindi il comportamento non dipende dal framerate.

Il wrap-around ai bordi è provvisorio: lo togliamo alla Tappa 06, quando la camera segue la navicella.

Legge di gravitazione: la forza è G m1 m2 / r². L’accelerazione del pianeta è F / m2 = G m1 / r², quindi la massa del pianeta si semplifica e conta solo la massa del Sole, che nel codice è compresa in GM.

Direzione dell’accelerazione: il vettore d = centro - pos punta verso il Sole. Dividendo per r diventa unitario (lunghezza 1), e lo moltiplichi per il modulo GM / r². Questa è la normalizzazione di un vettore.

Orbita circolare: serve una velocità perpendicolare al raggio con modulo sqrt(GM / r). Se la velocità è diversa, l’orbita diventa un’ellisse o un’iperbole.

Eulero semi-implicito: se aggiornassi la posizione con la velocità vecchia, il pianeta guadagnerebbe energia a ogni giro e si allontanerebbe a spirale. Aggiornando prima la velocità si mantiene l’orbita stabile. Puoi provare a invertire le due righe e vedere che succede.

Limite su dt: std::min(..., 0.05f) evita che un frame molto lungo faccia fare alla simulazione un passo troppo grande

struct e std::vector: raccogli i dati di un pianeta in una struttura e tieni i pianeti in un vettore, così lo stesso codice li aggiorna tutti con un ciclo for. Per aggiungere un pianeta basta una riga.

La massa si semplifica: a = F/m = G M / r² non dipende dalla massa del corpo che cade. Per questo la stessa funzione serve per pianeti e navicella.

Velocità perpendicolare: per il pianeta usi (-dir.y, dir.x), cioè il vettore dir ruotato di 90 gradi. È una rotazione 2D, che nella teoria si scrive con la matrice di rotazione (slide 02).

Distanza minima (softening): rEff = max(r, 25) evita che la forza diventi enorme quando passi vicino al Sole. È una scelta numerica, non fisica, e conviene spiegarla così.

Lambda resetNave: una funzione locale che usa le variabili dello scope ([&]). Se non l’hai mai usata, leggila con calma.

Coordinate del mondo e della finestra: gli oggetti hanno posizioni in coordinate del mondo. La View definisce quale rettangolo del mondo è visibile. SFML poi lo trasforma in pixel della finestra. Muovere la camera significa solo cambiare il centro della View, senza toccare gli oggetti (slide 02).

Zoom con la dimensione: una View più grande mostra più mondo nello stesso schermo, quindi tutto appare più piccolo (zoom out). Per questo zoom > 1 allontana. Moltiplicare zoom per 0.9 o 1.1 dà uno zoom proporzionale e regolare.

std::clamp: limita lo zoom tra 0.3 e 6, così non puoi ingrandire all’infinito.

window.setView(view) prima del disegno: tutto quello che disegni dopo usa la camera. Nella Tappa 09, per il testo a schermo (punteggio e vite), dovrai tornare alla vista di default con window.setView(window.getDefaultView()), altrimenti la scritta si muoverebbe con la camera.

Dimensione fissa: se ridimensioni la finestra, l’immagine si deforma. Per un progetto di queste dimensioni è accettabile, ma se vuoi puoi ricordartelo come limite nella relazione.

Somma di velocità: velocità del proiettile = velocità della nave + direzione \* velocitaProiettile. È la composizione dei moti: senza la somma, sparando in movimento il proiettile sembrerebbe innaturale.

Punto di nascita: posizione + direzione \* 20 mette il proiettile sulla punta (la punta è a 20 unità dal centro).

Tempo di vita: ogni proiettile ha un contatore che scende con dt. Quando arriva a 0, viene eliminato, altrimenti il vettore crescerebbe all’infinito e il gioco rallenterebbe.

Idioma erase-remove: remove_if sposta in fondo gli elementi da eliminare e erase li cancella davvero. Non si può cancellare dentro un ciclo for sullo stesso vettore senza invalidare gli iteratori. Te lo possono chiedere come “perché non hai usato erase dentro il ciclo”.

Cooldown: il timer ricarica limita la frequenza di sparo. Senza di esso, a 60 fps spareresti 60 proiettili al secondo.

Lambda in remove_if: [](const Proiettile& p) { return p.vita <= 0.f; } è il criterio per decidere cosa eliminare.

Collisione cerchio-cerchio: due cerchi si toccano se la distanza tra i centri è minore della somma dei raggi. Nel codice si confrontano i quadrati (dx² + dy² < somma²) per evitare la radice quadrata, che è costosa. Il risultato è identico.

Approssimazione con cerchio: asteroide e navicella non sono cerchi, ma li trattiamo come tali. Il 0.9f sul raggio dell’asteroide compensa il fatto che il poligono è un po’ più piccolo del cerchio. Si chiama bounding circle: è una verifica approssimata ma molto economica.

Coordinate locali e coordinate del mondo: i vertici dell’asteroide sono attorno a (0,0). La posizione e la rotazione sono dati separati, applicati al disegno con una sf::Transform. È la matrice di modellazione (slide 02 e 04).

Ordine delle trasformazioni: translate poi rotate significa che il poligono viene prima ruotato attorno al suo centro e poi traslato. Se invertissi l’ordine, ruoterebbe attorno all’origine del mondo, cioè attorno a un punto lontano. Questa domanda è molto probabile all’orale.

TriangleFan: un ventaglio di triangoli che condividono il primo vertice (il centro). Funziona perché il poligono è “a stella” rispetto al centro. Il centro è più scuro e il bordo più chiaro, e SFML interpola i colori tra i vertici.

Marcare e poi rimuovere: durante i cicli non si cancella niente (viva = false). Si rimuove tutto alla fine con erase-remove, come per i proiettili.

Camera e interfaccia: il mondo si disegna con la View della camera, il testo con window.getDefaultView(). Se disegnassi il punteggio con la camera attiva, si muoverebbe nel mondo e uscirebbe dallo schermo. È un esempio diretto di due sistemi di riferimento diversi: mondo e schermo (slide 02).

Frammenti: la velocità del frammento è la velocità della madre più una componente perpendicolare. Il vettore perpendicolare si ottiene ruotando di 90 gradi: (-y, x). Con segno +1 e -1 i due pezzi vanno in versi opposti.

Perché nuovi: non si aggiungono elementi a un std::vector mentre lo scorri con un ciclo, perché lo spazio in memoria può spostarsi e i riferimenti diventano invalidi. Si accumulano da parte e si aggiungono dopo.

Testo in SFML 3: sf::Text richiede il font nel costruttore. getLocalBounds() restituisce un rettangolo (position e size), che si usa per centrare il testo impostando l’origine a metà.
Ricerca del font: caricaFont prova tre percorsi relativi, perché la cartella da cui lanci l’eseguibile cambia (radice, build, build\Debug). Non c’è nessun percorso assoluto, come richiede il progetto.

Invulnerabilità: senza di essa, dopo il reset la navicella potrebbe finire subito contro un altro asteroide e perdere tutte le vite in pochi secondi. Il lampeggio usa fmod(tempo, 0.2) < 0.1, cioè un’onda quadra.

std::deque: una coda a due estremità. Aggiungi i punti nuovi in fondo con push_back e togli i più vecchi davanti con pop_front, in modo efficiente. Con un vector togliere il primo elemento sposterebbe tutti gli altri.

Sfumatura con l’alpha: ogni vertice della linea ha un colore con trasparenza diversa (t², quindi sfuma più in fretta in coda). SFML interpola i colori tra i vertici di un segmento, esattamente come interpola il colore dentro un triangolo. È l’interpolazione lineare dei vertici (slide 06, rasterizzazione).

Ordine di disegno: le scie si disegnano prima di pianeti e navicella, così restano sotto. Nella grafica 2D vale l’algoritmo del pittore: l’ultimo disegnato copre i precedenti (slide 06 lo confronta con lo z-buffer del 3D).

Pausa con dt = 0: tutta la simulazione si muove con dt, quindi azzerandolo si ferma ogni cosa, e basta una riga. Il campionamento delle scie usa lo stesso dt, perciò in pausa non aggiunge punti.

Scia della navicella azzerata: quando la navicella viene riportata in orbita “salta” in un altro punto, e senza clear() si vedrebbe una linea dritta tra la vecchia e la nuova posizione.

# Asteroids nel sistema solare

Gioco 2D in C++17 con SFML 3.0 per l'esame di Fondamenti di Computer Grafica.
Si pilota una navicella in un sistema solare con gravità newtoniana e si
sparano gli asteroidi. Il progetto è diviso in 10 tappe (cartelle `Tappa01` ... `Tappa10`).

## Requisiti

CMake 4.0 o più recente, un compilatore C++17 (Visual Studio, GCC o Clang), Git
e una connessione a internet (CMake scarica SFML 3.0.2 al primo avvio).

## Build di tutte le tappe

```
cmake -B build
cmake --build build --config Debug
```

## Esecuzione

Lanciare dalla radice del progetto (così trovano `risorse/`):

- Windows: `.\build\Debug\Tappa10.exe` (cambiare 10 con il numero della tappa)
- Linux e macOS: `./build/Tappa10`

Nessuna tappa richiede argomenti da riga di comando.

## Comandi

| Comando                  | Effetto                         | Dalla tappa |
| ------------------------ | ------------------------------- | ----------- |
| Frecce sinistra e destra | Ruotano la navicella            | 02          |
| Freccia su               | Spinta                          | 03          |
| R                        | Riporta la navicella in orbita  | 05          |
| Rotella del mouse        | Zoom                            | 06          |
| Spazio                   | Spara                           | 07          |
| Invio                    | Nuova partita dopo il game over | 09          |
| T                        | Attiva o disattiva le scie      | 10          |
| P                        | Pausa                           | 10          |
| Esc                      | Esci                            | 01          |

## Tappe

01 finestra e Sole · 02 navicella che ruota · 03 spinta e inerzia · 04 gravità e primo pianeta ·
05 più pianeti e gravità sulla navicella · 06 camera e zoom · 07 proiettili ·
08 asteroidi e collisioni · 09 frammenti, punteggio e vite · 10 scie e pausa

## Fonti

Il template CMake deriva dal Laboratorio 3 dei docenti. Il font Arial.ttf viene dai materiali dei docenti.
Il codice è stato scritto con l'aiuto di Claude (Anthropic). Dettagli in `relazione.md`.
