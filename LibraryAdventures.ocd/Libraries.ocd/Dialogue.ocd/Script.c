

/**
 * Displays text.
 * If multiple calls to this function are executed in a row, then
 * the text should be output in several message boxes, where the user
 * has to confirm each of the messages individually.
 *
 * @par text this text will be displayed.
 * @par speaker the portrait of this object will be displayed. 
                By default this is the object that the user is
                speaking to.
 */
public func DlgText(string text, object speaker)
{
	//Log("Progress is %d, visiting %d (%s)", dlg_progress, dlg_internal, text);
	// TODO
	if (dlg_internal == dlg_progress)
	{
		MessageBox(text, dlg_player, speaker);
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
	//Log("Progress is %d, resetting number is %d", dlg_progress, dlg_internal);
	if (dlg_internal == dlg_progress)
	{
		ResetDialogue();
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

/**
 Adds an event to the dialogue.
 @return Returns true, if the dialogue event should be performed.
 @example This dialogue gives a cookie to the player every time that it is called.
 {@code
     Dlg_Cookie(object player)
     {
         DlgText("Here, have a cookie!"); // displays the message
         if (DlgEvent()) // is called after the message is displayed
         {
             player->CreateContents(Cookie); // gives the cookie to the player
         }
     }
 } 
 */
public func DlgEvent(int delay)
{
	var execute_event = false;
	if (dlg_internal == dlg_progress)
	{
		execute_event = true;
		ScheduleCall(this, this.ProgressDialogue, 1, nil, dlg_player);
	} // progress should be increased too
	++dlg_internal;
	return execute_event;
}

//-------------------------------------------------------------------------
// properties

local dlg_target; // the npc that provides this dialogue?
local dlg_player; // the player who we are talking to
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
	// No conversation context: abort.
	if (!dlg_name)
	{
		FatalError("This function needs a dialogue name, but it is not set.");
	}

	// Currently in a dialogue: abort that dialogue.
	if (InDialogue(player))
		CloseMessageBox(player);	

		
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
	
	// Start conversation context.
	// Update dialogue progress first.
	dlg_player = player;
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
	
	//Log("--> After dialogue: progress %d, internal %d", dlg_progress, dlg_internal);
	if (dlg_progress >= dlg_internal)
	{
		ResetDialogue();
	}
	
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
public func MessageBoxAll(string message, object speaker, bool as_message)
{
	for(var i = 0; i < GetPlayerCount(C4PT_User); ++i)
	{
		var plr = GetPlayerByIndex(i, C4PT_User);
		MessageBox(message, GetCursor(plr), speaker, plr, as_message);
	}
}

/**
 Message box as dialog to player with a message copy to all other players
 @author Sven2
 @version 0.1.0
 */
public func MessageBoxBroadcast(string message, object player, object speaker, array options)
{
	// message copy to other players
	for(var i = 0; i < GetPlayerCount(C4PT_User); ++i)
	{
		var plr = GetPlayerByIndex(i, C4PT_User);
		if (GetCursor(plr) != player)
			MessageBox(message, GetCursor(plr), speaker, plr, true);
	}
	// main message as dialog box
	return MessageBox(message, player, speaker, nil, false, options);
}

static MessageBox_last_speaker, MessageBox_last_pos;


/**
 A single message box.
 @author Sven2
 @version 0.1.0
 */
private func MessageBox(string message, object player, object speaker, int for_player, bool as_message, array options)
{
	// broadcast enabled: message copy to other players
	if (dlg_broadcast && !as_message)
	{
		for(var i = 0; i < GetPlayerCount(C4PT_User); ++i)
		{
			var other_plr = GetPlayerByIndex(i, C4PT_User);
			if (GetCursor(other_plr) != player)
				MessageBox(message, GetCursor(other_plr), speaker, other_plr, true);
		}
	}
	// Use current NPC as speaker if unspecified.
	// On definition call or without speaker, just show the message without a source
	var portrait;
	if (!speaker && this != DialogueEx) speaker = dlg_target;
	if (speaker)
	{
		message = Format("<c %x>%s:</c> %s", speaker->GetColor(), speaker->GetName(), message);
	    portrait = speaker->~GetPortrait();
	}
	
	// A target player is given: Use a menu for this dialogue.
	if (player && !as_message)
	{
		var menu_target, cmd;
		if (this != DialogueEx)
		{
			menu_target = this;
			cmd = "MenuOK";
		}
		player->CreateMenu(DialogueEx, menu_target, C4MN_Extra_None, nil, nil, C4MN_Style_Dialog, false, DialogueEx);
		
		// Add NPC portrait.
		//var portrait = Format("%i", speaker->GetID()); //, DialogueEx, speaker->GetColor(), "1");
		if (speaker)
			if (portrait)
				player->AddMenuItem("", nil, DialogueEx, nil, player, nil, C4MN_Add_ImgPropListSpec, portrait);
			else
				player->AddMenuItem("", nil, DialogueEx, nil, player, nil, C4MN_Add_ImgObject, speaker);

		// Add NPC message.
		player->AddMenuItem(message, nil, nil, nil, player, nil, C4MN_Add_ForceNoDesc);
		
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
					option_command = Format("CallDialogue(Object(%d), 1, \"%s\")", player->ObjectNumber(), ocmd);
				}
				else
				{
					// if only a command is given, the standard parameter is just the player
					if (!WildcardMatch(option_command, "*(*")) option_command = Format("%s(Object(%d))", option_command, player->ObjectNumber());
				}
			}
			else
			{
				// Only text given - command means regular dialogue advance
				option_text = option;
				option_command = cmd;
			}
			player->AddMenuItem(option_text, option_command, nil, nil, player, nil, C4MN_Add_ForceNoDesc);
		}
		
		// If there are no answers, add a next entry
		if (cmd && !options) player->AddMenuItem("$Next$", cmd, nil, nil, player, nil, C4MN_Add_ForceNoDesc);
		
		// Set menu decoration.
		player->SetMenuDecoration(GUI_MenuDeco);
		
		// Set text progress to NPC name.
		if (speaker)
		{
			var name = speaker->GetName();
			var n_length;
			while (GetChar(name, n_length))
				n_length++;
			player->SetMenuTextProgress(n_length + 1);
		}
	}
	else
	{
		// No target is given: Global (player) message
		if (!GetType(for_player)) for_player = NO_OWNER;
		// altenate left/right position as speakers change
		if (speaker != MessageBox_last_speaker) MessageBox_last_pos = !MessageBox_last_pos;
		MessageBox_last_speaker = speaker;
		var flags = 0, xoff = 150;
		if (!MessageBox_last_pos)
		{
			flags = MSG_Right;
			xoff *= -1;
			CustomMessage("", nil, for_player); // clear prev msg
		}
		else
		{
			CustomMessage("", nil, for_player, 0, 0, nil, nil, nil, MSG_Right);  // clear prev msg
		}
		CustomMessage(message, nil, for_player, xoff, 150, nil, GUI_MenuDeco, portrait ?? speaker, flags);
	}

	return;
}
