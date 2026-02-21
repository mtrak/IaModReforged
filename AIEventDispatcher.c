// ============================================================
// AIEventDispatcher.c — Captura eventos del juego
// ReforgerAI Mod v1.0.0
// ============================================================

enum EAIEventType
{
	CONTACT_SPOTTED,
	UNIT_KILLED,
	OBJECTIVE_CAPTURED,
	PLAYER_DOWNED,
	VEHICLE_DESTROYED,
	MISSION_COMPLETED,
	MISSION_FAILED,
	REINFORCEMENT_ARRIVED
}

class AIEvent
{
	string eventId;
	string type;
	float timestamp;
	string sourceGroup;
	ref JsonWriteContext dataJson;

	void AIEvent(string t, string src)
	{
		static int counter = 0;
		eventId = "evt_" + (counter++).ToString().PadLeft(4, "0");
		type = t;
		timestamp = System.GetTickCount() / 1000.0;
		sourceGroup = src;
		dataJson = new JsonWriteContext();
	}
}

class AIEventDispatcher
{
	private AIBridge m_Bridge;
	private ref array<ref AIEvent> m_PendingEvents;

	void AIEventDispatcher(AIBridge bridge)
	{
		m_Bridge = bridge;
		m_PendingEvents = new array<ref AIEvent>();
		RegisterCallbacks();
	}

	// -------------------------------------------------------
	private void RegisterCallbacks()
	{
		// Suscribirse a eventos del juego
		GetGame().GetCallqueue().CallLater(PollGameEvents, 500, true);
	}

	// -------------------------------------------------------
	// Polling de eventos (Reforger no tiene todos los callbacks necesarios en scripting)
	private void PollGameEvents()
	{
		// Detectar contactos, bajas, objetivos — expandir según mapa/misión
	}

	// -------------------------------------------------------
	void PushEvent(AIEvent evt)
	{
		m_PendingEvents.Insert(evt);
		if (m_PendingEvents.Count() > 50)
			m_PendingEvents.RemoveIndex(0); // Evitar overflow
	}

	// -------------------------------------------------------
	void FlushEvents(JsonWriteContext json)
	{
		foreach (AIEvent evt : m_PendingEvents)
		{
			json.WriteObjectBegin();
			json.WriteString("event_id", evt.eventId);
			json.WriteString("type", evt.type);
			json.WriteFloat("timestamp", evt.timestamp);
			json.WriteString("source_group", evt.sourceGroup);
			json.WriteObjectEnd();
		}
		m_PendingEvents.Clear();
	}

	// -------------------------------------------------------
	// API pública para que otros componentes registren eventos
	void OnContactSpotted(string groupId, vector enemyPos, int enemyCount, float distance)
	{
		AIEvent evt = new AIEvent("CONTACT_SPOTTED", groupId);
		PushEvent(evt);
	}

	void OnUnitKilled(string groupId, string unitId)
	{
		AIEvent evt = new AIEvent("UNIT_KILLED", groupId);
		PushEvent(evt);
	}

	void OnPlayerDowned(int playerId)
	{
		AIEvent evt = new AIEvent("PLAYER_DOWNED", "");
		PushEvent(evt);
	}
}
