# Open Airsoft Countdown

[🇮🇹 Versione italiana](#italiano) | [🇬🇧 English version](#english)

---

<a id="italiano"></a>

# 🇮🇹 Open Airsoft Countdown

**Open Airsoft Countdown** è un timer countdown open source basato su ESP32-S3, pensato per softair, laser tag, escape room e giochi di ruolo.

Il progetto è destinato esclusivamente all’intrattenimento e all’uso scenico.  
Non controlla, attiva o interagisce con dispositivi esplosivi, pirotecnici, incendiari o pericolosi reali.

[Vai alla versione inglese](#english)

## Stato del progetto

🚧 In sviluppo

Il firmware è attualmente in fase di sviluppo.  
I moduli principali vengono implementati uno alla volta:

- Storage
- Configurazione JSON
- Display OLED
- Timer countdown
- Tastierino I2C
- Lettore NFC
- Buzzer
- LED di stato

Il supporto BLE e l’app Android verranno aggiunti in seguito.

## Funzionalità principali

Funzionalità previste:

- Firmware basato su ESP32-S3
- Display OLED 1.3" SH1106
- Lettore NFC PN532
- Tastierino 4x4 tramite I2C
- Timer countdown
- PIN amministratore all’avvio
- Impostazione del timer da tastierino
- Impostazione del timer da smartphone tramite BLE
- Autenticazione utente tramite card NFC + PIN a 6 cifre
- LED di stato che lampeggia ogni secondo durante il countdown
- LED di stato acceso fisso quando il timer termina
- Feedback acustico tramite buzzer
- Configurazione JSON salvata su LittleFS
- Firmware e documentazione open source

## Hardware

Hardware previsto:

| Componente | Descrizione |
|---|---|
| MCU | ESP32-S3 DevKitC-1 |
| Display | OLED 1.3" SH1106 128x64 I2C |
| Lettore NFC | PN532 I2C |
| Tastierino | 4x4 tramite espansore I2C |
| LED di stato | LED standard con resistenza |
| Buzzer | Buzzer attivo |
| Storage | Flash interna ESP32 tramite LittleFS |

## Pinout attuale

Pinout temporaneo usato durante lo sviluppo:

| Funzione | GPIO ESP32-S3 |
|---|---:|
| I2C SDA | GPIO 8 |
| I2C SCL | GPIO 9 |
| PN532 IRQ | GPIO 10 |
| PN532 RESET | GPIO 11 |
| LED di stato | GPIO 2 |
| Buzzer | GPIO 4 |

Questo pinout potrebbe cambiare prima della prima release stabile.

## Configurazione

Il dispositivo salva la configurazione in LittleFS usando file JSON.

### `config.json`

Configurazione predefinita:

```json
{
  "adminPin": "000000",
  "bleName": "Open Airsoft Countdown",
  "soundEnabled": true
}
```

### `users.json`

Database utenti predefinito:

```json
[]
```

Ogni futuro utente sarà salvato così:

```json
{
  "id": 1,
  "name": "Player",
  "uid": "04AABBCCDDEE",
  "pin": "123456"
}
```

Il PIN non deve essere univoco.  
L’UID della card NFC deve invece essere univoco.

L’autenticazione è valida solo quando la card NFC e il PIN appartengono allo stesso utente.

## Flusso di avvio amministratore

Quando il dispositivo si accende:

1. Chiede il PIN amministratore.
2. Il PIN amministratore predefinito è `000000`.
3. Dopo il login amministratore, chiede la durata del countdown.
4. La durata viene inserita nel formato:

```text
HHMMSS
```

Esempi:

| Input | Significato |
|---|---|
| `000030` | 30 secondi |
| `000130` | 1 minuto e 30 secondi |
| `001500` | 15 minuti |
| `010000` | 1 ora |

Comandi del tastierino:

| Tasto | Azione |
|---|---|
| `0-9` | Inserisce cifre |
| `*` | Cancella input |
| `#` | Conferma |

## Comportamento del timer

Durante il countdown:

- Il tempo rimanente viene mostrato sul display OLED.
- Il LED di stato cambia stato ogni secondo.
- Il buzzer emette un beep durante gli ultimi 5 secondi.
- Quando il timer arriva a zero:
  - il buzzer suona;
  - il LED di stato rimane acceso fisso.

## Comportamento dell’autenticazione

Durante il gioco, il countdown può essere fermato solo autenticandosi con:

1. Una card NFC valida.
2. Il PIN a 6 cifre associato a quella stessa card.

Dopo errori nell’inserimento del PIN:

- 1° PIN errato: contatore errori `1/3`
- 2° PIN errato: contatore errori `2/3`
- 3° PIN errato:
  - se il tempo rimanente è maggiore di 5 secondi, viene ridotto a 5 secondi;
  - se il tempo rimanente è già pari o inferiore a 5 secondi, non viene modificato;
  - il display mostra:

```text
Numero massimo di errori raggiunto.
Detonazione.
```

## Firmware

Il firmware è sviluppato con:

- PlatformIO
- Arduino framework
- C++17

Librerie principali:

- U8g2
- ArduinoJson
- LittleFS
- Adafruit PN532
- Keypad
- NimBLE-Arduino

## Istruzioni di compilazione

Installa:

- Visual Studio Code
- Estensione PlatformIO IDE
- Git

Clona la repository:

```bash
git clone https://github.com/MaydayAlaska/Open-Airsoft-Countdown.git
cd Open-Airsoft-Countdown
```

Compila il firmware:

```bash
pio run
```

Carica il firmware su ESP32-S3:

```bash
pio run --target upload
```

Apri il monitor seriale:

```bash
pio device monitor
```

Velocità seriale predefinita:

```text
115200
```

## Struttura della repository

```text
Open-Airsoft-Countdown/
├── include/
├── src/
├── lib/
├── test/
├── platformio.ini
├── README.md
├── LICENSE
└── .gitignore
```

La struttura potrebbe evolversi durante lo sviluppo.

## Licenza

Questo progetto è distribuito con licenza **GNU Affero General Public License v3.0**.

Puoi usare, modificare e condividere questo progetto, ma le versioni modificate devono rimanere open source sotto la stessa licenza.

Consulta il file `LICENSE` per i dettagli.

## Disclaimer

Questo progetto è un controller scenico per softair, laser tag, escape room e attività di intrattenimento simili.

Non deve essere usato con dispositivi esplosivi, pirotecnici, incendiari o pericolosi reali.

Gli autori non sono responsabili per usi impropri, pericolosi o illegali del progetto.

---

<a id="english"></a>

# 🇬🇧 Open Airsoft Countdown

**Open Airsoft Countdown** is an open-source ESP32-S3 countdown timer designed for airsoft, laser tag, escape rooms and role-play props.

The project is intended only for entertainment and game scenarios.  
It does not control, trigger or interact with any real pyrotechnic, explosive or dangerous device.

[Go to Italian version](#italiano)

## Project status

🚧 Work in progress

The firmware is currently under development.  
Basic modules are being implemented one by one:

- Storage
- JSON configuration
- OLED display
- Countdown timer
- I2C keypad input
- NFC reader
- Buzzer
- Status LED

BLE support and the Android app will be added later.

## Main features

Planned features:

- ESP32-S3 based firmware
- OLED 1.3" SH1106 display
- PN532 NFC reader
- 4x4 keypad via I2C
- Countdown timer
- Admin PIN at startup
- Timer setup from keypad
- Timer setup from smartphone app via BLE
- User authentication with NFC card + 6-digit PIN
- Status LED blinking once per second during countdown
- Status LED always on when the timer ends
- Buzzer feedback
- JSON configuration stored in LittleFS
- Open-source firmware and documentation

## Hardware

Current target hardware:

| Component | Description |
|---|---|
| MCU | ESP32-S3 DevKitC-1 |
| Display | 1.3" OLED SH1106 128x64 I2C |
| NFC reader | PN532 I2C |
| Keypad | 4x4 keypad via I2C expander |
| Status LED | Standard LED with resistor |
| Buzzer | Active buzzer |
| Storage | ESP32 internal flash using LittleFS |

## Current pinout

Temporary pinout used during development:

| Function | ESP32-S3 GPIO |
|---|---:|
| I2C SDA | GPIO 8 |
| I2C SCL | GPIO 9 |
| PN532 IRQ | GPIO 10 |
| PN532 RESET | GPIO 11 |
| Status LED | GPIO 2 |
| Buzzer | GPIO 4 |

This pinout may change before the first stable release.

## Configuration

The device stores its configuration in LittleFS using JSON files.

### `config.json`

Default configuration:

```json
{
  "adminPin": "000000",
  "bleName": "Open Airsoft Countdown",
  "soundEnabled": true
}
```

### `users.json`

Default user database:

```json
[]
```

Each future user entry will contain:

```json
{
  "id": 1,
  "name": "Player",
  "uid": "04AABBCCDDEE",
  "pin": "123456"
}
```

The PIN does not need to be unique.  
The NFC UID must be unique.

Authentication is valid only when the NFC card and PIN belong to the same user.

## Admin startup flow

When the device starts:

1. It asks for the admin PIN.
2. The default admin PIN is `000000`.
3. After successful admin login, it asks for the countdown duration.
4. The timer duration is entered as:

```text
HHMMSS
```

Examples:

| Input | Meaning |
|---|---|
| `000030` | 30 seconds |
| `000130` | 1 minute and 30 seconds |
| `001500` | 15 minutes |
| `010000` | 1 hour |

Keypad controls:

| Key | Action |
|---|---|
| `0-9` | Enter digits |
| `*` | Clear input |
| `#` | Confirm |

## Timer behavior

During countdown:

- The remaining time is shown on the OLED display.
- The status LED toggles every second.
- The buzzer beeps during the last 5 seconds.
- When the timer reaches zero:
  - the buzzer sounds;
  - the status LED remains on.

## Authentication behavior

During the game, the countdown can be stopped only by authenticating with:

1. A valid NFC card.
2. The correct 6-digit PIN associated with that same card.

After wrong PIN attempts:

- 1st wrong PIN: error count becomes `1/3`
- 2nd wrong PIN: error count becomes `2/3`
- 3rd wrong PIN:
  - if the remaining time is greater than 5 seconds, it is reduced to 5 seconds;
  - if the remaining time is already 5 seconds or less, it is not changed;
  - the display shows:

```text
Numero massimo di errori raggiunto.
Detonazione.
```

## Firmware

The firmware is developed with:

- PlatformIO
- Arduino framework
- C++17

Main libraries:

- U8g2
- ArduinoJson
- LittleFS
- Adafruit PN532
- Keypad
- NimBLE-Arduino

## Build instructions

Install:

- Visual Studio Code
- PlatformIO IDE extension
- Git

Clone the repository:

```bash
git clone https://github.com/MaydayAlaska/Open-Airsoft-Countdown.git
cd Open-Airsoft-Countdown
```

Build the firmware:

```bash
pio run
```

Upload to the ESP32-S3:

```bash
pio run --target upload
```

Open the serial monitor:

```bash
pio device monitor
```

Default serial speed:

```text
115200
```

## Repository structure

```text
Open-Airsoft-Countdown/
├── include/
├── src/
├── lib/
├── test/
├── platformio.ini
├── README.md
├── LICENSE
└── .gitignore
```

The structure may evolve as the project grows.

## License

This project is licensed under the **GNU Affero General Public License v3.0**.

You are free to use, modify and share this project, but modified versions must remain open source under the same license.

See the `LICENSE` file for details.

## Disclaimer

This project is a game prop controller for airsoft, laser tag, escape rooms and similar entertainment activities.

It must not be used with real explosive, pyrotechnic, incendiary or hazardous devices.

The authors are not responsible for improper, unsafe or illegal use of this project.