# config.py — Configuración del servicio ReforgerAI
# Edita estos valores o usa variables de entorno

import os

# ── Servidor ─────────────────────────────────────────────────
BIND_HOST = os.getenv("RAI_HOST", "0.0.0.0")
BIND_PORT = int(os.getenv("RAI_PORT", "8765"))

# ── Ollama ───────────────────────────────────────────────────
OLLAMA_URL   = os.getenv("RAI_OLLAMA_URL",   "http://localhost:11434")
OLLAMA_MODEL = os.getenv("RAI_OLLAMA_MODEL", "mistral:7b-instruct")
LLM_TIMEOUT  = int(os.getenv("RAI_LLM_TIMEOUT", "60"))

# ── LLM Parámetros ───────────────────────────────────────────
LLM_TEMPERATURE   = float(os.getenv("RAI_TEMPERATURE",   "0.4"))
LLM_CONTEXT_SIZE  = int(os.getenv("RAI_CONTEXT_SIZE",    "4096"))

# ── Debug ────────────────────────────────────────────────────
DEBUG_MODE = os.getenv("RAI_DEBUG", "false").lower() == "true"
