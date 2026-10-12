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
