// ============================================================
// AICommandReceiver.c — Procesa y ejecuta comandos del LLM
// ReforgerAI Mod v1.0.0
// ============================================================

class AICommandReceiver
{
	private AIBridge m_Bridge;
	private ref AIGroupController m_GroupCtrl;
	private ref AIMissionManager m_MissionMgr;

	// -------------------------------------------------------
	void AICommandReceiver(AIBridge bridge)
	{
		m_Bridge = bridge;
		m_GroupCtrl = AIGroupController.GetInstance();
		m_MissionMgr = AIMissionManager.GetInstance();
	}

	// -------------------------------------------------------
	void ProcessCommandJson(string jsonStr)
	{
		JsonLoadContext json = new JsonLoadContext();
		if (!json.LoadFromString(jsonStr))
		{
			Print("[ReforgerAI] JSON inválido en respuesta IA");
			return;
		}

		string cmdId, reasoning;
		json.ReadString("command_id", cmdId);
		json.ReadString("reasoning", reasoning);

		if (m_Bridge.m_Config.m_bDebugMode)
			Print("[ReforgerAI] Razón IA: " + reasoning);

		// Iterar array de comandos
		JsonLoadContext cmdsArray;
		if (!json.ReadObject("commands", cmdsArray)) return;

		int cmdCount = cmdsArray.GetArraySize();
		for (int i = 0; i < cmdCount; i++)
		{
			JsonLoadContext cmd;
			cmdsArray.ReadArrayElement(i, cmd);
			ExecuteCommand(cmd);
		}
	}

	// -------------------------------------------------------
	private void ExecuteCommand(JsonLoadContext cmd)
	{
		string type, target;
		cmd.ReadString("type", type);
		cmd.ReadString("target", target);

		JsonLoadContext params;
		cmd.ReadObject("params", params);

		switch (type)
		{
			case "SET_FORMATION":
				ExecSetFormation(target, params);
				break;
			case "SET_WAYPOINT":
				ExecSetWaypoint(target, params);
				break;
			case "SET_BEHAVIOR":
				ExecSetBehavior(target, params);
				break;
			case "SPAWN_GROUP":
				ExecSpawnGroup(params);
				break;
			case "DESPAWN_GROUP":
				ExecDespawnGroup(target);
				break;
			case "UPDATE_MISSION":
				ExecUpdateMission(target, params);
				break;
			case "CREATE_MISSION":
				ExecCreateMission(params);
				break;
			case "END_MISSION":
				ExecEndMission(target);
				break;
			case "CALL_REINFORCEMENTS":
				ExecCallReinforcements(params);
				break;
			case "SET_AMBUSH":
				ExecSetAmbush(target, params);
				break;
			case "BROADCAST_MESSAGE":
				ExecBroadcastMessage(params);
				break;
			case "TRIGGER_EVENT":
				ExecTriggerEvent(params);
				break;
			case "VEHICLE_ORDER":
				ExecVehicleOrder(target, params);
				break;
			default:
				Print("[ReforgerAI] Comando desconocido: " + type);
		}
	}

	// -------------------------------------------------------
	private void ExecSetFormation(string groupId, JsonLoadContext params)
	{
		AIGroup group = m_GroupCtrl.GetGroup(groupId);
		if (!group) return;

		string formation;
		params.ReadString("formation", formation);
		m_GroupCtrl.SetFormation(group, formation);
		Print("[ReforgerAI] Formación " + formation + " → " + groupId);
	}

	// -------------------------------------------------------
	private void ExecSetWaypoint(string groupId, JsonLoadContext params)
	{
		AIGroup group = m_GroupCtrl.GetGroup(groupId);
		if (!group) return;

		vector pos = ReadPosition(params);
		string behavior;
		params.ReadString("behavior", behavior);
		m_GroupCtrl.SetWaypointWithBehavior(group, pos, behavior);
	}

	// -------------------------------------------------------
	private void ExecSetBehavior(string groupId, JsonLoadContext params)
	{
		AIGroup group = m_GroupCtrl.GetGroup(groupId);
		if (!group) return;

		string behavior;
		params.ReadString("behavior", behavior);
		m_GroupCtrl.SetGroupBehavior(group, behavior);
	}

	// -------------------------------------------------------
	private void ExecSpawnGroup(JsonLoadContext params)
	{
		string faction, template;
		params.ReadString("faction", faction);
		params.ReadString("template", template);
		vector pos = ReadPosition(params);

		string missionId;
		params.ReadString("assign_mission", missionId);

		AIGroup newGroup = m_GroupCtrl.SpawnGroup(faction, template, pos);
		if (newGroup && missionId != "")
			m_MissionMgr.AssignGroupToMission(newGroup, missionId);
	}

	// -------------------------------------------------------
	private void ExecDespawnGroup(string groupId)
	{
		m_GroupCtrl.DespawnGroup(groupId);
	}

	// -------------------------------------------------------
	private void ExecUpdateMission(string missionId, JsonLoadContext params)
	{
		m_MissionMgr.UpdateMission(missionId, params);
	}

	// -------------------------------------------------------
	private void ExecCreateMission(JsonLoadContext params)
	{
		m_MissionMgr.CreateMission(params);
	}

	// -------------------------------------------------------
	private void ExecEndMission(string missionId)
	{
		m_MissionMgr.EndMission(missionId);
	}

	// -------------------------------------------------------
	private void ExecCallReinforcements(JsonLoadContext params)
	{
		vector pos = ReadPosition(params);
		string faction;
		params.ReadString("faction", faction);
		int count;
		params.ReadInt("group_count", count);
		if (count <= 0) count = 1;

		for (int i = 0; i < count; i++)
		{
			vector spawnPos = pos + Vector(
				Math.RandomFloat(-100, 100), 0, Math.RandomFloat(-100, 100));
			m_GroupCtrl.SpawnGroup(faction, "infantry_squad", spawnPos);
		}
	}

	// -------------------------------------------------------
	private void ExecSetAmbush(string groupId, JsonLoadContext params)
	{
		AIGroup group = m_GroupCtrl.GetGroup(groupId);
		if (!group) return;

		vector pos = ReadPosition(params);
		m_GroupCtrl.SetAmbushPosition(group, pos);
	}

	// -------------------------------------------------------
	private void ExecBroadcastMessage(JsonLoadContext params)
	{
		string message;
		int duration;
		params.ReadString("message", message);
		params.ReadInt("duration", duration);
		if (duration <= 0) duration = 5;

		SCR_HintManagerComponent.ShowCustomHint(message, "[IA Táctica]", duration);
	}

	// -------------------------------------------------------
	private void ExecTriggerEvent(JsonLoadContext params)
	{
		string eventName;
		params.ReadString("event_name", eventName);
		// Disparar evento de script (expandir según necesidad)
		GetGame().GetCallqueue().Call(OnScriptEvent, eventName);
	}

	// -------------------------------------------------------
	private void ExecVehicleOrder(string vehicleId, JsonLoadContext params)
	{
		// Implementar órdenes a vehículos IA
		string order;
		params.ReadString("order", order);
		Print("[ReforgerAI] Orden vehículo " + order + " → " + vehicleId);
	}

	// -------------------------------------------------------
	// Helper: leer posición {x, y, z} de JsonLoadContext
	private vector ReadPosition(JsonLoadContext ctx)
	{
		JsonLoadContext posCtx;
		if (!ctx.ReadObject("position", posCtx))
			return Vector(0, 0, 0);

		float x, y, z;
		posCtx.ReadFloat("x", x);
		posCtx.ReadFloat("y", y);
		posCtx.ReadFloat("z", z);
		return Vector(x, y, z);
	}

	// -------------------------------------------------------
	private void OnScriptEvent(string eventName)
	{
		Print("[ReforgerAI] Evento disparado: " + eventName);
	}
}
