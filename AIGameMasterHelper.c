// ============================================================
// AIGameMasterHelper.c — Helpers para modo Game Master
// ReforgerAI Mod v1.0.0
// ============================================================

class AIGameMasterHelper
{
	private AIBridge m_Bridge;

	void AIGameMasterHelper(AIBridge bridge)
	{
		m_Bridge = bridge;
	}

	// -------------------------------------------------------
	// Serializar misiones activas (delega a AIMissionManager)
	void SerializeActiveMissions(JsonWriteContext json)
	{
		AIMissionManager.GetInstance().SerializeActiveMissions(json);
	}

	// -------------------------------------------------------
	// Listar zonas controladas por cada facción en Game Master
	void GetControlledZones(out array<string> bluforZones, out array<string> opforZones)
	{
		bluforZones = new array<string>();
		opforZones = new array<string>();

		// Iterar sobre SCR_CaptureArea o similares según el módulo de misión
		// Expandir según el tipo de misión que uses en Game Master
		GetGame().GetWorld().QueryEntitiesByComponent(
			SCR_CaptureArea,
			QueryCaptureAreas,
			null,
			true
		);
	}

	// -------------------------------------------------------
	// Callback interno de query de zonas
	private bool QueryCaptureAreas(IEntity entity)
	{
		// Procesar zona capturada — expandir según implementación
		return true;
	}

	// -------------------------------------------------------
	// Crear trigger de refuerzos en posición
	void CreateReinforcementTrigger(vector pos, string faction, float radius)
	{
		// Colocar trigger de reinforcement en zona
		Print("[ReforgerAI] Trigger de refuerzos en " + pos.ToString() + " para " + faction);
	}
}
