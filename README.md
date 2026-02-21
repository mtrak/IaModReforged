# ReforgerAI — Mod de Control IA para Arma Reforger

Versión: 1.0.0 | Compatible con: Game Master (single/multi) | LLM Backend: Ollama

---

## Estructura de archivos del mod

```
ReforgerAI/
├── mod.json                          # Metadatos del mod (OpenClaw)
├── Scripts/
│   └── Game/
│       └── ReforgerAI/
│           ├── AIBridge.c            # Puente principal Enfusion ↔ servicio IA
│           ├── AIEventDispatcher.c   # Captura eventos del juego y los envía al bridge
│           ├── AICommandReceiver.c   # Recibe y ejecuta comandos de la IA
│           ├── AIGroupController.c   # Controla formaciones y tácticas de grupos
│           ├── AIMissionManager.c    # Gestión dinámica de misiones
│           └── AIGameMasterHelper.c  # Helpers específicos para Game Master
├── service/
│   ├── main.py                       # Punto de entrada del servicio IA
│   ├── llm_client.py                 # Cliente Ollama
│   ├── game_state.py                 # Estado del juego en tiempo real
│   ├── command_executor.py           # Traducción de órdenes LLM → acciones juego
│   ├── schema.py                     # Validación del schema JSON
│   └── requirements.txt
├── config/
│   ├── ai_config.json                # Configuración principal
│   └── prompts/
│       ├── system_prompt.txt         # Prompt de sistema para el LLM
│       └── tactical_context.txt      # Contexto táctico base
└── docs/
    └── INSTALL_PROMPTS.md            # Este archivo de prompts de instalación
```

---

## Schema JSON de comunicación

### Juego → Servicio IA (GameState)

```json
{
  "timestamp": 1710000000.0,
  "session_id": "gm_session_001",
  "map": "Eden",
  "game_mode": "game_master",
  "tick": 4821,
  "players": [
    {
      "id": "player_001",
      "name": "Soldado1",
      "faction": "BLUFOR",
      "position": { "x": 1234.5, "y": 0.0, "z": 5678.9 },
      "health": 85,
      "alive": true,
      "role": "rifleman",
      "in_vehicle": false
    }
  ],
  "ai_groups": [
    {
      "group_id": "grp_opfor_001",
      "faction": "OPFOR",
      "leader_id": "ai_unit_003",
      "units": ["ai_unit_003", "ai_unit_004", "ai_unit_005"],
      "formation": "COLUMN",
      "state": "PATROL",
      "position": { "x": 2000.0, "y": 0.0, "z": 6000.0 },
      "waypoint": { "x": 2100.0, "y": 0.0, "z": 6100.0 },
      "threat_level": 0.0,
      "ammo_status": "FULL",
      "health_avg": 100
    }
  ],
  "active_missions": [
    {
      "mission_id": "mission_001",
      "type": "DEFEND",
      "status": "ACTIVE",
      "objective_position": { "x": 1500.0, "y": 0.0, "z": 5800.0 },
      "assigned_groups": ["grp_opfor_001"],
      "time_remaining": 600,
      "completion": 0.0
    }
  ],
  "events": [
    {
      "event_id": "evt_001",
      "type": "CONTACT_SPOTTED",
      "timestamp": 1710000005.0,
      "source_group": "grp_opfor_001",
      "data": {
        "enemy_position": { "x": 1300.0, "y": 0.0, "z": 5700.0 },
        "enemy_count": 3,
        "distance": 250.0
      }
    }
  ],
  "world_state": {
    "time_of_day": 14.5,
    "weather": "CLEAR",
    "visibility": 1000,
    "blufor_controlled_zones": ["zone_A"],
    "opfor_controlled_zones": ["zone_B", "zone_C"]
  }
}
```

### Servicio IA → Juego (AICommand)

```json
{
  "command_id": "cmd_001",
  "timestamp": 1710000006.0,
  "reasoning": "Contacto detectado a 250m. Grupo en columna expuesto. Transición a online de skirmisher para cubrir flancos.",
  "commands": [
    {
      "type": "SET_FORMATION",
      "target": "grp_opfor_001",
      "params": {
        "formation": "SKIRMISHER"
      }
    },
    {
      "type": "SET_WAYPOINT",
      "target": "grp_opfor_001",
      "params": {
        "position": { "x": 1900.0, "y": 0.0, "z": 6050.0 },
        "behavior": "ASSAULT"
      }
    },
    {
      "type": "SPAWN_GROUP",
      "target": "world",
      "params": {
        "faction": "OPFOR",
        "template": "infantry_squad",
        "position": { "x": 2200.0, "y": 0.0, "z": 6300.0 },
        "assign_mission": "mission_001"
      }
    },
    {
      "type": "UPDATE_MISSION",
      "target": "mission_001",
      "params": {
        "new_objective": { "x": 1400.0, "y": 0.0, "z": 5750.0 },
        "priority": "HIGH"
      }
    },
    {
      "type": "BROADCAST_MESSAGE",
      "target": "all_players",
      "params": {
        "message": "Refuerzos enemigos detectados en cuadrícula 214-578",
        "duration": 8
      }
    }
  ]
}
```

### Tipos de comando disponibles

| Tipo | Descripción |
|---|---|
| `SET_FORMATION` | Cambia formación del grupo (LINE, COLUMN, WEDGE, SKIRMISHER, VEE, ECHELON_LEFT/RIGHT) |
| `SET_WAYPOINT` | Mueve grupo a posición con comportamiento (PATROL, ASSAULT, DEFEND, RETREAT, FLANK) |
| `SET_BEHAVIOR` | Cambia actitud (SAFE, AWARE, COMBAT, STEALTH) |
| `SPAWN_GROUP` | Genera nuevo grupo de IA |
| `DESPAWN_GROUP` | Elimina grupo |
| `UPDATE_MISSION` | Modifica misión activa |
| `CREATE_MISSION` | Crea nueva misión dinámica |
| `END_MISSION` | Finaliza misión |
| `CALL_REINFORCEMENTS` | Activa refuerzos en zona |
| `SET_AMBUSH` | Coloca grupo en emboscada |
| `BROADCAST_MESSAGE` | Mensaje a jugadores |
| `TRIGGER_EVENT` | Dispara evento de scripted |
| `SET_WEATHER` | Cambia clima (si Game Master lo permite) |
| `VEHICLE_ORDER` | Órdenes a vehículos controlados por IA |

---

## Prompt de sistema para el LLM

```
Eres el director táctico de IA para una partida de Arma Reforger en modo Game Master.
Tu rol es analizar el estado del campo de batalla en tiempo real y emitir órdenes 
tácticas coherentes, dinámicas y desafiantes para los grupos de IA OPFOR/INDFOR.

PRINCIPIOS TÁCTICOS:
- Mantén presión constante pero justa sobre los jugadores
- Adapta dificultad según el rendimiento del equipo jugador
- Usa flanqueos, emboscadas y retiradas tácticas
- Coordina múltiples grupos cuando sea necesario
- Crea narrativa dinámica y misiones emergentes

RESTRICCIONES:
- Solo emite comandos del schema JSON definido
- No hagas suposiciones sobre posiciones no reportadas
- Responde SIEMPRE con JSON válido, sin texto extra
- Prioriza la diversión del jugador sobre el realismo puro

Recibirás GameState JSON y deberás responder con AICommand JSON.
```
