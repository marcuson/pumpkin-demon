# Pumpkin Demon — firmware di prova

Progetto PlatformIO per Arduino Nano Every. Il firmware prova due gruppi LED
indipendenti tramite i primi due canali dell'ULN2003A.

## Collegamenti sulla breadboard

| Nano Every | ULN2003A DIP-16 |
| --- | --- |
| D3 | pin 1 (IN1) |
| D5 | pin 2 (IN2) |
| GND | pin 8 (GND) |

- Pin 16 (OUT1) → catodi dei LED del gruppo 1.
- Pin 15 (OUT2) → catodi dei LED del gruppo 2.
- Pin 9 (COM) → non collegato.
- Per **ogni** LED: `5V → resistenza da 220 Ω → anodo LED → catodo LED → uscita ULN2003A`.
- Per questa prova con pochi LED, porta il 5V della scheda al rail positivo
  della breadboard e GND al rail negativo. Alimenta il Nano dalla presa USB.

Con la tacca del chip verso l'alto, i pin 1 e 2 sono i primi due a sinistra;
i pin 16 e 15 sono i primi due a destra.

## Prova

Apri questa cartella come progetto PlatformIO e usa **Build** e **Upload** con
il Nano Every collegato via USB. Da terminale, se `pio` è disponibile:

```sh
pio run
pio run -t upload
```

Il ciclo dura 5 secondi: gruppo 1 acceso, gruppo 2 acceso, entrambi a metà
luminosità, entrambi al massimo, entrambi spenti. Ogni fase dura 1 secondo.

## Log seriali

Dopo aver caricato il firmware aggiornato, apri il **Serial Monitor** di
PlatformIO (115200 baud), oppure esegui `pio device monitor`.
Il firmware stampa un messaggio di avvio e una riga per ogni fase con i valori
PWM comandati su D3 e D5 (0 = spento, 255 = massimo).
Se apri il monitor dopo l'avvio, vedrai comunque le fasi ripetersi; premi RESET
con il monitor aperto per rileggere anche i messaggi iniziali.

I log confermano l'esecuzione del firmware e i comandi inviati, non la corrente
nei LED. Se le fasi scorrono ma i LED restano spenti, occorre verificare
alimentazione, massa comune, orientamento dei LED e collegamenti dell'ULN2003A.
