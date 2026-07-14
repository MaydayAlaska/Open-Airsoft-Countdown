<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/MaydayAlaska/Open-Airsoft-Countdown-Android-App/main/app/src/main/res/drawable-nodpi/info_logo_dark.png">
  <source media="(prefers-color-scheme: light)" srcset="https://raw.githubusercontent.com/MaydayAlaska/Open-Airsoft-Countdown-Android-App/main/app/src/main/res/drawable-nodpi/info_logo_light.png">
  <img src="https://raw.githubusercontent.com/MaydayAlaska/Open-Airsoft-Countdown-Android-App/main/app/src/main/res/drawable-nodpi/info_logo_light.png" alt="Open Airsoft Countdown" width="190">
</picture>

# Open Airsoft Countdown

Timer scenico open source basato su ESP32-S3 per softair, laser tag, escape room e giochi a obiettivi.

Open-source ESP32-S3 game prop countdown timer for airsoft, laser tag, escape rooms, and objective-based games.

**Documentazione aggiornata al firmware v1.7**  
**Documentation updated for firmware v1.7**

[Italiano](#italiano) · [English](#english)

</div>

---

# Italiano

## Descrizione

**Open Airsoft Countdown** è un dispositivo di gioco open source basato su **ESP32-S3**. Integra un timer countdown, display OLED, tastierino 4×4, autenticazione utenti, lettore NFC opzionale, buzzer, LED di stato e controllo remoto tramite Bluetooth Low Energy.

È pensato per:

- softair e airsoft;
- laser tag;
- escape room;
- giochi a obiettivi;
- scenografie e prop interattivi.

> Questo progetto è destinato esclusivamente all’intrattenimento. Non è progettato per controllare dispositivi reali, pericolosi, pirotecnici o esplosivi.

## Funzionalità attuali

- countdown impostabile nel formato `HHMMSS`;
- richiesta del PIN amministratore all’avvio;
- gestione tramite tastierino 4×4;
- display OLED SH1106 128×64;
- interfaccia del display in italiano o inglese;
- visualizzazione continua di tempo rimanente ed errori;
- buzzer configurabile;
- LED di stato lampeggiante durante il countdown;
- autenticazione utenti tramite PIN;
- autenticazione PIN + NFC quando `rfid` è attivo;
- utenti autorizzati selezionabili tramite ID;
- modalità con un solo utente sufficiente oppure più utenti obbligatori;
- saluto personalizzato per 2 secondi dopo l’autenticazione;
- penalità configurabile dopo il numero massimo di errori;
- configurazione e utenti salvati in LittleFS;
- controllo amministrativo completo tramite BLE;
- applicazione Android dedicata;
- migrazione automatica dei vecchi `config.json` con campi mancanti.

## Hardware

| Componente | Modello / tipo |
|---|---|
| Microcontrollore | ESP32-S3-DevKitC-1 |
| Display | OLED SH1106 128×64 I²C |
| Tastierino | Matrice 4×4 |
| Lettore NFC | PN532 I²C |
| Buzzer | Buzzer attivo |
| LED | LED di stato |
| Memoria dati | LittleFS interna all’ESP32-S3 |
| Controllo remoto | Bluetooth Low Energy |

## Pinout attuale

| Funzione | GPIO |
|---|---:|
| I²C SDA — OLED e PN532 | 8 |
| I²C SCL — OLED e PN532 | 9 |
| PN532 IRQ | 10 |
| PN532 RESET | 11 |
| Tastierino R1 | 18 |
| Tastierino R2 | 17 |
| Tastierino R3 | 16 |
| Tastierino R4 | 15 |
| Tastierino C1 | 7 |
| Tastierino C2 | 6 |
| Tastierino C3 | 5 |
| Tastierino C4 | 4 |
| Buzzer attivo | 21 |
| LED di stato | 2 |

OLED e PN532 condividono il bus I²C su GPIO 8 e GPIO 9.

## Flusso di utilizzo

1. All’avvio viene mostrata la schermata iniziale per 3 secondi.
2. Il dispositivo richiede il PIN amministratore di 6 cifre.
3. Dopo il login viene richiesto il tempo nel formato `HHMMSS`.
4. Premendo `#` il timer viene avviato.
5. Durante il countdown si inserisce il PIN di un utente autorizzato.
6. Se `rfid=true`, prima della conferma deve essere letta anche la tessera NFC dello stesso utente.
7. Premendo `#` vengono verificati tutti i requisiti attivi.
8. Dopo un’autenticazione valida il display mostra:

```text
SALVE
NomeUtente
```

per 2 secondi.

Il PIN amministratore non viene utilizzato per il disarmo tramite tastierino. Serve per l’accesso iniziale e per il login amministrativo BLE.

## Configurazione

Le impostazioni vengono salvate nel file LittleFS:

```text
/config.json
```

Configurazione aggiornata:

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
| `adminPin` | PIN amministratore di esattamente 6 cifre |
| `bleName` | Nome Bluetooth Low Energy del dispositivo |
| `language` | Lingua del display: `it` oppure `en` |
| `authorizedUserIds` | ID degli utenti autorizzati al disarmo |
| `soundEnabled` | Abilita o disabilita tutti i suoni del buzzer |
| `rfid` | Richiede anche la tessera NFC associata allo stesso utente |
| `fingerprint` | Campo riservato al lettore di impronte |
| `maxErrorCount` | Numero massimo di errori, da 1 a 10 |
| `errorCountdownSeconds` | Durata della penalità, da 0 a 3600 secondi |

### Lingua

```json
"language": "it"
```

Valori disponibili:

- `it` — italiano;
- `en` — inglese.

### Utenti autorizzati

Con il punto e virgola:

```json
"authorizedUserIds": "1;2;3"
```

è sufficiente che si autentichi **uno qualsiasi** degli utenti indicati.

Con la virgola:

```json
"authorizedUserIds": "1,2,3"
```

devono autenticarsi **tutti** gli utenti indicati, in qualsiasi ordine. In questa modalità sono consentiti al massimo 4 utenti.

Regole:

- sono accettati soltanto numeri, virgole e punti e virgola;
- virgola e punto e virgola non possono essere usati insieme;
- gli ID devono esistere in `users.json`;
- l’ID `0` non è valido;
- una configurazione non valida viene sostituita dal valore predefinito `"1"`.

### Penalità dopo gli errori

Quando viene raggiunto `maxErrorCount`, il display mostra per 2 secondi:

```text
TROPPI ERRORI
ALLARME
```

Il timer viene impostato internamente a:

```text
errorCountdownSeconds + 2
```

I 2 secondi aggiuntivi compensano la durata della schermata di allarme. Terminato il messaggio, il tempo visibile corrisponde al valore configurato.

Con:

```json
"errorCountdownSeconds": 0
```

la riduzione forzata del timer è disattivata. Il disarmo resta comunque bloccato dopo il numero massimo di errori.

### Impronta digitale

Il campo `fingerprint` è già presente nella configurazione, ma il lettore di impronte **non è ancora implementato** nel firmware attuale.

Non impostare:

```json
"fingerprint": true
```

finché il relativo modulo non sarà supportato, perché il disarmo tramite tastierino non potrà essere completato.

## Utenti

Gli utenti sono salvati in:

```text
/users.json
```

Esempio:

```json
[
  {
    "id": 1,
    "name": "Carlo",
    "uid": "04A1B2C3D4",
    "pin": "123456"
  }
]
```

Regole principali:

- massimo 100 utenti;
- ID interno univoco;
- nome lungo al massimo 32 caratteri;
- PIN composto da esattamente 6 cifre;
- UID NFC esadecimale lungo al massimo 32 caratteri;
- ogni UID deve essere univoco;
- quando un utente viene eliminato, il più piccolo ID libero può essere riutilizzato.

Il PIN è sempre richiesto. Quando `rfid=true`, PIN e UID devono appartenere allo stesso utente.

## Applicazione Android

L’app Android permette di:

- cercare e collegare il dispositivo tramite BLE;
- effettuare il login amministrativo;
- impostare, avviare, fermare e resettare il timer;
- leggere lo stato in tempo reale;
- modificare `config.json`;
- selezionare la lingua del firmware;
- configurare gli utenti autorizzati;
- aggiungere, modificare ed eliminare gli utenti.

Repository:

https://github.com/MaydayAlaska/Open-Airsoft-Countdown-Android-App

## Protocollo BLE

Comandi principali:

| Comando | Funzione |
|---|---|
| `PING` | Verifica la comunicazione |
| `LOGIN:<pin>` | Login amministrativo |
| `LOGOUT` | Chiude la sessione |
| `STATUS` | Richiede lo stato del timer |
| `SETTIME:<HHMMSS>` | Imposta il timer |
| `START` | Avvia il countdown |
| `STOP` | Ferma il countdown |
| `RESET` | Reimposta il timer |
| `GETCONFIG` | Legge la configurazione |
| `SETCONFIG:...` | Salva la configurazione |
| `GETUSERS` | Legge gli utenti |
| `ADDUSER:...` | Aggiunge un utente |
| `UPDATEUSER:...` | Modifica un utente |
| `DELUSER:<id>` | Elimina un utente |

`PING`, `STATUS`, `LOGIN` e `LOGOUT` sono gestiti direttamente. Tutti gli altri comandi amministrativi richiedono un login BLE valido.

## Compilazione con PlatformIO

### Requisiti software

- Visual Studio Code;
- estensione PlatformIO IDE;
- framework Arduino per ESP32;
- cavo USB dati.

### Librerie

Le dipendenze sono definite in `platformio.ini`:

- U8g2;
- NimBLE-Arduino;
- Adafruit PN532;
- Keypad;
- ArduinoJson.

### Compilazione e caricamento

Da PlatformIO:

1. apri la cartella della repository;
2. collega l’ESP32-S3;
3. seleziona l’ambiente `esp32-s3-devkitc-1`;
4. esegui **Build**;
5. esegui **Upload**;
6. apri il monitor seriale a `115200` baud.

Da terminale:

```bash
pio run
pio run --target upload
pio device monitor
```

Il file `platformio.ini` contiene attualmente:

```ini
upload_port = COM9
```

Modifica o rimuovi questa riga se la scheda utilizza una porta differente.

LittleFS viene inizializzato automaticamente. Se `config.json` o `users.json` non esistono, il firmware li crea con valori predefiniti.

## Struttura principale

```text
data/       File iniziali LittleFS
include/    Header C++
src/        Implementazione firmware
lib/        Librerie locali
test/       Test
platformio.ini
README.md
LICENSE
```

## Segnalazione bug

Usa la sezione **Issues** di questa repository per segnalare problemi o proporre miglioramenti.

Indica possibilmente:

- versione del firmware;
- configurazione hardware;
- contenuto rilevante di `config.json`;
- passaggi per riprodurre il problema;
- log del monitor seriale;
- foto o screenshot del display.

## Supporta il progetto

Puoi sostenere Open Airsoft Countdown:

- mettendo una Star alle repository GitHub;
- lasciando Like, Salva e Boost su MakerWorld;
- segnalando bug tramite GitHub Issues;
- contribuendo con codice o documentazione;
- effettuando una donazione PayPal.

- MakerWorld: https://makerworld.com/it/@maydayalaska
- PayPal: https://paypal.me/lorisgennarini

## Licenza

Questo progetto è distribuito con licenza **GNU Affero General Public License v3.0**.

Consulta il file [LICENSE](LICENSE) per i dettagli.

---

# English

## Description

**Open Airsoft Countdown** is an open-source game device based on the **ESP32-S3**. It combines a countdown timer, OLED display, 4×4 keypad, user authentication, optional NFC reader, buzzer, status LED, and Bluetooth Low Energy remote control.

It is intended for:

- airsoft;
- laser tag;
- escape rooms;
- objective-based games;
- interactive scenery and props.

> This project is intended exclusively for entertainment. It is not designed to control real, dangerous, pyrotechnic, or explosive devices.

## Current features

- countdown input in `HHMMSS` format;
- administrator PIN required at startup;
- 4×4 keypad control;
- SH1106 128×64 OLED display;
- Italian or English display interface;
- continuous remaining-time and error display;
- configurable buzzer;
- status LED blinking during the countdown;
- user authentication through PIN;
- PIN + NFC authentication when `rfid` is enabled;
- authorized users selected by ID;
- any-user or all-users authentication modes;
- personalized 2-second greeting after authentication;
- configurable penalty after the maximum error count;
- configuration and users stored in LittleFS;
- complete administrative control over BLE;
- dedicated Android application;
- automatic migration of older `config.json` files with missing fields.

## Hardware

| Component | Model / type |
|---|---|
| Microcontroller | ESP32-S3-DevKitC-1 |
| Display | SH1106 128×64 I²C OLED |
| Keypad | 4×4 matrix |
| NFC reader | PN532 I²C |
| Buzzer | Active buzzer |
| LED | Status LED |
| Data storage | Internal ESP32-S3 LittleFS |
| Remote control | Bluetooth Low Energy |

## Current pinout

| Function | GPIO |
|---|---:|
| I²C SDA — OLED and PN532 | 8 |
| I²C SCL — OLED and PN532 | 9 |
| PN532 IRQ | 10 |
| PN532 RESET | 11 |
| Keypad R1 | 18 |
| Keypad R2 | 17 |
| Keypad R3 | 16 |
| Keypad R4 | 15 |
| Keypad C1 | 7 |
| Keypad C2 | 6 |
| Keypad C3 | 5 |
| Keypad C4 | 4 |
| Active buzzer | 21 |
| Status LED | 2 |

The OLED and PN532 share the I²C bus on GPIO 8 and GPIO 9.

## Operating flow

1. The startup screen is displayed for 3 seconds.
2. The device requests the 6-digit administrator PIN.
3. After login, it requests a duration in `HHMMSS` format.
4. Pressing `#` starts the timer.
5. During the countdown, enter the PIN of an authorized user.
6. When `rfid=true`, the same user’s NFC card must also be read before confirmation.
7. Pressing `#` validates every enabled requirement.
8. After successful authentication, the display shows:

```text
HELLO
UserName
```

for 2 seconds.

The administrator PIN is not used for keypad disarming. It is used for startup access and BLE administrator login.

## Configuration

Settings are stored in the LittleFS file:

```text
/config.json
```

Current configuration:

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
| `adminPin` | Administrator PIN containing exactly 6 digits |
| `bleName` | Bluetooth Low Energy device name |
| `language` | Display language: `it` or `en` |
| `authorizedUserIds` | IDs of users authorized to disarm |
| `soundEnabled` | Enables or disables every buzzer sound |
| `rfid` | Also requires the NFC card assigned to the same user |
| `fingerprint` | Reserved field for the fingerprint reader |
| `maxErrorCount` | Maximum error count, from 1 to 10 |
| `errorCountdownSeconds` | Penalty duration, from 0 to 3600 seconds |

### Language

```json
"language": "en"
```

Available values:

- `it` — Italian;
- `en` — English.

### Authorized users

Using semicolons:

```json
"authorizedUserIds": "1;2;3"
```

authentication from **any one** of the listed users is sufficient.

Using commas:

```json
"authorizedUserIds": "1,2,3"
```

**all** listed users must authenticate, in any order. A maximum of 4 users is supported in this mode.

Rules:

- only numbers, commas, and semicolons are accepted;
- commas and semicolons cannot be mixed;
- every ID must exist in `users.json`;
- ID `0` is invalid;
- an invalid configuration is replaced with the default value `"1"`.

### Error penalty

When `maxErrorCount` is reached, the display shows for 2 seconds:

```text
TOO MANY ERRORS
ALARM
```

The timer is internally set to:

```text
errorCountdownSeconds + 2
```

The additional 2 seconds compensate for the alarm screen duration. After the message ends, the visible time matches the configured value.

With:

```json
"errorCountdownSeconds": 0
```

forced timer reduction is disabled. Disarming still remains locked after the maximum error count.

### Fingerprint authentication

The `fingerprint` field is already part of the configuration, but the fingerprint reader is **not implemented yet** in the current firmware.

Do not set:

```json
"fingerprint": true
```

until the corresponding module is supported, because keypad disarming cannot be completed.

## Users

Users are stored in:

```text
/users.json
```

Example:

```json
[
  {
    "id": 1,
    "name": "Carlo",
    "uid": "04A1B2C3D4",
    "pin": "123456"
  }
]
```

Main rules:

- up to 100 users;
- unique internal ID;
- name up to 32 characters;
- PIN containing exactly 6 digits;
- hexadecimal NFC UID up to 32 characters;
- every UID must be unique;
- after deletion, the smallest available ID may be reused.

The PIN is always required. When `rfid=true`, PIN and UID must belong to the same user.

## Android application

The Android app can:

- scan for and connect to the device over BLE;
- perform administrator login;
- set, start, stop, and reset the timer;
- read live status;
- edit `config.json`;
- select the firmware language;
- configure authorized users;
- add, edit, and delete users.

Repository:

https://github.com/MaydayAlaska/Open-Airsoft-Countdown-Android-App

## BLE protocol

Main commands:

| Command | Function |
|---|---|
| `PING` | Tests communication |
| `LOGIN:<pin>` | Administrator login |
| `LOGOUT` | Ends the session |
| `STATUS` | Requests timer status |
| `SETTIME:<HHMMSS>` | Sets the timer |
| `START` | Starts the countdown |
| `STOP` | Stops the countdown |
| `RESET` | Resets the timer |
| `GETCONFIG` | Reads configuration |
| `SETCONFIG:...` | Saves configuration |
| `GETUSERS` | Reads users |
| `ADDUSER:...` | Adds a user |
| `UPDATEUSER:...` | Updates a user |
| `DELUSER:<id>` | Deletes a user |

`PING`, `STATUS`, `LOGIN`, and `LOGOUT` are handled directly. Every other administrative command requires a valid BLE login.

## Building with PlatformIO

### Software requirements

- Visual Studio Code;
- PlatformIO IDE extension;
- Arduino framework for ESP32;
- USB data cable.

### Libraries

Dependencies are defined in `platformio.ini`:

- U8g2;
- NimBLE-Arduino;
- Adafruit PN532;
- Keypad;
- ArduinoJson.

### Build and upload

Using PlatformIO:

1. open the repository folder;
2. connect the ESP32-S3;
3. select the `esp32-s3-devkitc-1` environment;
4. run **Build**;
5. run **Upload**;
6. open the serial monitor at `115200` baud.

Using a terminal:

```bash
pio run
pio run --target upload
pio device monitor
```

The current `platformio.ini` contains:

```ini
upload_port = COM9
```

Change or remove this line when the board uses a different port.

LittleFS is initialized automatically. When `config.json` or `users.json` is missing, the firmware creates it with default values.

## Main project structure

```text
data/       Initial LittleFS files
include/    C++ headers
src/        Firmware implementation
lib/        Local libraries
test/       Tests
platformio.ini
README.md
LICENSE
```

## Reporting bugs

Use the **Issues** section of this repository to report problems or suggest improvements.

Please include when possible:

- firmware version;
- hardware configuration;
- relevant `config.json` content;
- steps required to reproduce the problem;
- serial monitor logs;
- display photos or screenshots.

## Support the project

You can support Open Airsoft Countdown by:

- giving the GitHub repositories a Star;
- leaving a Like, saving the project, and using a Boost on MakerWorld;
- reporting bugs through GitHub Issues;
- contributing code or documentation;
- making a PayPal donation.

- MakerWorld: https://makerworld.com/it/@maydayalaska
- PayPal: https://paypal.me/lorisgennarini

## License

This project is released under the **GNU Affero General Public License v3.0**.

See the [LICENSE](LICENSE) file for details.
