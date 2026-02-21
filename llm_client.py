"""
llm_client.py — Cliente para Ollama
Gestiona la comunicación con el LLM local
"""

import json
import logging
import time
import aiohttp
import config as cfg

log = logging.getLogger("ReforgerAI.LLM")

SYSTEM_PROMPT = """Eres el director táctico de IA para una partida de Arma Reforger en modo Game Master.
Tu rol es analizar el estado del campo de batalla en tiempo real y emitir órdenes tácticas coherentes,
dinámicas y desafiantes para los grupos de IA OPFOR/INDFOR.

PRINCIPIOS TÁCTICOS:
- Mantén presión constante pero justa sobre los jugadores
- Adapta dificultad según el rendimiento del equipo jugador (bajas, zonas controladas)
- Usa flanqueos, emboscadas y retiradas tácticas cuando sea apropiado
- Coordina múltiples grupos cuando el escenario lo requiera
- Crea tensión narrativa y misiones emergentes basadas en el contexto
- Reacciona a eventos recientes (contactos, bajas, objetivos capturados)

RESTRICCIONES ABSOLUTAS:
- Responde SOLO con JSON válido, sin texto extra, sin markdown, sin explicaciones fuera del JSON
- Usa únicamente los tipos de comando del schema definido
- No inventes posiciones que no estén en el GameState
- El campo "reasoning" puede contener tu análisis táctico en español

TIPOS DE COMANDO VÁLIDOS:
SET_FORMATION, SET_WAYPOINT, SET_BEHAVIOR, SPAWN_GROUP, DESPAWN_GROUP,
UPDATE_MISSION, CREATE_MISSION, END_MISSION, CALL_REINFORCEMENTS,
SET_AMBUSH, BROADCAST_MESSAGE, TRIGGER_EVENT, VEHICLE_ORDER

FORMACIONES: LINE, COLUMN, WEDGE, SKIRMISHER, VEE, ECHELON_LEFT, ECHELON_RIGHT
COMPORTAMIENTOS: SAFE, AWARE, COMBAT, STEALTH
WAPOINTS/BEHAVIOR: PATROL, ASSAULT, DEFEND, RETREAT, FLANK"""


class OllamaClient:
    def __init__(self, base_url: str, model: str, timeout: int = 60):
        self.base_url = base_url.rstrip("/")
        self.model = model
        self.timeout = aiohttp.ClientTimeout(total=timeout)
        self._conversation_history = []

    async def ping(self) -> bool:
        try:
            async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=5)) as session:
                async with session.get(f"{self.base_url}/api/tags") as resp:
                    return resp.status == 200
        except Exception:
            return False

    async def generate(self, game_state_json: str) -> str:
        """Envía el estado del juego al LLM y devuelve el JSON de comandos."""

        messages = [
            {"role": "system", "content": SYSTEM_PROMPT},
            {
                "role": "user",
                "content": (
                    "Analiza este estado del campo de batalla y emite órdenes tácticas.\n"
                    "Responde SOLO con el JSON de AICommand:\n\n"
                    f"```json\n{game_state_json}\n```"
                )
            }
        ]

        payload = {
            "model": self.model,
            "messages": messages,
            "stream": False,
            "format": "json",
            "options": {
                "temperature": cfg.LLM_TEMPERATURE,
                "top_p": 0.9,
                "num_ctx": cfg.LLM_CONTEXT_SIZE
            }
        }

        t0 = time.perf_counter()
        async with aiohttp.ClientSession(timeout=self.timeout) as session:
            async with session.post(
                f"{self.base_url}/api/chat",
                json=payload
            ) as resp:
                if resp.status != 200:
                    text = await resp.text()
                    raise RuntimeError(f"Ollama error {resp.status}: {text}")

                data = await resp.json()
                elapsed = (time.perf_counter() - t0) * 1000
                log.debug(f"LLM respondió en {elapsed:.0f}ms")

                content = data["message"]["content"]

                # Limpiar posibles bloques markdown si el modelo los añade
                content = content.strip()
                if content.startswith("```"):
                    lines = content.split("\n")
                    content = "\n".join(lines[1:-1])

                # Verificar que es JSON válido
                json.loads(content)  # lanza si no es válido
                return content

    def clear_history(self):
        self._conversation_history = []
