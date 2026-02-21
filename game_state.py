"""
game_state.py — Procesa y enriquece el GameState antes de enviarlo al LLM
"""

import json
import logging

log = logging.getLogger("ReforgerAI.State")


class GameStateProcessor:
    def __init__(self):
        self.previous_state = None
        self.tick_history = []

    def process(self, game_state: dict) -> str:
        """
        Recibe el GameState crudo, lo enriquece con contexto adicional
        y devuelve el JSON string listo para el LLM.
        """
        enriched = dict(game_state)

        # Calcular métricas derivadas
        enriched["_meta"] = self._compute_meta(game_state)

        # Guardar historial reducido (últimos 3 ticks para contexto)
        if self.previous_state:
            enriched["_previous_tick_summary"] = self._summarize(self.previous_state)

        self.previous_state = game_state

        return json.dumps(enriched, ensure_ascii=False)

    def _compute_meta(self, gs: dict) -> dict:
        """Calcula métricas útiles para el LLM."""
        players = gs.get("players", [])
        alive_players = [p for p in players if p.get("alive", True)]
        ai_groups = gs.get("ai_groups", [])
        events = gs.get("events", [])

        return {
            "player_count_alive": len(alive_players),
            "player_count_total": len(players),
            "ai_group_count": len(ai_groups),
            "recent_event_count": len(events),
            "threat_events": [e["type"] for e in events if e.get("type") in
                              {"CONTACT_SPOTTED", "PLAYER_DOWNED", "OBJECTIVE_CAPTURED"}],
            "pressure_level": self._compute_pressure(gs)
        }

    def _compute_pressure(self, gs: dict) -> str:
        """Estima la presión táctica actual sobre los jugadores."""
        players = gs.get("players", [])
        alive = sum(1 for p in players if p.get("alive", True))
        total = len(players)

        if total == 0:
            return "UNKNOWN"

        ratio = alive / total
        if ratio < 0.3:
            return "CRITICAL"  # Muchos jugadores caídos
        if ratio < 0.6:
            return "HIGH"
        if ratio < 0.9:
            return "MEDIUM"
        return "LOW"

    def _summarize(self, gs: dict) -> dict:
        """Resumen compacto del tick anterior."""
        return {
            "tick": gs.get("tick"),
            "player_alive": sum(1 for p in gs.get("players", []) if p.get("alive", True)),
            "ai_groups": len(gs.get("ai_groups", [])),
            "events": [e.get("type") for e in gs.get("events", [])]
        }
