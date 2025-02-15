//
//

#include <weapon/weapon.h>
#include "weapon.h"
#include "object.h"
#include "weaponclass.h"
#include "subsystem.h"
#include "vecmath.h"
#include "team.h"
#include "mc_info.h"
#include "iff_defs/iff_defs.h"

namespace scripting {
namespace api {

//**********HANDLE: Weapon
ADE_OBJ_DERIV(l_Weapon, object_h, "weapon", "Weapon handle", l_Object);

ADE_VIRTVAR(Class, l_Weapon, "weaponclass", "Weapon's class", "weaponclass", "Weapon class, or invalid weaponclass handle if weapon handle is invalid")
{
	object_h *oh=NULL;
	int nc=-1;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&oh), l_Weaponclass.Get(&nc)))
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && nc > -1) {
		wp->weapon_info_index = nc;
	}

	return ade_set_args(L, "o", l_Weaponclass.Set(wp->weapon_info_index));
}

ADE_VIRTVAR(DestroyedByWeapon, l_Weapon, "boolean", "Whether weapon was destroyed by another weapon", "boolean", "True if weapon was destroyed by another weapon, false if weapon was destroyed by another object or if weapon handle is invalid")
{
	object_h *oh=NULL;
	bool b = false;

	int numargs = ade_get_args(L, "o|b", l_Weapon.GetPtr(&oh), &b);

	if(!numargs)
		return ade_set_error(L, "b", false);

	if(!oh->IsValid())
		return ade_set_error(L, "b", false);

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && numargs > 1) {
		wp->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon, b);
	}

	return ade_set_args(L, "b", wp->weapon_flags[Weapon::Weapon_Flags::Destroyed_by_weapon]);
}

ADE_VIRTVAR(LifeLeft, l_Weapon, "number", "Weapon life left (in seconds)", "number", "Life left (seconds) or 0 if weapon handle is invalid")
{
	object_h *oh=NULL;
	float nll = -1.0f;
	if(!ade_get_args(L, "o|f", l_Weapon.GetPtr(&oh), &nll))
		return ade_set_error(L, "f", 0.0f);

	if(!oh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && nll >= 0.0f) {
		wp->lifeleft = nll;
	}

	return ade_set_args(L, "f", wp->lifeleft);
}

ADE_VIRTVAR(FlakDetonationRange, l_Weapon, "number", "Range at which flak will detonate (meters)", "number", "Detonation range (meters) or 0 if weapon handle is invalid")
{
	object_h *oh=NULL;
	float rng = -1.0f;
	if(!ade_get_args(L, "o|f", l_Weapon.GetPtr(&oh), &rng))
		return ade_set_error(L, "f", 0.0f);

	if(!oh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && rng >= 0.0f) {
		wp->det_range = rng;
	}

	return ade_set_args(L, "f", wp->det_range);
}

ADE_VIRTVAR(Target, l_Weapon, "object", "Target of weapon. Value may also be a deriviative of the 'object' class, such as 'ship'.", "object", "Weapon target, or invalid object handle if weapon handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(wp->target_sig != newh->sig || !weapon_has_homing_object(wp))
			{
				weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, OBJ_INDEX(newh->objp), 1);
			}
		}
		else
		{
			weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, -1);
		}
	}

	return ade_set_object_with_breed(L, wp->target_num);
}

ADE_VIRTVAR(ParentTurret, l_Weapon, "subsystem", "Turret which fired this weapon.", "subsystem", "Turret subsystem handle, or an invalid handle if the weapon not fired from a turret")
{
	object_h *objh;
	ship_subsys_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->isSubsystemValid())
		{
			if(wp->turret_subsys != newh->ss)
			{
				wp->turret_subsys = newh->ss;
			}
		}
		else
		{
			wp->turret_subsys = NULL;
		}
	}

	if(wp->turret_subsys == NULL)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));
	else
		return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(&Objects[wp->turret_subsys->parent_objnum], wp->turret_subsys)));
}

ADE_VIRTVAR(HomingObject, l_Weapon, "object", "Object that weapon will home in on. Value may also be a deriviative of the 'object' class, such as 'ship'", "object", "Object that weapon is homing in on, or an invalid object handle if weapon is not homing or the weapon handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if (newh && newh->IsValid())
		{
			if (wp->target_sig != newh->sig)
			{
				weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, OBJ_INDEX(newh->objp), 1);
			}
		}
		else
		{
			weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, -1);
		}
	}

	if(!weapon_has_homing_object(wp))
		return ade_set_args(L, "o", l_Object.Set(object_h()));
	else
		return ade_set_object_with_breed(L, OBJ_INDEX(wp->homing_object));
}

ADE_VIRTVAR(HomingPosition, l_Weapon, "vector", "Position that weapon will home in on (World vector), setting this without a homing object in place will not have any effect!",
			"vector", "Homing point, or null vector if weapon handle is invalid")
{
	object_h *objh;
	vec3d *v3 = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR)
	{
		if(v3)
		{
			wp->homing_pos = *v3;
		}
		else
		{
			wp->homing_pos = vmd_zero_vector;
		}

		// need to update the position for multiplayer.
		if (Game_mode & GM_MULTIPLAYER) {
			wp->weapon_flags.set(Weapon::Weapon_Flags::Multi_homing_update_needed);
		}
	}

	return ade_set_args(L, "o", l_Vector.Set(wp->homing_pos));
}

ADE_VIRTVAR(HomingSubsystem, l_Weapon, "subsystem", "Subsystem that weapon will home in on.", "subsystem", "Homing subsystem, or invalid subsystem handle if weapon is not homing or weapon handle is invalid")
{
	object_h *objh;
	ship_subsys_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->isSubsystemValid())
		{
			if(wp->target_sig != newh->sig)
			{
				weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, OBJ_INDEX(newh->objp), 1, newh->ss);
			}
			else
			{
				wp->homing_subsys = newh->ss;
				get_subsystem_pos(&wp->homing_pos, wp->homing_object, wp->homing_subsys);
			}
		}
		else
		{
			weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, -1);
		}

		// need to update the position for multiplayer.
		if (Game_mode & GM_MULTIPLAYER) {
			wp->weapon_flags.set(Weapon::Weapon_Flags::Multi_homing_update_needed);
		}
	}

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(wp->homing_object, wp->homing_subsys)));
}

ADE_VIRTVAR(Team, l_Weapon, "team", "Weapon's team", "team", "Weapon team, or invalid team handle if weapon handle is invalid")
{
	object_h *oh=NULL;
	int nt=-1;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&oh), l_Team.Get(&nt)))
		return ade_set_error(L, "o", l_Team.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Team.Set(-1));

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && nt > -1 && nt < (int)Iff_info.size()) {
		wp->team = nt;
	}

	return ade_set_args(L, "o", l_Team.Set(wp->team));
}

ADE_VIRTVAR(OverrideHoming, l_Weapon, "boolean",
            "Whether homing is overridden for this weapon. When homing is overridden then the engine will not update "
            "the homing position of the weapon which means that it can be handled by scripting.",
            "boolean", "true if homing is overridden")
{
	object_h* oh = nullptr;
	bool new_val = false;
	if (!ade_get_args(L, "o|b", l_Weapon.GetPtr(&oh), &new_val))
		return ade_set_error(L, "b", false);

	if (!oh->IsValid())
		return ade_set_error(L, "b", false);

	weapon* wp = &Weapons[oh->objp->instance];

	if (ADE_SETTING_VAR) {
		wp->weapon_flags.set(Weapon::Weapon_Flags::Overridden_homing, new_val);
	}

	return ade_set_args(L, "b", wp->weapon_flags[Weapon::Weapon_Flags::Overridden_homing]);
}

ADE_FUNC(isArmed, l_Weapon, "[boolean HitTarget]", "Checks if the weapon is armed.", "boolean", "boolean value of the weapon arming status")
{
	object_h *oh = NULL;
	bool hit_target = false;
	if(!ade_get_args(L, "o|b", l_Weapon.GetPtr(&oh), &hit_target))
		return ADE_RETURN_FALSE;

	if(!oh->IsValid())
		return ADE_RETURN_FALSE;

	weapon *wp = &Weapons[oh->objp->instance];

	if(weapon_armed(wp, hit_target))
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
}

ADE_FUNC(getCollisionInformation, l_Weapon, nullptr, "Returns the collision information for this weapon",
         "collision_info", "The collision information or invalid handle if none")
{
	object_h *oh=NULL;
	if(!ade_get_args(L, "o", l_Weapon.GetPtr(&oh)))
		return ADE_RETURN_NIL;

	if(!oh->IsValid())
		return ADE_RETURN_NIL;

	weapon *wp = &Weapons[oh->objp->instance];

	if (wp->collisionInfo != nullptr)
		return ade_set_args(L, "o", l_ColInfo.Set(mc_info_h(*wp->collisionInfo)));
	else
		return ade_set_args(L, "o", l_ColInfo.Set(mc_info_h()));
}

ADE_FUNC(triggerSubmodelAnimation, l_Weapon, "string type, string triggeredBy, [boolean forwards = true, boolean resetOnStart = false, boolean completeInstant = false, boolean pause = false]",
	"Triggers an animation. Type is the string name of the animation type, "
	"triggeredBy is a closer specification which animation should trigger. See *-anim.tbm specifications. "
	"Forwards controls the direction of the animation. ResetOnStart will cause the animation to play from its initial state, as opposed to its current state. CompleteInstant will immediately complete the animation. Pause will instead stop the animation at the current state.",
	"boolean",
	"True if successful, false or nil otherwise")
{
	object_h* objh;
	const char* type = nullptr;
	const char* trigger = nullptr;
	bool forwards = true;
	bool forced = false;
	bool instant = false;
	bool pause = false;

	if (!ade_get_args(L, "oss|bbbb", l_Weapon.GetPtr(&objh), &type, &trigger, &forwards, &forced, &instant, &pause))
		return ADE_RETURN_NIL;

	if (!objh->IsValid())
		return ADE_RETURN_NIL;

	weapon* wp = &Weapons[objh->objp->instance];
	weapon_info* wip = &Weapon_info[wp->weapon_info_index];
	if(wip->render_type != WRT_POF || wp->model_instance_num < 0)
		return ADE_RETURN_FALSE;

	auto animtype = animation::anim_match_type(type);
	if (animtype == animation::ModelAnimationTriggerType::None)
		return ADE_RETURN_FALSE;

	return wip->animations.parseScripted(model_get_instance(wp->model_instance_num), animtype, trigger).start(forwards ? animation::ModelAnimationDirection::FWD : animation::ModelAnimationDirection::RWD, forced || instant, instant, pause) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE;
}

ADE_FUNC(getSubmodelAnimationTime, l_Weapon, "string type, string triggeredBy", "Gets time that animation will be done", "number", "Time (seconds), or 0 if weapon handle is invalid")
{
	object_h* objh;
	const char* type = nullptr;
	const char* trigger = nullptr;
	if (!ade_get_args(L, "oss", l_Weapon.GetPtr(&objh), &type, &trigger))
		return ade_set_error(L, "f", 0.0f);

	if (!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	auto animtype = animation::anim_match_type(type);
	if (animtype == animation::ModelAnimationTriggerType::None)
		return ade_set_error(L, "f", 0.0f);

	weapon* wp = &Weapons[objh->objp->instance];
	weapon_info* wip = &Weapon_info[wp->weapon_info_index];
	if (wip->render_type != WRT_POF || wp->model_instance_num < 0)
		return ade_set_error(L, "f", 0.0f);

	int time_ms = wip->animations.parseScripted(model_get_instance(wp->model_instance_num), animtype, trigger).getTime();
	float time_s = (float)time_ms / 1000.0f;

	return ade_set_args(L, "f", time_s);
}

ADE_FUNC(vanish, l_Weapon, nullptr, "Vanishes this weapon from the mission.", "boolean", "True if the deletion was successful, false otherwise.")
{

	object_h* oh = nullptr;
	if (!ade_get_args(L, "o", l_Weapon.GetPtr(&oh)))
		return ade_set_error(L, "b", false);

	if (!oh->IsValid())
		return ade_set_error(L, "b", false);

	oh->objp->flags.set(Object::Object_Flags::Should_be_dead);

	return ade_set_args(L, "b", true);
}

}
}

