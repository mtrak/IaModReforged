"""
schema.py — Validación de GameState y AICommand
"""

import json
import logging

log = logging.getLogger("ReforgerAI.Schema")

VALID_COMMAND_TYPES = {
    "SET_FORMATION", "SET_WAYPOINT", "SET_BEHAVIOR",
    "SPAWN_GROUP", "DESPAWN_GROUP", "UPDATE_MISSION",
    "CREATE_MISSION", "END_MISSION", "CALL_REINFORCEMENTS",
    "SET_AMBUSH", "BROADCAST_MESSAGE", "TRIGGER_EVENT", "VEHICLE_ORDER"
}

VALID_FORMATIONS = {
    "LINE", "COLUMN", "WEDGE", "SKIRMISHER",
    "VEE", "ECHELON_LEFT", "ECHELON_RIGHT"
}

VALID_BEHAVIORS = {"SAFE", "AWARE", "COMBAT", "STEALTH"}
VALID_WP_BEHAVIORS = {"PATROL", "ASSAULT", "DEFEND", "RETREAT", "FLANK"}


def validate_game_state(gs: dict) -> bool:
    """Valida que el GameState tenga los campos mínimos requeridos."""
    required = ["timestamp", "session_id", "tick"]
    for field in required:
        if field not in gs:
            log.warning(f"GameState: falta campo '{field}'")
            return False
    return True


def validate_ai_command(cmd: dict) -> bool:
    """Valida que el AICommand tenga estructura correcta."""
    if "commands" not in cmd:
        log.warning("AICommand: falta array 'commands'")
        return False

    if not isinstance(cmd["commands"], list):
        log.warning("AICommand: 'commands' no es array")
        return False

    for i, c in enumerate(cmd["commands"]):
        if "type" not in c:
            log.warning(f"Comando {i}: falta 'type'")
            return False
        if c["type"] not in VALID_COMMAND_TYPES:
            log.warning(f"Comando {i}: tipo inválido '{c['type']}'")
            return False

    return True
