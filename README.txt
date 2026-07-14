Open Airsoft Countdown firmware v1.8

Modifiche v1.8:
- OLED e PN532 ora utilizzano due bus I2C separati.
- OLED invariato: SDA GPIO8, SCL GPIO9.
- PN532 spostato su: SDA GPIO1, SCL GPIO2.
- PN532 IRQ invariato su GPIO10.
- PN532 RESET invariato su GPIO11.
- Il LED di stato è stato spostato da GPIO2 a GPIO14 per evitare il conflitto con SCL del PN532.
- Il PN532 usa il controller I2C secondario dell'ESP32-S3 tramite TwoWire(1).

Modifiche precedenti mantenute:
- supporto italiano/inglese;
- controllo BLE e applicazione Android;
- utenti autorizzati singoli o multipli;
- autenticazione PIN e NFC;
- penalità configurabile dopo il numero massimo di errori.
