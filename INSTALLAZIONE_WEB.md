# Installazione del Web Installer

Questi file vanno copiati nella cartella principale della repository `Open-Airsoft-Countdown`.

## File aggiunti

- `.github/workflows/web-installer.yml`
- `docs/.nojekyll`
- `docs/icon.svg`
- `docs/index.html`
- `docs/manifest.json`
- `tools/merge_firmware.py`

## File modificato

- `platformio.ini`

La riga seguente genera automaticamente il file completo per ESP Web Tools dopo ogni compilazione:

```ini
extra_scripts = post:tools/merge_firmware.py
```

## Pubblicazione

Dopo aver copiato i file:

```bash
git add .
git commit -m "Add ESP Web Tools installer"
git push
```

Poi apri la repository su GitHub:

1. `Settings`;
2. `Pages`;
3. in `Build and deployment`, scegli `GitHub Actions`;
4. apri la scheda `Actions` e verifica il workflow `Build and deploy web installer`.

Quando il workflow termina, l’installer sarà disponibile su:

```text
https://maydayalaska.github.io/Open-Airsoft-Countdown/
```
