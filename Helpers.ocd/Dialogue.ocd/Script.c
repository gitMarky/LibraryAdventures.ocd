/**
	Attach to a non player character to provide a message interface.

	@title Dialogue
	@author Marky
	@credits Sven2 for the original dialogue object that this code is based on.
    @version 0.1.0
*/

#include Library_Dialogue

/*-- Dialogue creation --*/

/**
 Sets a new dialogue for a npc.
 @par name The dialogue name. // TODO
 @par attention If set to true, then this dialogue will 
                have an attention marker displayed initially.
 @version 0.1.0
 */
global func SetDialogueEx(string name, bool attention)
{
	if (!this)
		return;
	var dialogue = CreateObject(DialogueEx, 0, 0, NO_OWNER);
	dialogue->InitDialogue(name, this, attention);
	
	dialogue->SetObjectLayer(nil);
	dialogue.Plane = this.Plane+1; // for proper placement of the attention symbol

	return dialogue;
}

/**
 Removes the existing dialogue of an object.
 @version 0.1.0
 */
global func RemoveDialogueEx()
{
	var dialogue = DialogueEx->FindByTarget(this);
	if (dialogue) return dialogue->RemoveObject();
	return false;
}

/**
  Find dialogue attached to a target (definition call, e.g. var dlg = DialogueEx->FindByTarget(foo))
  @par target The object or NPC that starts the dialogue.
  @version 0.1.0
  */
func FindByTarget(object target)
{
	if (!target) return nil;
	return FindObject(Find_ID(Dialogue), Find_ActionTarget(target));
}

/*-- Dialogue properties --*/

protected func Initialize()
{
	// Dialogue progress to one.
	_inherited(...);
	// Dialogue allows interaction by default.
	dlg_interact = true; // TODO: Replace with method calls
	// Dialogue is active by default.
	dlg_status = DLG_Status_Active; // TODO: Replace with method calls.
}

public func InitDialogue(string name, object target, bool attention)
{
	dlg_target = target; // TODO: Replace with method calls
	dlg_name = name;

	// Attach dialogue object to target.
	if (attention)
	{
		// Attention: Show exclamation mark and glitter effect every five seconds
		AddAttention();
	}
	else
	{
		// No attention: Set invisible action
		SetAction("Dialogue", target);
		RemoveAttention();
	}
	
	// Update dialogue to target.
	UpdateDialogue();
	
	// Effect on targets to remove the dialogue when target dies or is removed
	AddEffect("IntDialogue", target, 1, 0, this);

	// Custom dialogue initialization
	if (!Call(Format("~Dlg_%s_Init", dlg_name), dlg_target))
		GameCall(Format("~Dlg_%s_Init", dlg_name), this, dlg_target);
	
	return true;
}

private func FxIntDialogueStop(object target, proplist fx, int reason, bool temp)
{
	// Target removed or died: Remove dialogue
	if (!temp) RemoveObject();
	return FX_OK;
}

/**
 Adds an attention marker to this dialogue.
 @version 0.1.0
 */
public func AddAttention()
{
	// Attention: Show exclamation mark and glitter effect every five seconds
	if (!dlg_attention)
	{
		SetAction("DialogueAttention", dlg_target);
		RemoveTimer("AttentionEffect"); AddTimer("AttentionEffect", 36*5);
		dlg_attention = true;
	}
	return true; // TODO: remove return value?
}

/**
 Removes an existing attention marker from this dialogue.
 @version 0.1.0
 */
public func RemoveAttention()
{
	// No longer show exclamation mark and glitter effects
	if (dlg_attention)
	{
		RemoveTimer("AttentionEffect");
		if (dlg_target) SetAction("Dialogue", dlg_target);
		dlg_attention = false;
	}
	return true; // TODO: Remove return value
}

private func AttentionEffect()
{
	return SetAction("DialogueAttentionEffect", dlg_target);
}

private func UpdateDialogue()
{
	// Adapt size to target. Updating to direction does not work well for NPCs that walk around 
	// It's also not very intuitive if the player just walks to the attention symbol anyway.
	var wdt = dlg_target->GetID()->GetDefWidth() + 10;
	var hgt = dlg_target->GetID()->GetDefHeight();
	//var dir = dlg_target->GetDir();
	SetShape(-wdt/2, -hgt/2, wdt, hgt);
	// Transfer target name.
	//SetName(Format("$MsgSpeak$", dlg_target->GetName()));
}

public func SetDialogueInfo()
{
	// does nothing?
}

/**
 Sets the possibility to call this dialogue with the
 object interaction interface.
 @par allow If set to {@c true}, then the dialogue can be called
            by interaction.
 @version 0.1.0
 */
public func SetInteraction(bool allow)
{
	dlg_interact = allow;
}

/**
 Sets the dialogue progress.
 @int progress //TODO
 @version 0.1.0
 */
public func SetDialogueProgress(int progress, string section, bool add_attention)
{
	dlg_progress = Max(1, progress);
	dlg_section = section;
	if (add_attention) AddAttention();
}

public func SetDialogueStatus(int status)
{
	dlg_status = status;
}

/**
 To be called from within dialogue after the last message.
 @version 0.1.0
 */
public func StopDialogue()
{
	// put on wait for a while; then reenable
	SetDialogueStatus(DLG_Status_Wait);
	ScheduleCall(this, this.SetDialogueStatus, 30, 1, DLG_Status_Stop);
	return true; // TODO: remove return value?
}

/*-- Interaction --*/

// Players can talk to NPC via the interaction bar.
public func IsInteractable() { return dlg_interact; }

// Adapt appearance in the interaction bar.
public func GetInteractionMetaInfo(object clonk)
{
	if (InDialogue(clonk))
		return { Description = Format("$MsgSpeak$", dlg_target->GetName()) , IconName = nil, IconID = Clonk, Selected = true };

	return { Description = Format("$MsgSpeak$", dlg_target->GetName()) , IconName = nil, IconID = Clonk, Selected = false };
}

// Advance dialogue from script
public func CallDialogue(object clonk, progress, string section)
{
	if (GetType(progress)) SetDialogueProgress(progress, section);
	return Interact(clonk);
}

// Called on player interaction.
public func Interact(object clonk)
{
	// Should not happen: not active -> stop interaction
	if (!dlg_interact)
		return true;	
	
	// Currently in a dialogue: abort that dialogue.
	//if (InDialogue(clonk))
	//	clonk->CloseMenu();	
	
	// No conversation context: abort.
	//if (!dlg_name)
	//	return true;
		
	// Dialogue still waiting? Do nothing then
	// (A sound might be nice here)
	//if (dlg_status == DLG_Status_Wait)
	//{
	//	return true;
	//}
		
	// Stop dialogue?
	//if (dlg_status == DLG_Status_Stop)
	//{
	//	clonk->CloseMenu();
	//	dlg_status = DLG_Status_Active;
	//	return true;
	//}
	// Remove dialogue?
	//if (dlg_status == DLG_Status_Remove)
	//{
	//	clonk->CloseMenu();
	//	RemoveObject();
	//	return true;		
	//}

	if (ProgressDialogue(clonk))
	{	
		// Remove attention mark on first interaction
		RemoveAttention();
		
		// Have speakers face each other
		SetSpeakerDirs(dlg_target, clonk);
	}

	return true;
}


public func MenuOK(proplist menu_id, object clonk)
{
	// prevent the menu from closing when pressing MenuOK
	if (dlg_interact)
		Interact(clonk);
}

public func SetSpeakerDirs(object speaker1, object speaker2)
{
	// Force direction of two clonks to ace each other for dialogue
	if (!speaker1 || !speaker2) return false;
	speaker1->SetDir(speaker1->GetX() < speaker2->GetX());
	speaker2->SetDir(speaker1->GetX() > speaker2->GetX());
	return true;
}

/* Scenario saving */

// Scenario saving
func SaveScenarioObject(props)
{
	if (!inherited(props, ...)) return false;
	if (!dlg_target) return false; // don't save dead dialogue object
	// Dialog has its own creation procedure
	props->RemoveCreation();
	props->Remove("Plane"); // updated when setting dialogue
	props->Add(SAVEOBJ_Creation, "%s->SetDialogue(%v,%v)", dlg_target->MakeScenarioSaveName(), dlg_name, !!dlg_attention);
	// Force dependency on all contained objects, so dialogue initialization procedure can access them
	var i=0, obj;
	while (obj = dlg_target->Contents(i++)) obj->MakeScenarioSaveName();
	return true;
}


/* Properties */

local ActMap = {
	Dialogue = {
		Prototype = Action,
		Name = "Dialogue",
		Procedure = DFA_ATTACH,
		Delay = 0,
		NextAction = "Dialogue",
	},
	DialogueAttention = {
		Prototype = Action,
		Name = "DialogueAttention",
		Procedure = DFA_ATTACH,
		X = 0, Y = 0, Wdt = 8, Hgt = 24, OffX = 0, OffY = -30,
		Delay = 0,
		NextAction = "DialogueAttention",
	},
	DialogueAttentionEffect = {
		Prototype = Action,
		Name = "DialogueAttentionEffect",
		Procedure = DFA_ATTACH,
		X = 0, Y = 0, Wdt = 8, Hgt = 24, OffX = 0, OffY = -30,
		Delay = 2,
		Length = 4,
		NextAction = "DialogueAttentionREffect",
	},
	DialogueAttentionREffect = {
		Prototype = Action,
		Name = "DialogueAttentionREffect",
		Procedure = DFA_ATTACH,
		X = 0, Y = 0, Wdt = 8, Hgt = 24, OffX = 0, OffY = -30,
		Delay = 2,
		Length = 4,
		Reverse = 1,
		NextAction = "DialogueAttention",
	}
};

local Name = "$Name$";
