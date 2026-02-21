#!/usr/bin/env python3
"""
ReforgerAI Service — Servicio IA para Arma Reforger
Conecta el mod de Enfusion con un LLM local (Ollama)
Arranque: python main.py
"""

import asyncio
import json
import logging
import time
import signal
import sys
from aiohttp import web
from llm_client import OllamaClient
from game_state import GameStateProcessor
from command_executor import CommandValidator
from schema import validate_game_state, validate_ai_command
import config as cfg

# ─── Logging ────────────────────────────────────────────────
logging.basicConfig(
    level=logging.DEBUG if cfg.DEBUG_MODE else logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S"
)
log = logging.getLogger("ReforgerAI")

# ─── Servicio principal ──────────────────────────────────────
class ReforgerAIService:
    def __init__(self):
        self.llm = OllamaClient(
            base_url=cfg.OLLAMA_URL,
            model=cfg.OLLAMA_MODEL,
            timeout=cfg.LLM_TIMEOUT
        )
        self.state_processor = GameStateProcessor()
        self.validator = CommandValidator()
        self.session_stats = {
            "requests": 0,
            "errors": 0,
            "avg_latency_ms": 0,
            "started_at": time.time()
        }

    # ─── Handler principal: recibe estado, devuelve comandos ─
    async def handle_command(self, request: web.Request) -> web.Response:
        start = time.perf_counter()
        self.session_stats["requests"] += 1

        try:
            raw = await request.text()
            game_state = json.loads(raw)

            # Validar schema de entrada
            if not validate_game_state(game_state):
                log.warning("GameState inválido recibido")
                return web.Response(status=400, text='{"error":"invalid_game_state"}')

            # Procesar y enriquecer estado
            context = self.state_processor.process(game_state)

            # Llamar al LLM
            log.debug(f"Tick {game_state.get('tick', '?')} — enviando a LLM")
            ai_response = await self.llm.generate(context)

            # Validar respuesta del LLM
            command = json.loads(ai_response)
            if not validate_ai_command(command):
                log.error("LLM devolvió comando inválido, usando fallback")
                command = self.get_fallback_command(game_state)

            elapsed = (time.perf_counter() - start) * 1000
            self._update_latency(elapsed)
            log.info(f"Respuesta en {elapsed:.0f}ms — {len(command.get('commands', []))} comandos")

            return web.Response(
                content_type="application/json",
                text=json.dumps(command)
            )

        except json.JSONDecodeError as e:
            log.error(f"JSON decode error: {e}")
            self.session_stats["errors"] += 1
            return web.Response(status=400, text='{"error":"json_parse_error"}')
        except Exception as e:
            log.error(f"Error procesando petición: {e}", exc_info=True)
            self.session_stats["errors"] += 1
            return web.Response(status=500, text='{"error":"internal_error"}')

    # ─── Health check ────────────────────────────────────────
    async def handle_health(self, request: web.Request) -> web.Response:
        llm_ok = await self.llm.ping()
        uptime = int(time.time() - self.session_stats["started_at"])
        return web.Response(
            content_type="application/json",
            text=json.dumps({
                "status": "ok" if llm_ok else "degraded",
                "llm": cfg.OLLAMA_MODEL,
                "llm_reachable": llm_ok,
                "uptime_s": uptime,
                "stats": self.session_stats
            })
        )

    # ─── Stats endpoint ──────────────────────────────────────
    async def handle_stats(self, request: web.Request) -> web.Response:
        return web.Response(
            content_type="application/json",
            text=json.dumps(self.session_stats)
        )

    # ─── Fallback cuando el LLM falla ───────────────────────
    def get_fallback_command(self, game_state: dict) -> dict:
        return {
            "command_id": f"fallback_{int(time.time())}",
            "timestamp": time.time(),
            "reasoning": "Fallback: LLM no disponible, manteniendo estado actual",
            "commands": []
        }

    def _update_latency(self, ms: float):
        n = self.session_stats["requests"]
        prev = self.session_stats["avg_latency_ms"]
        self.session_stats["avg_latency_ms"] = round((prev * (n - 1) + ms) / n, 1)


# ─── Arranque ────────────────────────────────────────────────
async def main():
    service = ReforgerAIService()

    app = web.Application()
    app.router.add_post("/command", service.handle_command)
    app.router.add_get("/health",   service.handle_health)
    app.router.add_get("/stats",    service.handle_stats)

    # Comprobación inicial del LLM
    log.info(f"Verificando conexión con Ollama en {cfg.OLLAMA_URL} ...")
    if not await service.llm.ping():
        log.warning("⚠️  Ollama no responde — el servicio arrancará en modo degradado")
    else:
        log.info(f"✓ Ollama conectado. Modelo: {cfg.OLLAMA_MODEL}")

    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, cfg.BIND_HOST, cfg.BIND_PORT)
    await site.start()

    log.info(f"🚀 ReforgerAI Service escuchando en http://{cfg.BIND_HOST}:{cfg.BIND_PORT}")
    log.info("   POST /command  — recibe GameState, devuelve AICommand")
    log.info("   GET  /health   — estado del servicio")
    log.info("   GET  /stats    — estadísticas de sesión")
    log.info("Pulsa Ctrl+C para detener")

    # Mantener vivo
    stop_event = asyncio.Event()

    def _sig(sig, frame):
        log.info("Señal recibida, deteniendo...")
        stop_event.set()

    signal.signal(signal.SIGINT, _sig)
    signal.signal(signal.SIGTERM, _sig)

    await stop_event.wait()
    await runner.cleanup()
    log.info("Servicio detenido.")


if __name__ == "__main__":
    asyncio.run(main())
