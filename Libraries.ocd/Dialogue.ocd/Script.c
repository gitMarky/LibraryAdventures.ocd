

public func DlgText(string text, object speaker)
{
	Log("Progress is %d, visiting %d (%s)", dlg_progress, dlg_internal, text);
	// TODO
	if (dlg_internal == dlg_progress)
	{
		MessageBox(text, dlg_target, speaker);
	}
	++dlg_internal;
}

public func DlgOption(string text)
{
	// TODO
	var result = dlg_internal == dlg_progress;
	if (result)
	{
		++dlg_option;
	}
	++dlg_internal;
	return result || dlg_option > 0;
}

public func DlgReset()
{
	// TODO
	if (dlg_internal == dlg_progress)
	{
		dlg_progress = 0;
	}
	++dlg_internal;
}

public func DlgOptionEnd()
{
	// TODO
	// not sure if this is necessary
	// it is!
	if (dlg_internal == dlg_progress)
	{
		dlg_option = Max(0, dlg_option -1);
	}
	++dlg_internal;
}

public func DlgEvent()
{
	// TODO
	// not sure if this is necessary
}

//-------------------------------------------------------------------------
// properties

local dlg_target; // the npc that provides this dialogue?
local dlg_name;
local dlg_info;
local dlg_progress; // the player progress
local dlg_internal; // the internal id
local dlg_section;   // if set, this string is included in progress callbacks (i.e., func Dlg_[Name]_[Section][Progress]() is called)
local dlg_status;
local dlg_interact;  // default true. can be set to false to deactivate the dialogue
local dlg_attention; // if set, a red attention mark is put above the clonk
local dlg_broadcast; // if set, all non-message (i.e. menu) MessageBox calls are called as MessageBoxBroadcast.
local dlg_option; // branch depth

//-------------------------------------------------------------------------
// internal functions

protected func Initialize()
{
	ResetDialogue();
	_inherited(...);
}

private func ResetDialogue()
{
	dlg_progress = -1;
	dlg_internal = -1;
}

private func InDialogue(object player)
{
	return player->GetMenu() == Dialogue
	    || player->GetMenu() == DialogueEx;
}

private func CloseMessageBox(object player)
{
	player->CloseMenu();
}


//-------------------------------------------------------------------------
// logic

/**
 Called on player interaction.
 @author Sven2
 @version 0.1.0
 */
public func ProgressDialogue(object player)
{
	// Currently in a dialogue: abort that dialogue.
	if (InDialogue(player))
		CloseMessageBox(player);	
	
	// No conversation context: abort.
	//if (!dlg_name)
	//	return true;
		
	// Dialogue still waiting? Do nothing then
	// (A sound might be nice here)
	if (dlg_status == DLG_Status_Wait)
	{
		return false;
	}
		
	// Stop dialogue?
	if (dlg_status == DLG_Status_Stop)
	{
		CloseMessageBox(player);
		dlg_status = DLG_Status_Active;
		return false;
	}

	// Remove dialogue?
	if (dlg_status == DLG_Status_Remove)
	{
		CloseMessageBox(player);
		RemoveObject();
		return false;		
	}
	
	// Remove attention mark on first interaction
	// RemoveAttention();
	
	// this is actually a part of the speaker object!
	// Have speakers face each other
	// SetSpeakerDirs(dlg_target, clonk);

	// Start conversation context.
	// Update dialogue progress first.
	dlg_target = player;
	//var progress = dlg_progress;
	dlg_progress++;
	dlg_internal = 0;
	// Then call relevant functions.
	// Call generic function first, then progress function
	//var fn_generic = Format("~Dlg_%s", dlg_name);
	//var fn_progress = Format("~Dlg_%s_%s%d", dlg_name, dlg_section ?? "", progress);
	//if (!Call(fn_generic, player))
	//	if (!GameCall(fn_generic, this, player, dlg_target))
	//		if (!Call(fn_progress, player))
	//			GameCall(fn_progress, this, player, dlg_target);
	var fn_generic = Format("Dlg_%s", dlg_name);
	Call(fn_generic, player);
	return true;
}

//------------------------------------------------------------------------
// message boxes
// since these are unchanged... maybe have them in another library in the objects??

/**
 A message box for all players 
 @author Sven2
 @version 0.1.0
 */
public func MessageBoxAll(string message, object talker, bool as_message)
{
	for(var i = 0; i < GetPlayerCount(C4PT_User); ++i)
	{
		var plr = GetPlayerByIndex(i, C4PT_User);
		MessageBox(message, GetCursor(plr), talker, plr, as_message);
	}
}

/**
 Message box as dialog to player with a message copy to all other players
 @author Sven2
 @version 0.1.0
 */
public func MessageBoxBroadcast(string message, object clonk, object talker, array options)
{
	// message copy to other players
	for(var i = 0; i < GetPlayerCount(C4PT_User); ++i)
	{
		var plr = GetPlayerByIndex(i, C4PT_User);
		if (GetCursor(plr) != clonk)
			MessageBox(message, GetCursor(plr), talker, plr, true);
	}
	// main message as dialog box
	return MessageBox(message, clonk, talker, nil, false, options);
}

static MessageBox_last_talker, MessageBox_last_pos;


/**
 A single message box.
 @author Sven2
 @version 0.1.0
 */
private func MessageBox(string message, object clonk, object talker, int for_player, bool as_message, array options)
{
	// broadcast enabled: message copy to other players
	if (dlg_broadcast && !as_message)
	{
		for(var i = 0; i < GetPlayerCount(C4PT_User); ++i)
		{
			var other_plr = GetPlayerByIndex(i, C4PT_User);
			if (GetCursor(other_plr) != clonk)
				MessageBox(message, GetCursor(other_plr), talker, other_plr, true);
		}
	}
	// Use current NPC as talker if unspecified.
	// On definition call or without talker, just show the message without a source
	if (!talker && this != Dialogue) talker = dlg_target;
	if (talker) message = Format("<c %x>%s:</c> %s", talker->GetColor(), talker->GetName(), message);
	var portrait;
	if (talker) portrait = talker->~GetPortrait();
	
	// A target Clonk is given: Use a menu for this dialogue.
	if (clonk && !as_message)
	{
		var menu_target, cmd;
		if (this != Dialogue)
		{
			menu_target = this;
			cmd = "MenuOK";
		}
		clonk->CreateMenu(Dialogue, menu_target, C4MN_Extra_None, nil, nil, C4MN_Style_Dialog, false, Dialogue);
		
		// Add NPC portrait.
		//var portrait = Format("%i", talker->GetID()); //, Dialogue, talker->GetColor(), "1");
		if (talker)
			if (portrait)
				clonk->AddMenuItem("", nil, Dialogue, nil, clonk, nil, C4MN_Add_ImgPropListSpec, portrait);
			else
				clonk->AddMenuItem("", nil, Dialogue, nil, clonk, nil, C4MN_Add_ImgObject, talker);

		// Add NPC message.
		clonk->AddMenuItem(message, nil, nil, nil, clonk, nil, C4MN_Add_ForceNoDesc);
		
		// Add answers.
		if (options) for (var option in options)
		{
			var option_text, option_command;
			if (GetType(option) == C4V_Array)
			{
				// Text+Command given
				option_text = option[0];
				option_command = option[1];
				if (GetChar(option_command) == GetChar("#"))
				{
					// Command given as section name: Remove leading # and call section change
					var ichar=1, ocmd = "", c;
					while (c = GetChar(option_command, ichar++)) ocmd = Format("%s%c", ocmd, c);
					option_command = Format("CallDialogue(Object(%d), 1, \"%s\")", clonk->ObjectNumber(), ocmd);
				}
				else
				{
					// if only a command is given, the standard parameter is just the clonk
					if (!WildcardMatch(option_command, "*(*")) option_command = Format("%s(Object(%d))", option_command, clonk->ObjectNumber());
				}
			}
			else
			{
				// Only text given - command means regular dialogue advance
				option_text = option;
				option_command = cmd;
			}
			clonk->AddMenuItem(option_text, option_command, nil, nil, clonk, nil, C4MN_Add_ForceNoDesc);
		}
		
		// If there are no answers, add a next entry
		if (cmd && !options) clonk->AddMenuItem("$Next$", cmd, nil, nil, clonk, nil, C4MN_Add_ForceNoDesc);
		
		// Set menu decoration.
		clonk->SetMenuDecoration(GUI_MenuDeco);
		
		// Set text progress to NPC name.
		if (talker)
		{
			var name = talker->GetName();
			var n_length;
			while (GetChar(name, n_length))
				n_length++;
			clonk->SetMenuTextProgress(n_length + 1);
		}
	}
	else
	{
		// No target is given: Global (player) message
		if (!GetType(for_player)) for_player = NO_OWNER;
		// altenate left/right position as speakers change
		if (talker != MessageBox_last_talker) MessageBox_last_pos = !MessageBox_last_pos;
		MessageBox_last_talker = talker;
		var flags = 0, xoff = 150;
		if (!MessageBox_last_pos)
		{
			flags = MSG_Right;
			xoff *= -1;
			CustomMessage("", nil, for_player); // clear prev msg
		}
		else
		{
			CustomMessage("", nil, for_player, 0,0, nil, nil, nil, MSG_Right);  // clear prev msg
		}
		CustomMessage(message, nil, for_player, xoff,150, nil, GUI_MenuDeco, portrait ?? talker, flags);
	}

	return;
}
