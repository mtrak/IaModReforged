// ============================================================
// AIGroupController.c — Control de formaciones y tácticas
// ReforgerAI Mod v1.0.0
// ============================================================

class AIGroupController
{
	private static AIGroupController s_Instance;
	private ref map<string, AIGroup> m_Groups;
	private int m_iGroupCounter;

	static AIGroupController GetInstance()
	{
		if (!s_Instance) s_Instance = new AIGroupController();
		return s_Instance;
	}

	void AIGroupController()
	{
		m_Groups = new map<string, AIGroup>();
		m_iGroupCounter = 0;
	}

	// -------------------------------------------------------
	// Registrar grupo existente
	void RegisterGroup(AIGroup group, string id = "")
	{
		if (id == "")
			id = "grp_" + (m_iGroupCounter++).ToString().PadLeft(3, "0");
		m_Groups.Set(id, group);
	}

	AIGroup GetGroup(string id)
	{
		return m_Groups.Get(id);
	}

	// -------------------------------------------------------
	// Formaciones disponibles
	void SetFormation(AIGroup group, string formationKey)
	{
		if (!group) return;

		// Mapeo de claves a enum de Reforger
		EUnitFormation formation = EUnitFormation.COLUMN;
		switch (formationKey)
		{
			case "LINE":          formation = EUnitFormation.LINE; break;
			case "COLUMN":        formation = EUnitFormation.COLUMN; break;
			case "WEDGE":         formation = EUnitFormation.WEDGE; break;
			case "SKIRMISHER":    formation = EUnitFormation.STAGGERED_COLUMN; break;
			case "VEE":           formation = EUnitFormation.VEE; break;
			case "ECHELON_LEFT":  formation = EUnitFormation.ECHELON_LEFT; break;
			case "ECHELON_RIGHT": formation = EUnitFormation.ECHELON_RIGHT; break;
		}

		SCR_AIGroup scrGroup = SCR_AIGroup.Cast(group);
		if (scrGroup) scrGroup.SetFormation(formation);
	}

	// -------------------------------------------------------
	void SetWaypointWithBehavior(AIGroup group, vector position, string behavior)
	{
		if (!group) return;

		AIWaypoint wp = AIWaypoint.Cast(
			GetGame().SpawnEntityPrefab(
				Resource.Load("{E2957DCB8B2F14F9}Prefabs/AI/Waypoints/AIWaypoint.et"),
				null, position));

		if (!wp) return;

		// Configurar comportamiento del waypoint
		switch (behavior)
		{
			case "PATROL":
				wp.SetCompletionType(AIWaypointCompletionType.MOVE);
				break;
			case "ASSAULT":
				wp.SetCompletionType(AIWaypointCompletionType.ATTACK);
				break;
			case "DEFEND":
				wp.SetCompletionType(AIWaypointCompletionType.DEFEND);
				break;
			case "RETREAT":
				wp.SetCompletionType(AIWaypointCompletionType.MOVE);
				SetGroupBehavior(group, "SAFE");
				break;
			case "FLANK":
				wp.SetCompletionType(AIWaypointCompletionType.MOVE);
				SetGroupBehavior(group, "COMBAT");
				break;
		}

		group.AddWaypoint(wp);
	}

	// -------------------------------------------------------
	void SetGroupBehavior(AIGroup group, string behavior)
	{
		if (!group) return;

		SCR_AIGroup scrGroup = SCR_AIGroup.Cast(group);
		if (!scrGroup) return;

		switch (behavior)
		{
			case "SAFE":
				scrGroup.SetBehavior(AIGroupBehavior.SAFE);
				break;
			case "AWARE":
				scrGroup.SetBehavior(AIGroupBehavior.AWARE);
				break;
			case "COMBAT":
				scrGroup.SetBehavior(AIGroupBehavior.COMBAT);
				break;
			case "STEALTH":
				scrGroup.SetBehavior(AIGroupBehavior.STEALTH);
				break;
		}
	}

	// -------------------------------------------------------
	void SetAmbushPosition(AIGroup group, vector position)
	{
		if (!group) return;
		SetWaypointWithBehavior(group, position, "DEFEND");
		SetGroupBehavior(group, "STEALTH");
	}

	// -------------------------------------------------------
	AIGroup SpawnGroup(string faction, string template, vector position)
	{
		// Templates básicos (expandir con tus prefabs)
		string prefabPath = GetTemplatePrefab(faction, template);
		if (prefabPath == "") return null;

		IEntity groupEnt = GetGame().SpawnEntityPrefab(
			Resource.Load(prefabPath), null, position);

		if (!groupEnt) return null;

		AIGroup group = AIGroup.Cast(groupEnt);
		if (!group) return null;

		string newId = "grp_" + faction.ToLower() + "_" + (m_iGroupCounter++).ToString();
		RegisterGroup(group, newId);
		Print("[ReforgerAI] Grupo spawneado: " + newId + " en " + position.ToString());
		return group;
	}

	// -------------------------------------------------------
	void DespawnGroup(string groupId)
	{
		AIGroup group = m_Groups.Get(groupId);
		if (!group) return;

		// Eliminar unidades del grupo
		array<AIAgent> agents = new array<AIAgent>();
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (agent)
				SCR_EntityHelper.DeleteEntityAndChildren(agent.GetControlledEntity());
		}
		m_Groups.Remove(groupId);
		Print("[ReforgerAI] Grupo eliminado: " + groupId);
	}

	// -------------------------------------------------------
	void SerializeGroups(JsonWriteContext json)
	{
		foreach (string id, AIGroup group : m_Groups)
		{
			if (!group) continue;
			SerializeGroup(json, id, group);
		}
	}

	// -------------------------------------------------------
	private void SerializeGroup(JsonWriteContext json, string id, AIGroup group)
	{
		array<AIAgent> agents = new array<AIAgent>();
		group.GetAgents(agents);

		json.WriteObjectBegin();
		json.WriteString("group_id", id);
		json.WriteString("faction", GetGroupFaction(group));
		json.WriteInt("unit_count", agents.Count());

		// Posición del líder
		AIAgent leader = group.GetLeader();
		if (leader && leader.GetControlledEntity())
		{
			vector pos = leader.GetControlledEntity().GetOrigin();
			json.WriteKey("position");
			json.WriteObjectBegin();
			json.WriteFloat("x", pos[0]);
			json.WriteFloat("y", pos[1]);
			json.WriteFloat("z", pos[2]);
			json.WriteObjectEnd();
		}

		json.WriteFloat("health_avg", GetGroupAverageHealth(agents));
		json.WriteObjectEnd();
	}

	// -------------------------------------------------------
	private string GetTemplatePrefab(string faction, string template)
	{
		// Mapa de plantillas — ajusta con los prefabs de tu servidor
		if (faction == "OPFOR" && template == "infantry_squad")
			return "{B5DF06B6DCA0D870}Prefabs/Groups/OPFOR/Group_OPFOR_Rifle_Squad.et";
		if (faction == "BLUFOR" && template == "infantry_squad")
			return "{A8476E3F6B2B7541}Prefabs/Groups/FIA/Group_FIA_Rifle_Squad.et";
		return "";
	}

	private string GetGroupFaction(AIGroup group)
	{
		AIAgent leader = group.GetLeader();
		if (!leader) return "UNKNOWN";
		IEntity ent = leader.GetControlledEntity();
		if (!ent) return "UNKNOWN";
		FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(
			ent.FindComponent(FactionAffiliationComponent));
		if (!fac || !fac.GetAffiliatedFaction()) return "UNKNOWN";
		return fac.GetAffiliatedFaction().GetFactionKey();
	}

	private float GetGroupAverageHealth(array<AIAgent> agents)
	{
		if (agents.IsEmpty()) return 0;
		float total = 0;
		foreach (AIAgent agent : agents)
		{
			if (!agent || !agent.GetControlledEntity()) continue;
			SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(
				agent.GetControlledEntity().FindComponent(SCR_CharacterDamageManagerComponent));
			total += dmg ? dmg.GetHealthScaled() * 100.0 : 100.0;
		}
		return total / agents.Count();
	}
}
