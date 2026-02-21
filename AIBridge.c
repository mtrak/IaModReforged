// ============================================================
// AIBridge.c — Puente principal Enfusion ↔ Servicio IA
// ReforgerAI Mod v1.0.0
// ============================================================

class AIBridgeConfig : ScriptAndConfig
{
	[Attribute("http://localhost:8765", UIWidgets.EditBox, "URL del servicio IA")]
	string m_sServiceURL;

	[Attribute("2.0", UIWidgets.EditBox, "Intervalo de envío de estado (segundos)")]
	float m_fTickInterval;

	[Attribute("30.0", UIWidgets.EditBox, "Timeout de petición HTTP (segundos)")]
	float m_fRequestTimeout;

	[Attribute("1", UIWidgets.CheckBox, "Activar logs de depuración")]
	bool m_bDebugMode;
}

class AIBridge : ScriptComponent
{
	[Attribute("", UIWidgets.Object, "Configuración del puente IA")]
	ref AIBridgeConfig m_Config;

	private static AIBridge s_Instance;
	private ref AIEventDispatcher m_EventDispatcher;
	private ref AICommandReceiver m_CommandReceiver;
	private ref AIGameMasterHelper m_GMHelper;
	private float m_fTickTimer;
	private string m_sSessionId;
	private int m_iTick;
	private bool m_bActive;

	// -------------------------------------------------------
	static AIBridge GetInstance()
	{
		return s_Instance;
	}

	// -------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		s_Instance = this;
		m_sSessionId = "gm_" + Math.RandomInt(1000, 9999).ToString();
		m_iTick = 0;
		m_bActive = true;

		m_EventDispatcher = new AIEventDispatcher(this);
		m_CommandReceiver = new AICommandReceiver(this);
		m_GMHelper = new AIGameMasterHelper(this);

		GetGame().GetCallqueue().CallLater(FirstTick, 3000, false);
		Print("[ReforgerAI] Bridge iniciado. Sesión: " + m_sSessionId);
	}

	// -------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_bActive) return;

		m_fTickTimer += timeSlice;
		if (m_fTickTimer >= m_Config.m_fTickInterval)
		{
			m_fTickTimer = 0;
			SendGameState();
		}
	}

	// -------------------------------------------------------
	void FirstTick()
	{
		SendGameState();
	}

	// -------------------------------------------------------
	void SendGameState()
	{
		m_iTick++;
		string stateJson = BuildGameStateJson();

		if (m_Config.m_bDebugMode)
			Print("[ReforgerAI] Enviando estado tick " + m_iTick);

		// Petición HTTP al servicio IA
		RestContext ctx = GetGame().GetRestApi().GetContext(m_Config.m_sServiceURL);
		RestCallback cb = new RestCallback();
		cb.m_Callback = OnAIResponse;
		ctx.POST(cb, "/command", stateJson);
	}

	// -------------------------------------------------------
	private string BuildGameStateJson()
	{
		float ts = System.GetTickCount() / 1000.0;
		string mapName = "Unknown";
		// Obtener mapa actual
		ChimeraWorld world = GetGame().GetWorld();

		JsonWriteContext json = new JsonWriteContext();
		json.WriteObjectBegin();
		json.WriteFloat("timestamp", ts);
		json.WriteString("session_id", m_sSessionId);
		json.WriteString("map", mapName);
		json.WriteString("game_mode", "game_master");
		json.WriteInt("tick", m_iTick);

		// Serializar jugadores
		json.WriteKey("players");
		json.WriteArrayBegin();
		array<int> players = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(players);
		foreach (int pid : players)
		{
			IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(pid);
			if (!playerEnt) continue;
			SerializePlayer(json, pid, playerEnt);
		}
		json.WriteArrayEnd();

		// Serializar grupos IA
		json.WriteKey("ai_groups");
		json.WriteArrayBegin();
		SerializeAllAIGroups(json);
		json.WriteArrayEnd();

		// Serializar misiones activas
		json.WriteKey("active_missions");
		json.WriteArrayBegin();
		m_GMHelper.SerializeActiveMissions(json);
		json.WriteArrayEnd();

		// Eventos pendientes
		json.WriteKey("events");
		json.WriteArrayBegin();
		m_EventDispatcher.FlushEvents(json);
		json.WriteArrayEnd();

		// Estado del mundo
		json.WriteKey("world_state");
		SerializeWorldState(json);

		json.WriteObjectEnd();
		return json.GetResult();
	}

	// -------------------------------------------------------
	private void SerializePlayer(JsonWriteContext json, int pid, IEntity ent)
	{
		CharacterControllerComponent ctrl = CharacterControllerComponent.Cast(
			ent.FindComponent(CharacterControllerComponent));

		json.WriteObjectBegin();
		json.WriteString("id", "player_" + pid.ToString());
		json.WriteString("name", GetGame().GetPlayerManager().GetPlayerName(pid));
		json.WriteString("faction", GetEntityFaction(ent));
		SerializePosition(json, "position", ent.GetOrigin());
		json.WriteFloat("health", GetEntityHealth(ent));
		json.WriteBool("alive", ctrl && !ctrl.IsDead());
		json.WriteBool("in_vehicle", IsInVehicle(ent));
		json.WriteObjectEnd();
	}

	// -------------------------------------------------------
	private void SerializeAllAIGroups(JsonWriteContext json)
	{
		// Iterar sobre grupos IA registrados en el GroupController
		AIGroupController.GetInstance().SerializeGroups(json);
	}

	// -------------------------------------------------------
	private void SerializeWorldState(JsonWriteContext json)
	{
		json.WriteObjectBegin();
		TimeAndWeatherManagerEntity tw = TimeAndWeatherManagerEntity.Cast(
			GetGame().GetWorld().FindEntityByName("TimeAndWeather"));
		if (tw)
		{
			json.WriteFloat("time_of_day", tw.GetTimeOfDay());
		}
		json.WriteString("weather", "CLEAR"); // Expandir según API
		json.WriteObjectEnd();
	}

	// -------------------------------------------------------
	private void SerializePosition(JsonWriteContext json, string key, vector pos)
	{
		json.WriteKey(key);
		json.WriteObjectBegin();
		json.WriteFloat("x", pos[0]);
		json.WriteFloat("y", pos[1]);
		json.WriteFloat("z", pos[2]);
		json.WriteObjectEnd();
	}

	// -------------------------------------------------------
	void OnAIResponse(int code, string data)
	{
		if (code != 200)
		{
			if (m_Config.m_bDebugMode)
				Print("[ReforgerAI] Error HTTP " + code);
			return;
		}

		if (m_Config.m_bDebugMode)
			Print("[ReforgerAI] Respuesta recibida: " + data.Substring(0, Math.Min(200, data.Length())));

		m_CommandReceiver.ProcessCommandJson(data);
	}

	// -------------------------------------------------------
	// Helpers
	private string GetEntityFaction(IEntity ent)
	{
		FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(
			ent.FindComponent(FactionAffiliationComponent));
		if (!fac) return "UNKNOWN";
		Faction f = fac.GetAffiliatedFaction();
		if (!f) return "UNKNOWN";
		return f.GetFactionKey();
	}

	private float GetEntityHealth(IEntity ent)
	{
		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(
			ent.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!dmg) return 100.0;
		return dmg.GetHealthScaled() * 100.0;
	}

	private bool IsInVehicle(IEntity ent)
	{
		CompartmentAccessComponent comp = CompartmentAccessComponent.Cast(
			ent.FindComponent(CompartmentAccessComponent));
		return comp && comp.IsInCompartment();
	}

	// -------------------------------------------------------
	void SetActive(bool active)
	{
		m_bActive = active;
		Print("[ReforgerAI] Bridge " + (active ? "activado" : "desactivado"));
	}
}
