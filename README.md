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
