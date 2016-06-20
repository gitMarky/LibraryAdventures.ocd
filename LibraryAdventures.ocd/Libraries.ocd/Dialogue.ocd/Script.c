
/**
 Displays text.
 If multiple calls to this function are executed in a row, then
 the text should be output in several message boxes, where the user
 has to confirm each of the messages individually.
 
 @par text this text will be displayed.
 @par speaker the portrait of this object will be displayed. 
              By default this is the object that the user is
              speaking to.
 */
public func DlgText(string text, object speaker)
{
	//Log("Progress is %d, visiting %d (%s), Layer = %d", dlg_progress[dlg_layer], dlg_internal[dlg_layer], text, dlg_layer);
	
	var log_output = Format("L(%d)/I(%d): %s", dlg_layer, dlg_internal[dlg_layer], text); 
	
	// TODO	
	if (dlg_internal[dlg_layer] == dlg_progress[dlg_layer])
	{
		BroadcastDialogue({ Prototype = DlgMessage(), text = text, sender = speaker ?? dlg_target, receiver = dlg_player});
		Log("* %s", log_output);
	}
	else
	{
		Log("  %s", log_output);
	}
	
	dlg_last_nonoption[dlg_layer] = dlg_internal[dlg_layer];
	++dlg_internal[dlg_layer];
}


public func DlgOption(string text)
{
	// TODO
	var internal = dlg_internal[dlg_layer] + dlg_internal_option[dlg_layer];
	var selected_now = internal == dlg_progress[dlg_layer];
	var selected_previously = (dlg_option[dlg_layer] == internal);
	var possible_option = dlg_internal[dlg_layer] >= dlg_progress[dlg_layer];
	var under_text = dlg_progress[dlg_layer] == dlg_last_nonoption[dlg_layer];
	var add_option = possible_option && under_text;
	
	var log_output = Format("L(%d)/I(%d): %s", dlg_layer, dlg_internal[dlg_layer], text); 
	
	
	//Log("Visiting dialogue option: dlg_internal = %d / %d, dlg_progress = %d, text = %s, selected prev = %v, selected now = %v, add = %v", dlg_internal[dlg_layer], internal, dlg_progress[dlg_layer], text, selected_previously, selected_now, add_option);
	if (selected_now)
	{
		Log("* %s", log_output);
		//Log("* Took option: %s", text);
		if (!selected_previously)
		{
			Log("* Going one layer deeper");
			dlg_option[dlg_layer] = internal; // save the latest option
			dlg_request_layer_increase = true;
		}
		ProgressDialogueDelayed();
	}
	else
	{
		Log("  %s", log_output);
	}
	
	
	if (add_option)
	{
		BroadcastOption({ Prototype = DlgMessage(), text = text, receiver = dlg_player, override_progress = internal}); // the progress has to be set here, if the option is chosen!
	}
	
	++dlg_internal_option[dlg_layer];
	if (selected_previously)
	{
		dlg_internal[dlg_layer] += dlg_internal_option[dlg_layer];
	}
	return selected_previously; 
}


/**
 Resets the dialogue, that is, the dialogue will start at the beginning again.
 @example This will create an endless loop:
 {@code
     Dlg_Loop(object player)
     {
         DlgText("Are you tired of this message yet?");
         DlgReset(); // starts the dialogue again
     }
 }
 */
public func DlgReset()
{
	var log_output = Format("L(%d)/I(%d): DlgReset()", dlg_layer, dlg_internal[dlg_layer]); 

	//Log("Progress is %d, resetting number is %d", dlg_progress, dlg_internal);
	var execute_event = false;
	if (dlg_internal[dlg_layer] == dlg_progress[dlg_layer])
	{
		execute_event = true;
		ResetDialogue();
		ProgressDialogueDelayed();
		Log("* %s", log_output);
	}
	else
	{
		Log("  %s", log_output);
	}
	
	dlg_last_nonoption[dlg_layer] = dlg_internal[dlg_layer];
	++dlg_internal[dlg_layer];
	return execute_event;
}


public func DlgOptionEnd()
{
	var log_output = Format("L(%d)/I(%d): DlgOptionEnd()", dlg_layer, dlg_internal[dlg_layer]); 
	// TODO
	// not sure if this is necessary
	// it is!
	if (dlg_internal[dlg_layer] == dlg_progress[dlg_layer])
	{
//			Log("* Going one layer higher");
		--dlg_layer;
		PopBack(dlg_option); // remove the last option
		Log("*  %s", log_output);
	}
	else
	{
		Log("  %s", log_output);
	}

	dlg_last_nonoption[dlg_layer] = dlg_internal[dlg_layer];
}


/**
 Adds an event to the dialogue.
 @par delay the dialogue will wait this many frames after performing the event.
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
public func DlgEvent()
{
	var log_output = Format("L(%d)/I(%d): DlgEvent()", dlg_layer, dlg_internal[dlg_layer]); 

	var execute_event = false;
	if (dlg_internal[dlg_layer] == dlg_progress[dlg_layer])
	{
		execute_event = true;
		ProgressDialogueDelayed();
		Log("* %s", log_output);
	} // progress should be increased too
	else
	{
		Log("  %s", log_output);
	}
	
	dlg_last_nonoption[dlg_layer] = dlg_internal[dlg_layer];
	++dlg_internal[dlg_layer];
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
local dlg_internal_option; // the internal option id
local dlg_last_nonoption; // the internal id of the last dialogue that was not an option
local dlg_section;   // if set, this string is included in progress callbacks (i.e., func Dlg_[Name]_[Section][Progress]() is called)
local dlg_status;
local dlg_interact;  // default true. can be set to false to deactivate the dialogue
local dlg_attention; // if set, a red attention mark is put above the clonk
local dlg_option; // branch depth
local dlg_listeners; // array of all objects that are listening to the dialogue
local dlg_layer; // layer of the dialogue, important for options processing
local dlg_request_layer_increase;


//-------------------------------------------------------------------------
// internal functions

protected func Initialize()
{
	dlg_layer = 0;
	dlg_option = [];
	dlg_listeners = [];
	dlg_progress = [];
	dlg_internal = [];
	dlg_internal_option = [];
	dlg_last_nonoption = [];
	ResetDialogue();
	_inherited(...);
}


protected func Destruction()
{
	BroadcastDestruction();
	_inherited(...);
}


private func ResetDialogue(int layer)
{
	if (layer == nil)
	{
		dlg_layer = 0;
		dlg_progress = [];
		dlg_internal = [];
		dlg_internal_option = [];
		dlg_last_nonoption = [];
		dlg_progress[0] = -1;
		dlg_last_nonoption[0] = -1;
		dlg_option = [];
	}
	else
	{
		dlg_progress[layer] = -1;
		dlg_last_nonoption[layer] = -1;

		// advance progress in the previous layer
		if (dlg_request_layer_increase)
		{
			dlg_layer += 1;
			dlg_request_layer_increase = false;
			ProgressDialogueDelayed(); // Go on in the next layer
		}
		else
		{
			if (layer > 0)
			{
				dlg_layer -= 1;
				dlg_progress[layer] += 1;
				dlg_last_nonoption[layer] = dlg_progress;
			}
		}
	}
}


private func InDialogue(object player)
{
	return player->GetMenu() == Dialogue
	    || player->GetMenu() == DialogueEx;
}


//-------------------------------------------------------------------------
// logic


/**
 Called on player interaction.
 @author Sven2
 @version 0.1.0
 */
public func ProgressDialogue(object player, int override)
{
	// No conversation context: abort.
	if (!dlg_name)
	{
		FatalError("This function needs a dialogue name, but it is not set.");
	}

	// Currently in a dialogue: abort that dialogue.
	if (InDialogue(player))
		BroadcastClose(player);

		
	// Dialogue still waiting? Do nothing then
	// (A sound might be nice here)
	if (dlg_status == DLG_Status_Wait)
	{
		return false;
	}

		
	// Stop dialogue?
	if (dlg_status == DLG_Status_Stop)
	{
		BroadcastClose(player);
		dlg_status = DLG_Status_Active;
		return false;
	}


	// Remove dialogue?
	if (dlg_status == DLG_Status_Remove)
	{
		BroadcastClose(player);
		RemoveObject();
		return false;		
	}
		
	// Start conversation context.
	// Update dialogue progress first.
	dlg_player = player;
	if (override)
	{
		dlg_progress[dlg_layer] = override;
		//Log(">>> Override set to %d", override);
	}
	else
	{
		dlg_progress[dlg_layer]++;
	}

	Log("-----------------------------------------");
	Log("Progress dialogue: %v", dlg_progress);
	
	for (var i = 0; i <= dlg_layer; ++i)
	{	
		dlg_internal[i] = 0;
		dlg_internal_option[i] = 0;
	}
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
	if (dlg_progress[dlg_layer] >= dlg_internal[dlg_layer])
	{
		ResetDialogue(dlg_layer);
	}
	
	return true;
}


private func ProgressDialogueDelayed(int delay, int override)
{
	ScheduleCall(this, this.ProgressDialogue, delay ?? 1, nil, dlg_player, override);
}


//-------------------------------------------------------------------------
// broadcasting


private func DoBroadcast(string function, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
{
	for (var listener in dlg_listeners)
	{
		if (listener == nil) continue;
		
		listener->Call(listener[function], arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	}
}


public func BroadcastDialogue(proplist message)
{
	//Log("* Broadcasting dialogue %s", message.text);
	DoBroadcast("OnBroadcastDialogue", message);
}


public func BroadcastOption(proplist message)
{
	//Log("* Broadcasting option %s", message.text);
	DoBroadcast("OnBroadcastOption", message);
}


public func BroadcastClose(object player)
{
	DoBroadcast("OnBroadcastClose", player);
}


public func BroadcastDestruction()
{
	DoBroadcast("OnBroadcastDestruction");
}

public func AddListener(object listener)
{
	if (listener == nil)
	{
		FatalError("Listener must not be nil.");
	}

	if (!IsValueInArray(dlg_listeners, listener))
	{
		PushBack(dlg_listeners, listener);
	}
}


public func RemoveListener(object listener)
{
	RemoveArrayValue(dlg_listeners, listener);
}