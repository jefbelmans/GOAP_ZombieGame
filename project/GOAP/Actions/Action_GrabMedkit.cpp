#include "Action_GrabMedkit.h"
#include "../WorldState.h"

#include "Exam_HelperStructs.h"
#include "IExamInterface.h"

GOAP::Action_GrabMedkit::Action_GrabMedkit()
	: BaseAction("Grab Medkit", 5)
	, m_MedkitGrabbed(false)
	, m_InventorySlot(2U)
{
	SetPrecondition("medkit_aquired", true);
	SetEffect("medkit_in_inventory", true);
	SetEffect("medkit_grabbed", true);
}

bool GOAP::Action_GrabMedkit::IsDone()
{
	return m_MedkitGrabbed;
}

void GOAP::Action_GrabMedkit::Reset()
{
	BaseAction::Reset();
	m_MedkitGrabbed = false;
}

int GOAP::Action_GrabMedkit::GetCost() const
{
	return static_cast<int>(m_pTarget->Location.Distance(m_AgentInfo.Position));
}

#pragma warning( push )
#pragma warning( disable : 6011)
bool GOAP::Action_GrabMedkit::IsValid(Elite::Blackboard* pBlackboard)
{
	if (!pBlackboard->GetData("Interface", m_pInterface) || m_pInterface == nullptr) return false;
	if (!pBlackboard->GetData("WorldState", m_pWorldState) || m_pWorldState == nullptr) return false;
	if (!pBlackboard->GetData("Medkits", m_pEntities) || m_pEntities == nullptr || m_pEntities->empty()) return false;
	
	m_AgentInfo = m_pInterface->Agent_GetInfo();
	m_pTarget = GetClosestEntity();
	return m_pTarget != nullptr || !m_pWorldState->GetVariable("all_houses_looted");
}
#pragma warning( pop )

bool GOAP::Action_GrabMedkit::Execute(Elite::Blackboard* pBlackboard)
{
	ItemInfo newMedkit;
	if (m_pInterface->Item_GetInfo(*m_pTarget, newMedkit))
	{
		if (newMedkit.Type != eItemType::MEDKIT) return false;
		// We do not have a medkit yet, equip this one
		if (!m_pWorldState->GetVariable("medkit_in_inventory"))
		{
			m_pInterface->Item_Grab(*m_pTarget, newMedkit);
			m_pInterface->Inventory_AddItem(m_InventorySlot, newMedkit);
		}
		else
		{
			ItemInfo equipedMedkit;
			m_pInterface->Inventory_GetItem(m_InventorySlot, equipedMedkit);
			// We have a medkit equiped, check if the new one has more health
			if (m_pInterface->Medkit_GetHealth(newMedkit) >= m_pInterface->Medkit_GetHealth(equipedMedkit))
			{
				// The new medkit has more health, use the current equiped and equip the new one
				m_pInterface->Inventory_UseItem(m_InventorySlot);
				m_pInterface->Inventory_RemoveItem(m_InventorySlot);
				m_pInterface->Item_Grab(*m_pTarget, newMedkit);
				m_pInterface->Inventory_AddItem(m_InventorySlot, newMedkit);
			}
			else
			{
				// The new medkit has less health, destroy it.
				std::cout << "[Grab Medkit]: Better medkit in inventory, destroying ground item!" << std::endl;
				m_pInterface->Item_Destroy(*m_pTarget);
			}
		}
		

		// Remove item from general entity list
		std::vector<EntityInfo>* pEntities;
		pBlackboard->GetData("Entities", pEntities);
		pEntities->erase(std::find(pEntities->begin(), pEntities->end(), *m_pTarget));

		// Remove from medkit list
		m_pEntities->erase(std::find(m_pEntities->begin(), m_pEntities->end(), *m_pTarget));
		m_pWorldState->SetVariable("medkit_in_inventory", true);
		m_pWorldState->SetVariable("item_looted", true);
		return m_MedkitGrabbed = true;
	}

	// Couldn't grab item, abort action
	return false;
}
