# Open Airsoft Countdown

[🇬🇧 go to English](#english)

---

<a id="italiano"></a>

# 🇮🇹

**Open Airsoft Countdown** è un progetto open source pensato per creare un timer scenico per softair, laser tag, escape room e giochi di ruolo.

Lo scopo è realizzare un dispositivo da usare come oggetto di gioco: un countdown interattivo con display, tastierino, NFC, buzzer e LED di stato.

Il progetto sarà abbinato anche ad alcuni file per **stampa 3D**, utili per costruire il contenitore e le parti esterne del dispositivo.

Questo progetto è destinato solo all’intrattenimento. Non è pensato per controllare dispositivi reali, pericolosi, pirotecnici o esplosivi.

## Funzionalità previste

- Timer countdown per scenari di gioco
- Inserimento del PIN amministratore all’avvio
- Impostazione del tempo tramite tastierino
- Display per mostrare stato e tempo rimanente
- Lettura NFC per identificare i giocatori
- Autenticazione tramite card NFC + PIN
- Feedback acustico tramite buzzer
- LED di stato durante il countdown
- Configurazione salvata sul dispositivo
- Supporto futuro per controllo da smartphone tramite BLE
- File di stampa 3D per contenitore e parti sceniche

## Configurazione

Il dispositivo utilizza un file `config.json` per salvare alcune impostazioni principali.

Esempio:

```json
{
  "adminPin": "000000",
  "bleName": "Open Airsoft Countdown",
  "soundEnabled": true,
  "rfid": false,
  "fingerprint": false,
  "maxErrorCount": 3,
  "errorCountdownSeconds": 10
}
```

Significato dei campi:

| Campo | Descrizione |
|---|---|
| `adminPin` | PIN amministratore richiesto all’avvio |
| `bleName` | Nome del dispositivo Bluetooth/BLE |
| `soundEnabled` | Abilita o disabilita il buzzer |
| `rfid` | Abilita o disabilita la lettura del tag RFID/NFC |
| `fingerprint` | Abilita o disabilita la lettura dell’impronta digitale |
| `maxErrorCount` | Numero massimo di errori consentiti prima del blocco del sistema |
| `errorCountdownSeconds` | Valore a cui viene portato il countdown dopo il numero massimo di errori. 0 disattiva la funzione |

## Stato del progetto

Il progetto è in sviluppo.

La struttura hardware, il pinout, il case e alcune funzioni potranno cambiare prima della prima versione stabile.

## Licenza

Questo progetto è distribuito con licenza **GNU Affero General Public License v3.0**.

Consulta il file `LICENSE` per i dettagli.

---

<a id="english"></a>

# 🇬🇧

**Open Airsoft Countdown** is an open-source project designed to create a game prop countdown timer for airsoft, laser tag, escape rooms and role-play scenarios.

The goal is to build an interactive game device with a display, keypad, NFC reader, buzzer and status LED.

The project will also be paired with some **3D printing files**, useful for building the enclosure and the external prop parts.

This project is intended only for entertainment. It is not designed to control real, dangerous, pyrotechnic or explosive devices.

[🇮🇹 vai a Italiano](#italiano)

## Planned features

- Countdown timer for game scenarios
- Admin PIN at startup
- Timer setup using a keypad
- Display for status and remaining time
- NFC reader for player identification
- Authentication using NFC card + PIN
- Buzzer feedback
- Status LED during countdown
- Configuration stored on the device
- Future smartphone control via BLE
- 3D printing files for the enclosure and prop parts

## Configuration

The device uses a `config.json` file to store some main settings.

Example:

```json
{
  "adminPin": "000000",
  "bleName": "Open Airsoft Countdown",
  "soundEnabled": true,
  "rfid": false,
  "fingerprint": false,
  "maxErrorCount": 3,
  "errorCountdownSeconds": 10
}
```

Field meaning:

| Field | Description |
|---|---|
| `adminPin` | Admin PIN required at startup |
| `bleName` | Bluetooth/BLE device name |
| `soundEnabled` | Enables or disables the buzzer |
| `rfid` | Enables or disables RFID/NFC tag reading |
| `fingerprint` | Enables or disables fingerprint reading |
| `maxErrorCount` | Maximum number of allowed errors before the system locks |
| `errorCountdownSeconds` | Countdown value forced after the maximum number of errors is reached. 0 disables the function|

## Project status

The project is under development.

The hardware structure, pinout, enclosure and some features may change before the first stable release.

## License

This project is licensed under the **GNU Affero General Public License v3.0**.

See the `LICENSE` file for details.
