// ============================================================
// AIMissionManager.c — Gestión dinámica de misiones
// ReforgerAI Mod v1.0.0
// ============================================================

class MissionData
{
	string missionId;
	string type;
	string status;
	vector objectivePosition;
	ref array<string> assignedGroups;
	float timeRemaining;
	float completion;

	void MissionData()
	{
		assignedGroups = new array<string>();
		timeRemaining = -1;
		completion = 0;
		status = "ACTIVE";
	}
}

class AIMissionManager
{
	private static AIMissionManager s_Instance;
	private ref map<string, ref MissionData> m_Missions;
	private int m_iMissionCounter;

	static AIMissionManager GetInstance()
	{
		if (!s_Instance) s_Instance = new AIMissionManager();
		return s_Instance;
	}

	void AIMissionManager()
	{
		m_Missions = new map<string, ref MissionData>();
		m_iMissionCounter = 0;
	}

	// -------------------------------------------------------
	void CreateMission(JsonLoadContext params)
	{
		string type;
		params.ReadString("type", type);

		MissionData md = new MissionData();
		md.missionId = "mission_" + (m_iMissionCounter++).ToString().PadLeft(3, "0");
		md.type = type;

		JsonLoadContext posCtx;
		if (params.ReadObject("objective_position", posCtx))
		{
			float x, y, z;
			posCtx.ReadFloat("x", x);
			posCtx.ReadFloat("y", y);
			posCtx.ReadFloat("z", z);
			md.objectivePosition = Vector(x, y, z);
		}

		params.ReadFloat("time_limit", md.timeRemaining);
		m_Missions.Set(md.missionId, md);
		Print("[ReforgerAI] Misión creada: " + md.missionId + " tipo " + type);
	}

	// -------------------------------------------------------
	void UpdateMission(string missionId, JsonLoadContext params)
	{
		MissionData md = m_Missions.Get(missionId);
		if (!md) return;

		string priority;
		params.ReadString("priority", priority);

		JsonLoadContext posCtx;
		if (params.ReadObject("new_objective", posCtx))
		{
			float x, y, z;
			posCtx.ReadFloat("x", x);
			posCtx.ReadFloat("y", y);
			posCtx.ReadFloat("z", z);
			md.objectivePosition = Vector(x, y, z);
		}

		Print("[ReforgerAI] Misión actualizada: " + missionId);
	}

	// -------------------------------------------------------
	void EndMission(string missionId)
	{
		MissionData md = m_Missions.Get(missionId);
		if (!md) return;
		md.status = "COMPLETED";
		Print("[ReforgerAI] Misión finalizada: " + missionId);
	}

	// -------------------------------------------------------
	void AssignGroupToMission(AIGroup group, string missionId)
	{
		// Registrar la asignación
		Print("[ReforgerAI] Grupo asignado a misión: " + missionId);
	}

	// -------------------------------------------------------
	void SerializeActiveMissions(JsonWriteContext json)
	{
		foreach (string id, MissionData md : m_Missions)
		{
			if (!md || md.status != "ACTIVE") continue;

			json.WriteObjectBegin();
			json.WriteString("mission_id", md.missionId);
			json.WriteString("type", md.type);
			json.WriteString("status", md.status);
			json.WriteKey("objective_position");
			json.WriteObjectBegin();
			json.WriteFloat("x", md.objectivePosition[0]);
			json.WriteFloat("y", md.objectivePosition[1]);
			json.WriteFloat("z", md.objectivePosition[2]);
			json.WriteObjectEnd();
			json.WriteFloat("completion", md.completion);
			json.WriteObjectEnd();
		}
	}
}
