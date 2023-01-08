#pragma once
#include "BaseAction.h"

namespace GOAP
{
	class Action_ClearPistol final : public BaseAction
	{
	public:
		Action_ClearPistol();

		virtual bool IsDone() override;
		virtual void Reset() override;
		virtual int GetCost() const override;

		virtual bool IsValid(Elite::Blackboard* pBlackboard) override;
		virtual bool Execute(Elite::Blackboard* pBlackboard) override;

	private:
		bool m_PistolCleared;
	};
}