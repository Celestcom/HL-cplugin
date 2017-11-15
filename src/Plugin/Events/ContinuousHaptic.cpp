#include "stdafx.h"
#include "ContinuousHaptic.h"
#pragma warning(push)
#pragma warning(disable : 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)
//ContinuousHaptic::ContinuousHaptic(float time) : PlayableEvent(time), m_strength(1.0)
//{
//}
//float ContinuousHaptic::duration() const
//{
//	return 0.0f;
//}
//
//std::vector<Validator> ContinuousHaptic::makeValidators() const
//{
//	return{
//		make_optional_constraint<float>(HLVR_EventKey_ContinuousHaptic_Strength_Float, [](float strength) { return strength >= 0 && strength <= 1.0; })
//	};
//}
//
//void ContinuousHaptic::doSerialize(NullSpaceIPC::HighLevelEvent & event) const
//{
//	NullSpaceIPC::LocationalEvent* location = event.mutable_locational_event();
//	NullSpaceIPC::ContinuousHaptic* cont = location->mutable_continuous_haptic();
//	cont->set_strength(m_strength);
//}
//
//void ContinuousHaptic::doParse(const ParameterizedEvent &ev)
//{
//	m_strength = ev.GetOr(HLVR_EventKey_ContinuousHaptic_Strength_Float, 1.0f);
//}
//
//bool ContinuousHaptic::isEqual(const PlayableEvent & other) const
//{
//	const auto& ev = static_cast<const ContinuousHaptic&>(other);
//	return m_strength == ev.m_strength;
//}
