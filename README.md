<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/MaydayAlaska/Open-Airsoft-Countdown-Android-App/main/app/src/main/res/drawable-nodpi/info_logo_dark.png">
  <source media="(prefers-color-scheme: light)" srcset="https://raw.githubusercontent.com/MaydayAlaska/Open-Airsoft-Countdown-Android-App/main/app/src/main/res/drawable-nodpi/info_logo_light.png">
  <img src="https://raw.githubusercontent.com/MaydayAlaska/Open-Airsoft-Countdown-Android-App/main/app/src/main/res/drawable-nodpi/info_logo_light.png" alt="Open Airsoft Countdown" width="190">
</picture>

# Open Airsoft Countdown

Timer scenico open source basato su ESP32-S3 per softair, laser tag, escape room e giochi a obiettivi.
Open-source ESP32-S3 game-prop countdown timer for airsoft, laser tag, escape rooms, and objective-based games.

**Firmware corrente: v1.9.1**

[Italiano](#italiano) · [English](#english)

</div>

---

# Italiano

> **Solo per intrattenimento.** Il progetto non è progettato per controllare esplosivi, dispositivi pirotecnici o sistemi reali pericolosi.

## Installa il firmware

Il modo più semplice è il [Web Installer](https://maydayalaska.github.io/Open-Airsoft-Countdown/): collega la scheda con un cavo USB dati e usa un browser desktop compatibile con Web Serial, come Chrome o Edge. Non servono Visual Studio Code né la compilazione manuale.

Per compilare o caricare da sorgente, consulta [Compilazione con PlatformIO](#compilazione-con-platformio).

## Funzionalità

- timer countdown impostabile in formato `HHMMSS` tramite tastierino 4×4 o BLE;
- accesso iniziale protetto da PIN amministratore a 6 cifre;
- OLED SH1106 con interfaccia in italiano o inglese;
- LED di stato lampeggiante durante il countdown e buzzer configurabile, compresi i beep negli ultimi 5 secondi;
- disarmo con PIN utente e, se abilitato, autenticazione NFC con PN532;
- modalità con uno qualunque o tutti gli utenti autorizzati;
- gestione di utenti, configurazione e timer da Bluetooth Low Energy;
- dati persistenti in LittleFS e migrazione automatica della configurazione precedente;
- app Android dedicata con login, controllo timer, configurazione e gestione utenti.

## Novità di v1.9.1

- flusso NFC ridisegnato: durante il countdown il lettore seleziona prima l'utente dalla tessera, poi il PIN deve appartenere a quello stesso utente;
- il PN532 ascolta soltanto quando serve al disarmo e gestisce il rilascio della tessera, le letture duplicate e i tentativi di recupero del lettore;
- la fine del countdown viene gestita prima di qualsiasi nuovo input: il timer non può più essere disarmato nell'istante in cui scade;
- messaggio esplicito per un tempo non valido e saluto utente senza bloccare l'inserimento del PIN;
- correzioni di affidabilità per il login amministrativo BLE e per l'aggiornamento dello stato nell'app;
- bus I²C separati per OLED e PN532, così da evitare conflitti elettrici.

## Hardware e pinout

| Componente | Modello / tipo | Collegamenti GPIO |
|---|---|---|
| Microcontrollore | ESP32-S3-DevKitC-1 | — |
| Display | OLED SH1106 I²C 128×64 | SDA 8, SCL 9 |
| Lettore NFC opzionale | PN532 I²C | SDA 1, SCL 2, IRQ 10 |
| Tastierino | Matrice 4×4 | R1 18, R2 17, R3 16, R4 15; C1 7, C2 6, C3 5, C4 4 |
| Buzzer | Buzzer attivo | 21 |
| LED | LED di stato | 14 |

Collega alimentazione e massa di OLED e PN532 rispettivamente a `3V3` e `GND`. OLED e PN532 usano due controller I²C distinti: non collegare il bus del PN532 ai pin 8/9 dell'OLED.

## Utilizzo dal tastierino

1. Attendi la schermata iniziale, quindi inserisci il PIN amministratore di 6 cifre e premi `#`.
2. Inserisci il tempo in `HHMMSS`, ad esempio `001530` per 15 minuti e 30 secondi, quindi premi `#`.
3. Durante il countdown inserisci il PIN di un utente autorizzato e premi `#`.
4. Con `rfid=true`, presenta prima la tessera NFC dell'utente, inserisci il suo PIN e conferma con `#`.
5. Un'autenticazione valida arresta il timer. Se sono richiesti più utenti, il timer si arresta dopo l'autenticazione di tutti loro.

`*` cancella l'ultima cifra durante l'inserimento. Al termine naturale del countdown, premi `#` per tornare all'impostazione del timer.

## Configurazione e utenti

Le impostazioni sono salvate in LittleFS in `/config.json`:

```json
{
  "adminPin": "000000",
  "bleName": "Open Airsoft Countdown",
  "language": "it",
  "authorizedUserIds": "1",
  "soundEnabled": true,
  "rfid": false,
  "fingerprint": false,
  "maxErrorCount": 3,
  "errorCountdownSeconds": 10
}
```

| Campo | Descrizione |
|---|---|
| `adminPin` | PIN amministratore, esattamente 6 cifre |
| `bleName` | Nome BLE del dispositivo |
| `language` | `it` oppure `en` |
| `authorizedUserIds` | Utenti autorizzati al disarmo |
| `soundEnabled` | Abilita tutti i suoni del buzzer |
| `rfid` | Richiede la tessera NFC associata allo stesso utente del PIN |
| `fingerprint` | Campo riservato: il lettore di impronte non è ancora implementato |
| `maxErrorCount` | Errori massimi, da 1 a 10 |
| `errorCountdownSeconds` | Tempo della penalità, da 0 a 3600 secondi |

Per `authorizedUserIds`, usa `1;2;3` se basta uno degli utenti indicati; usa `1,2,3` se devono autenticarsi tutti, in qualsiasi ordine (massimo quattro). I separatori non possono essere mescolati e tutti gli ID devono esistere.

Dopo `maxErrorCount` errori, il disarmo viene bloccato. Se `errorCountdownSeconds` è maggiore di zero, il tempo rimanente viene ridotto a quel valore dopo la schermata di allarme; `0` disattiva soltanto la riduzione del tempo.

Gli utenti sono salvati in `/users.json`. Se il file è assente o vuoto viene creato l'utente iniziale seguente:

```json
[{ "id": 1, "name": "Default (delete)", "uid": "00000000", "pin": "000000" }]
```

Sostituiscilo appena possibile e aggiorna `authorizedUserIds`. Ogni utente ha ID univoco, nome fino a 32 caratteri, PIN di 6 cifre e UID NFC esadecimale univoco fino a 32 caratteri.

> Non impostare `fingerprint` su `true`: finché il modulo non sarà supportato, il disarmo da tastierino non potrà essere completato.

## Bluetooth Low Energy e app Android

L'[app Android](https://github.com/MaydayAlaska/Open-Airsoft-Countdown-Android-App) consente di cercare e collegare il dispositivo, effettuare il login amministrativo, controllare il timer, leggere lo stato, modificare configurazione e utenti. Il login BLE viene chiuso alla disconnessione.

| Comando | Funzione |
|---|---|
| `PING` | Verifica la comunicazione |
| `LOGIN:<pin>` / `LOGOUT` | Apre o chiude la sessione amministrativa |
| `STATUS` | Legge lo stato del timer |
| `SETTIME:<HHMMSS>` | Imposta la durata |
| `START`, `STOP`, `RESET` | Controllano il timer |
| `GETCONFIG`, `SETCONFIG:...` | Leggono e salvano la configurazione |
| `GETUSERS`, `ADDUSER:...`, `UPDATEUSER:...`, `DELUSER:<id>` | Gestiscono gli utenti |

`PING`, `STATUS`, `LOGIN` e `LOGOUT` sono sempre disponibili; gli altri comandi richiedono un login BLE valido. Non è possibile modificare configurazione o utenti mentre il timer è attivo.

## Compilazione con PlatformIO

Requisiti: Visual Studio Code con PlatformIO IDE, framework Arduino per ESP32 e un cavo USB dati.

```bash
pio run
pio run --target upload
pio device monitor
```

L'ambiente è `esp32-s3-devkitc-1`; il monitor seriale usa `115200` baud. Le dipendenze (`U8g2`, `NimBLE-Arduino`, `Adafruit PN532`, `Keypad` e `ArduinoJson`) sono definite in [platformio.ini](platformio.ini). Lo script post-build produce anche il firmware unificato per il Web Installer.

## Struttura del progetto

```text
data/       File iniziali LittleFS
docs/       Web Installer e firmware pubblicato
include/    Header C++
src/        Firmware
tools/      Script di build
```

## Segnalazioni, contributi e licenza

Apri una Issue con versione del firmware, hardware, configurazione rilevante, passaggi per riprodurre il problema e log seriale. Contributi e Pull Request sono benvenuti.

Il progetto è distribuito con licenza [GNU Affero General Public License v3.0](LICENSE).

Puoi sostenerlo con una Star su GitHub, un Like/Save/Boost su [MakerWorld](https://makerworld.com/it/@maydayalaska) o una [donazione PayPal](https://paypal.me/lorisgennarini).

---

# English

> **Entertainment only.** This project is not designed to control explosives, pyrotechnics, or other real-world hazardous systems.

## Install the firmware

The easiest option is the [Web Installer](https://maydayalaska.github.io/Open-Airsoft-Countdown/): connect the board with a USB data cable and use a Web Serial-compatible desktop browser, such as Chrome or Edge. Visual Studio Code and manual compilation are not required.

For a source build or upload, see [Building with PlatformIO](#building-with-platformio).

## Features

- `HHMMSS` countdown input through the 4×4 keypad or BLE;
- 6-digit administrator PIN at startup;
- SH1106 OLED interface in Italian or English;
- blinking status LED and configurable buzzer, including beeps in the final 5 seconds;
- user-PIN disarming and optional PN532 NFC authentication;
- any-authorized-user or all-authorized-users modes;
- Bluetooth Low Energy timer, configuration, and user administration;
- LittleFS persistent data with automatic configuration migration;
- dedicated Android app for login, timer control, configuration, and user management.

## What's new in v1.9.1

- redesigned NFC flow: during a countdown, the reader selects the user from their card first, then the PIN must belong to that same user;
- the PN532 listens only when disarming needs it and handles card release, duplicate reads, and reader recovery attempts;
- countdown completion is processed before any new input, so it cannot be disarmed at the instant it expires;
- clear invalid-duration feedback and a non-blocking user greeting while entering a PIN;
- reliability fixes for BLE administrator login and app status updates;
- separate I²C buses for the OLED and PN532, avoiding electrical conflicts.

## Hardware and pinout

| Component | Model / type | GPIO connections |
|---|---|---|
| Microcontroller | ESP32-S3-DevKitC-1 | — |
| Display | SH1106 I²C OLED, 128×64 | SDA 8, SCL 9 |
| Optional NFC reader | PN532 I²C | SDA 1, SCL 2, IRQ 10 |
| Keypad | 4×4 matrix | R1 18, R2 17, R3 16, R4 15; C1 7, C2 6, C3 5, C4 4 |
| Buzzer | Active buzzer | 21 |
| LED | Status LED | 14 |

Connect OLED and PN532 power and ground to `3V3` and `GND` respectively. They use separate I²C controllers; do not connect the PN532 bus to the OLED pins 8/9.

## Keypad operation

1. Wait for the startup screen, enter the 6-digit administrator PIN, and press `#`.
2. Enter the duration in `HHMMSS`—for example, `001530` for 15 minutes 30 seconds—then press `#`.
3. During the countdown, enter an authorized user's PIN and press `#`.
4. With `rfid=true`, present that user's NFC card first, enter their PIN, and confirm with `#`.
5. A valid authentication stops the timer. If multiple users are required, it stops once all of them have authenticated.

`*` deletes the last digit while entering data. When the countdown finishes naturally, press `#` to return to timer setup.

## Configuration and users

Settings are stored in LittleFS at `/config.json`:

```json
{
  "adminPin": "000000",
  "bleName": "Open Airsoft Countdown",
  "language": "it",
  "authorizedUserIds": "1",
  "soundEnabled": true,
  "rfid": false,
  "fingerprint": false,
  "maxErrorCount": 3,
  "errorCountdownSeconds": 10
}
```

| Field | Description |
|---|---|
| `adminPin` | Exactly 6-digit administrator PIN |
| `bleName` | BLE device name |
| `language` | `it` or `en` |
| `authorizedUserIds` | Users allowed to disarm |
| `soundEnabled` | Enables all buzzer sounds |
| `rfid` | Requires the NFC card belonging to the PIN user |
| `fingerprint` | Reserved field: fingerprint hardware is not implemented yet |
| `maxErrorCount` | Maximum errors, from 1 to 10 |
| `errorCountdownSeconds` | Penalty duration, from 0 to 3600 seconds |

For `authorizedUserIds`, use `1;2;3` when any listed user is enough; use `1,2,3` when every listed user must authenticate, in any order (up to four). Separators cannot be mixed and every ID must exist.

After `maxErrorCount` errors, disarming is locked. If `errorCountdownSeconds` is greater than zero, the remaining time is reduced to that value after the alarm screen; `0` disables only the forced time reduction.

Users are stored in `/users.json`. When it is missing or empty, this initial user is created:

```json
[{ "id": 1, "name": "Default (delete)", "uid": "00000000", "pin": "000000" }]
```

Replace it as soon as possible and update `authorizedUserIds`. Each user has a unique ID, a name up to 32 characters, a 6-digit PIN, and a unique hexadecimal NFC UID up to 32 characters.

> Do not set `fingerprint` to `true`: until its module is implemented, keypad disarming cannot be completed.

## Bluetooth Low Energy and Android app

The [Android app](https://github.com/MaydayAlaska/Open-Airsoft-Countdown-Android-App) can scan and connect to the device, authenticate an administrator, control the timer, read its status, and edit configuration and users. The BLE login session closes on disconnection.

| Command | Function |
|---|---|
| `PING` | Tests communication |
| `LOGIN:<pin>` / `LOGOUT` | Opens or closes the administrator session |
| `STATUS` | Reads timer status |
| `SETTIME:<HHMMSS>` | Sets the duration |
| `START`, `STOP`, `RESET` | Control the timer |
| `GETCONFIG`, `SETCONFIG:...` | Read and save configuration |
| `GETUSERS`, `ADDUSER:...`, `UPDATEUSER:...`, `DELUSER:<id>` | Manage users |

`PING`, `STATUS`, `LOGIN`, and `LOGOUT` are always available; all other commands require a valid BLE login. Configuration and users cannot be modified while the timer is running.

## Building with PlatformIO

Requirements: Visual Studio Code with PlatformIO IDE, the Arduino framework for ESP32, and a USB data cable.

```bash
pio run
pio run --target upload
pio device monitor
```

The environment is `esp32-s3-devkitc-1`, with serial monitor at `115200` baud. Dependencies (`U8g2`, `NimBLE-Arduino`, `Adafruit PN532`, `Keypad`, and `ArduinoJson`) are listed in [platformio.ini](platformio.ini). The post-build script also produces the merged firmware for the Web Installer.

## Project structure

```text
data/       Initial LittleFS files
docs/       Web Installer and published firmware
include/    C++ headers
src/        Firmware
tools/      Build scripts
```

## Issues, contributions, and license

Open an Issue with firmware version, hardware, relevant configuration, reproduction steps, and serial logs. Contributions and Pull Requests are welcome.

This project is released under the [GNU Affero General Public License v3.0](LICENSE).

You can support it with a GitHub Star, a Like/Save/Boost on [MakerWorld](https://makerworld.com/it/@maydayalaska), or a [PayPal donation](https://paypal.me/lorisgennarini).
