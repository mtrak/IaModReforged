# INSTALL_PROMPTS.md — Prompts de instalación y uso para IA
# ReforgerAI Mod v1.0.0
# Copia cada bloque y pégalo directamente en tu IA para ejecutar la acción indicada.

---

## 1. INSTALACIÓN DEL SERVICIO IA — WINDOWS

```
Instala el servicio ReforgerAI en Windows. Ejecuta estos pasos en orden:

1. Abre PowerShell como administrador.

2. Verifica que Python 3.11+ está instalado:
   python --version
   Si no está instalado, descárgalo de https://python.org e instálalo marcando "Add to PATH".

3. Verifica que Ollama está instalado:
   ollama --version
   Si no está instalado, descárgalo de https://ollama.ai e instálalo.

4. Descarga el modelo de LLM recomendado (mistral 7B, buen equilibrio velocidad/calidad):
   ollama pull mistral:7b-instruct

5. Ve a la carpeta del servicio del mod:
   cd "C:\Users\TU_USUARIO\Documents\ReforgerAI\service"
   (ajusta la ruta a donde hayas colocado el mod)

6. Crea un entorno virtual e instala dependencias:
   python -m venv venv
   .\venv\Scripts\activate
   pip install -r requirements.txt

7. Verifica que Ollama está corriendo (debería arrancarse con Windows automáticamente):
   curl http://localhost:11434/api/tags

8. Arranca el servicio ReforgerAI:
   python main.py

   Deberías ver:
   ✓ Ollama conectado. Modelo: mistral:7b-instruct
   🚀 ReforgerAI Service escuchando en http://0.0.0.0:8765

9. Deja esta ventana de PowerShell abierta mientras juegas.
```

---

## 2. INSTALACIÓN DEL SERVICIO IA — LINUX (Ubuntu/Debian)

```
Instala el servicio ReforgerAI en Linux. Ejecuta estos pasos en una terminal:

1. Actualiza paquetes e instala Python:
   sudo apt update && sudo apt install -y python3.11 python3.11-venv python3-pip curl

2. Instala Ollama:
   curl -fsSL https://ollama.ai/install.sh | sh

3. Arranca el servicio Ollama en segundo plano:
   ollama serve &

4. Descarga el modelo LLM:
   ollama pull mistral:7b-instruct

5. Ve a la carpeta del servicio:
   cd ~/ReforgerAI/service

6. Crea entorno virtual e instala dependencias:
   python3.11 -m venv venv
   source venv/bin/activate
   pip install -r requirements.txt

7. Verifica la conexión con Ollama:
   curl http://localhost:11434/api/tags

8. Arranca el servicio:
   python main.py

   Deberías ver:
   ✓ Ollama conectado. Modelo: mistral:7b-instruct
   🚀 ReforgerAI Service escuchando en http://0.0.0.0:8765
```

---

## 3. ARRANQUE CON UN SOLO COMANDO

### Windows (PowerShell) — start_reforger_ai.ps1
```
Crea el archivo start_reforger_ai.ps1 en el escritorio de Windows con este contenido,
luego ejecútalo con clic derecho > "Ejecutar con PowerShell":

---CONTENIDO DEL ARCHIVO---
# start_reforger_ai.ps1
$ErrorActionPreference = "Stop"
$ServiceDir = "$PSScriptRoot\service"

Write-Host "=== ReforgerAI Launcher ===" -ForegroundColor Cyan

# 1. Arrancar Ollama si no está corriendo
$ollamaRunning = Get-Process ollama -ErrorAction SilentlyContinue
if (-not $ollamaRunning) {
    Write-Host "Arrancando Ollama..." -ForegroundColor Yellow
    Start-Process ollama -ArgumentList "serve" -WindowStyle Hidden
    Start-Sleep -Seconds 3
}

# 2. Verificar modelo
Write-Host "Verificando modelo LLM..." -ForegroundColor Yellow
ollama pull mistral:7b-instruct 2>$null

# 3. Activar venv y arrancar servicio
Set-Location $ServiceDir
& ".\venv\Scripts\activate.ps1"
Write-Host "Arrancando servicio ReforgerAI en puerto 8765..." -ForegroundColor Green
python main.py
---FIN DEL ARCHIVO---
```

### Linux — start_reforger_ai.sh
```
Crea el archivo start_reforger_ai.sh en el directorio raíz del mod con este contenido
y dale permisos de ejecución con: chmod +x start_reforger_ai.sh

Luego arráncalo con: ./start_reforger_ai.sh

---CONTENIDO DEL ARCHIVO---
#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SERVICE_DIR="$SCRIPT_DIR/service"

echo "=== ReforgerAI Launcher ==="

# 1. Arrancar Ollama si no está corriendo
if ! pgrep -x "ollama" > /dev/null; then
    echo "[*] Arrancando Ollama..."
    ollama serve &
    sleep 3
fi

# 2. Verificar modelo
echo "[*] Verificando modelo LLM..."
ollama pull mistral:7b-instruct 2>/dev/null || true

# 3. Activar venv y arrancar servicio
cd "$SERVICE_DIR"
source venv/bin/activate
echo "[*] Arrancando ReforgerAI en http://0.0.0.0:8765 ..."
python main.py
---FIN DEL ARCHIVO---
```

---

## 4. INSTALACIÓN DEL MOD EN ARMA REFORGER (OpenClaw)

```
Instala el mod ReforgerAI en Arma Reforger usando OpenClaw. Sigue estos pasos:

1. Abre OpenClaw en tu sistema.

2. En OpenClaw, ve a la sección "Mods locales" o "Workspace".

3. Haz clic en "Importar mod" o "Añadir mod desde carpeta".

4. Selecciona la carpeta raíz del mod: la que contiene el archivo mod.json
   (ejemplo: C:\ReforgerAI\ en Windows, o ~/ReforgerAI/ en Linux)

5. OpenClaw leerá el mod.json automáticamente y registrará el mod con:
   - ID: reforger-ai-gamemaster
   - Nombre: ReforgerAI — Control IA Tiempo Real

6. En OpenClaw, compila el mod:
   - Haz clic derecho sobre el mod → "Compilar" o "Build"
   - Espera a que compile todos los scripts .c de Scripts/Game/ReforgerAI/

7. Para un servidor dedicado, en OpenClaw ve a "Configuración del servidor" y añade
   el mod a la lista de mods activos copiando su ID: reforger-ai-gamemaster

8. Guarda la configuración y arranca el servidor desde OpenClaw.

NOTA: El mod requiere que el servicio Python (main.py) esté corriendo ANTES de
arrancar la partida. Arranca primero el servicio (paso 3), luego el servidor.
```

---

## 5. CONFIGURACIÓN DEL MOD EN PARTIDA (Game Master)

```
Configura el mod ReforgerAI dentro de Arma Reforger en modo Game Master:

1. Inicia una partida en modo Game Master (single o multiplayer).

2. En el editor de Game Master, busca la entidad "AIBridge" en el panel de entidades.
   Estará bajo la categoría "ReforgerAI".

3. Coloca la entidad AIBridge en cualquier punto del mapa (no importa dónde,
   solo necesita existir en la escena).

4. Selecciona la entidad AIBridge y en sus propiedades configura:
   - Service URL: http://localhost:8765
     (si el servicio corre en otra máquina, usa su IP: http://192.168.1.X:8765)
   - Tick Interval: 2.0  (segundos entre envíos de estado al LLM)
   - Debug Mode: activado para ver logs en la consola de Reforger

5. Guarda la escena en Game Master.

6. El mod comenzará a operar automáticamente: cada 2 segundos enviará el estado
   del juego al servicio IA y ejecutará los comandos que devuelva el LLM.

7. Puedes verificar que funciona abriendo en el navegador:
   http://localhost:8765/health
   Deberías ver: {"status": "ok", "llm_reachable": true, ...}
```

---

## 6. CAMBIAR EL MODELO LLM

```
Cambia el modelo LLM que usa ReforgerAI sin tocar código. Ejecuta esto en terminal:

# Opción A — Variable de entorno (recomendado, sin reiniciar el servicio)
# Windows PowerShell:
$env:RAI_OLLAMA_MODEL = "llama3:8b"
python main.py

# Linux bash:
RAI_OLLAMA_MODEL="llama3:8b" python main.py

# Opción B — Editar config.py directamente:
# Abre service/config.py y cambia la línea:
#   OLLAMA_MODEL = os.getenv("RAI_OLLAMA_MODEL", "mistral:7b-instruct")
# Por el modelo que quieras usar, por ejemplo:
#   OLLAMA_MODEL = os.getenv("RAI_OLLAMA_MODEL", "llama3:8b")

# Modelos recomendados por hardware:
# GPU 4-6 GB VRAM:  mistral:7b-instruct  (rápido, buena táctica)
# GPU 8+ GB VRAM:   llama3:8b            (mejor razonamiento)
# GPU 16+ GB VRAM:  llama3:70b-instruct  (excelente táctica, más lento)
# Solo CPU:         phi3:mini            (muy ligero, táctica básica)

# Descargar el nuevo modelo antes de cambiar:
ollama pull llama3:8b
```

---

## 7. CONFIGURACIÓN PARA SERVIDOR DEDICADO MULTIPLAYER

```
Configura ReforgerAI para un servidor dedicado de Arma Reforger en Linux.
Ejecuta estos pasos en el servidor:

1. Instala el servicio ReforgerAI en el servidor siguiendo el prompt de instalación Linux.

2. Configura el servicio para escuchar solo en localhost (más seguro) o en la IP
   del servidor si el LLM corre en otra máquina:
   
   # Solo localhost (LLM y servidor en la misma máquina — recomendado):
   RAI_HOST=127.0.0.1 RAI_PORT=8765 python main.py
   
   # IP específica (LLM en máquina separada):
   RAI_HOST=0.0.0.0 RAI_PORT=8765 RAI_OLLAMA_URL=http://192.168.1.100:11434 python main.py

3. Crea un servicio systemd para que arranque automáticamente con el servidor.
   Crea el archivo /etc/systemd/system/reforger-ai.service con este contenido:

[Unit]
Description=ReforgerAI LLM Service
After=network.target ollama.service

[Service]
Type=simple
User=TU_USUARIO
WorkingDirectory=/home/TU_USUARIO/ReforgerAI/service
Environment=RAI_HOST=127.0.0.1
Environment=RAI_PORT=8765
Environment=RAI_OLLAMA_MODEL=mistral:7b-instruct
ExecStart=/home/TU_USUARIO/ReforgerAI/service/venv/bin/python main.py
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target

4. Activa e inicia el servicio:
   sudo systemctl daemon-reload
   sudo systemctl enable reforger-ai
   sudo systemctl start reforger-ai

5. Verifica que está corriendo:
   sudo systemctl status reforger-ai
   curl http://localhost:8765/health

6. En el mod (AIBridge), configura Service URL como:
   http://127.0.0.1:8765

7. Los jugadores NO necesitan el servicio en sus máquinas. Solo corre en el servidor.
```

---

## 8. VERIFICACIÓN Y DIAGNÓSTICO

```
Diagnostica problemas con la instalación de ReforgerAI. Ejecuta estos comandos
en tu terminal para verificar cada componente:

# 1. Verificar que Python está instalado correctamente:
python --version
# Esperado: Python 3.11.x o superior

# 2. Verificar que Ollama está corriendo:
curl http://localhost:11434/api/tags
# Esperado: JSON con lista de modelos instalados

# 3. Verificar que el modelo está descargado:
ollama list
# Deberías ver: mistral:7b-instruct (o el modelo que hayas elegido)

# 4. Verificar que el servicio ReforgerAI está corriendo:
curl http://localhost:8765/health
# Esperado: {"status": "ok", "llm_reachable": true, ...}

# 5. Ver estadísticas de sesión (requests procesados, latencia):
curl http://localhost:8765/stats

# 6. Test manual del endpoint con GameState de ejemplo:
curl -X POST http://localhost:8765/command \
  -H "Content-Type: application/json" \
  -d '{"timestamp":1234567890,"session_id":"test","tick":1,"players":[],"ai_groups":[],"active_missions":[],"events":[],"world_state":{"time_of_day":12,"weather":"CLEAR","visibility":1000}}'
# Esperado: JSON con "commands" array

# 7. Ver logs del servicio en tiempo real (Linux):
journalctl -fu reforger-ai

# PROBLEMAS COMUNES:
# - "Connection refused" en port 8765: el servicio no está arrancado, ejecuta main.py
# - "llm_reachable: false": Ollama no está corriendo, ejecuta: ollama serve
# - "Modelo no encontrado": descarga el modelo con: ollama pull mistral:7b-instruct
# - Latencia alta (>10s): usa un modelo más ligero (phi3:mini) o activa GPU en Ollama
# - El mod no conecta en multijugador: verifica que el firewall permite el puerto 8765
```

---

## 9. AJUSTE FINO DEL COMPORTAMIENTO TÁCTICO

```
Ajusta cómo el LLM toma decisiones tácticas en ReforgerAI editando el archivo
service/llm_client.py en la sección SYSTEM_PROMPT.

Para hacer la IA más agresiva, añade al SYSTEM_PROMPT:
"Prioriza siempre el ataque directo. Los grupos deben avanzar hacia los jugadores
constantemente y llamar refuerzos cuando sufran más de un 30% de bajas."

Para hacer la IA más defensiva:
"Prioriza posiciones defensivas en altura y cobertura. Solo ataca cuando los
jugadores estén a menos de 200 metros o en campo abierto."

Para partidas PvE cooperativas difíciles:
"Coordina emboscadas entre múltiples grupos. Un grupo fija a los jugadores mientras
otro flanquea. Usa el terreno y la noche a tu favor. Llama refuerzos agresivamente."

Para modo narrativo/roleplay:
"Crea tensión narrativa gradual. Empieza con patrullas lentas, escala a emboscadas
y finalmente a asalto masivo cuando los jugadores hayan avanzado suficiente.
Usa BROADCAST_MESSAGE para dar contexto narrativo al encuentro."

Después de editar, reinicia el servicio:
# Windows: Ctrl+C en la ventana de PowerShell y vuelve a ejecutar python main.py
# Linux: sudo systemctl restart reforger-ai
```

---

## 10. ESTRUCTURA DE ARCHIVOS — REFERENCIA RÁPIDA

```
Explica qué hace cada archivo de ReforgerAI:

SCRIPTS DEL MOD (Enfusion/Arma Reforger):
- Scripts/Game/ReforgerAI/AIBridge.c
  Componente principal del mod. Se coloca en la escena de Game Master.
  Envía el estado del juego al servicio Python cada N segundos (configurable).
  Recibe los comandos JSON del LLM y los pasa al CommandReceiver.

- Scripts/Game/ReforgerAI/AICommandReceiver.c
  Parsea el JSON de comandos del LLM y ejecuta cada acción en el motor del juego:
  mover grupos, cambiar formaciones, spawnear unidades, actualizar misiones, etc.

- Scripts/Game/ReforgerAI/AIGroupController.c
  Registra y gestiona todos los grupos de IA en la partida.
  Aplica formaciones, waypoints y comportamientos a los grupos.

- Scripts/Game/ReforgerAI/AIEventDispatcher.c
  Captura eventos del juego (contactos, bajas, objetivos) y los acumula
  para incluirlos en el próximo GameState enviado al LLM.

- Scripts/Game/ReforgerAI/AIMissionManager.c
  Gestiona misiones dinámicas: crea, actualiza y finaliza misiones en tiempo real.

- Scripts/Game/ReforgerAI/AIGameMasterHelper.c
  Funciones auxiliares específicas del modo Game Master.

SERVICIO PYTHON (corre fuera del juego):
- service/main.py
  Punto de entrada. Servidor HTTP aiohttp que recibe GameState y devuelve AICommand.
  Arrancar con: python main.py

- service/llm_client.py
  Cliente para Ollama. Envía el estado del juego al LLM y recibe los comandos.
  Contiene el SYSTEM_PROMPT que define el comportamiento táctico de la IA.

- service/game_state.py
  Enriquece el GameState con métricas calculadas (presión táctica, resumen anterior).

- service/schema.py
  Valida que GameState y AICommand tengan el formato correcto.

- service/config.py
  Configuración del servicio (puerto, URL de Ollama, modelo, temperatura).
  Todos los valores son sobreescribibles con variables de entorno RAI_*.

- service/requirements.txt
  Dependencias Python: solo aiohttp.

CONFIGURACIÓN:
- mod.json
  Metadatos del mod para OpenClaw (ID, nombre, versión, lista de scripts).
```
