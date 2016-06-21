
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
	++dlg_internal[dlg_internal_layer];
	
	var log_output = Format("L(%d)/I(%d): %s", dlg_internal_layer, dlg_internal[dlg_layer], text); 
	
	// TODO	
	if (dlg_internal[dlg_layer] == dlg_progress[dlg_internal_layer])
	{
		BroadcastDialogue({ Prototype = DlgMessage(), text = text, sender = speaker ?? dlg_target, receiver = dlg_player});
		Log("* %s", log_output);
	}
	else
	{
		Log("  %s", log_output);
	}	
}


public func DlgOption(string text)
{
	++dlg_internal_option[dlg_internal_layer];
	// TODO
	var display_option = (dlg_layer == dlg_internal_layer)
	                  && (dlg_progress[dlg_layer] == dlg_internal[dlg_layer]);
	var was_chosen = dlg_option[dlg_internal_layer] == dlg_internal_option[dlg_internal_layer];
	
	var log_output = Format("L(%d)/I(%d): %s, display = %v, chosen = %v", dlg_internal_layer, dlg_internal[dlg_layer], text, display_option, was_chosen); 
	
	if (was_chosen)
	{
		Log("* %s", log_output);
	}
	else
	{
		Log("  %s", log_output);
	}
	
	
	if (display_option)
	{
		BroadcastOption({ Prototype = DlgMessage(), text = text, receiver = dlg_player, option_choice = dlg_internal_option[dlg_internal_layer]}); // the progress has to be set here, if the option is chosen!
	}
	
	++dlg_internal_layer;
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
	++dlg_internal[dlg_internal_layer];
	var log_output = Format("L(%d)/I(%d): DlgReset()", dlg_internal_layer, dlg_internal[dlg_layer]); 

	//Log("Progress is %d, resetting number is %d", dlg_progress, dlg_internal);
	var execute_event = false;
	if (dlg_internal[dlg_layer] == dlg_progress[dlg_internal_layer])
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
	
	return execute_event;
}


public func DlgOptionEnd()
{
	++dlg_internal[dlg_internal_layer];
	var log_output = Format("L(%d)/I(%d): DlgOptionEnd()", dlg_internal_layer, dlg_internal[dlg_layer]); 
	// TODO
	// not sure if this is necessary
	// it is!
	if ((dlg_layer == dlg_internal_layer) && (dlg_internal[dlg_layer] == dlg_progress[dlg_internal_layer]))
	{
		//dlg_option[dlg_internal_layer] = nil;
		Log("* %s", log_output);
		ResetDialogue(dlg_layer);
		ProgressDialogueDelayed();
	}
	else
	{
		Log("  %s", log_output);
	}
	
	--dlg_internal_layer;
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
	++dlg_internal[dlg_internal_layer];
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
	
	return execute_event;
}


//-------------------------------------------------------------------------
// properties


local dlg_target; // the npc that provides this dialogue?
local dlg_player; // the player who we are talking to
local dlg_name;
local dlg_info;
local dlg_section;   // if set, this string is included in progress callbacks (i.e., func Dlg_[Name]_[Section][Progress]() is called)
local dlg_status;
local dlg_interact;  // default true. can be set to false to deactivate the dialogue
local dlg_attention; // if set, a red attention mark is put above the clonk
local dlg_listeners; // array of all objects that are listening to the dialogue

local dlg_progress; // the player progress
local dlg_option; // branch depth
local dlg_layer; // layer of the dialogue, important for options processing

local dlg_internal; // the internal id
local dlg_internal_option; // the internal option id
local dlg_internal_layer;


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
		dlg_progress = [];
		dlg_internal = [];
		dlg_internal_option = [];
		dlg_option = [];
	}
	else
	{
		dlg_progress[layer] = 0;

		// advance progress in the previous layer
		if (layer > 0)
		{
			dlg_layer -= 1;
			dlg_internal_option = [];
			dlg_option[dlg_layer] = nil;
			dlg_progress[layer] = nil;
			//dlg_progress[dlg_layer] += 1;
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
public func ProgressDialogue(object player, int option_choice)
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
	if (option_choice)
	{
		Log("Took option %d", option_choice);
		dlg_option[dlg_layer] = option_choice;
		dlg_layer += 1;
	}
	dlg_progress[dlg_layer]++;

	Log("-----------------------------------------");
	Log("Progress dialogue: %v, layer = %v", dlg_progress, dlg_layer);
	
	dlg_internal_layer = 0;
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


private func ProgressDialogueDelayed(int delay, int option_choice)
{
	ScheduleCall(this, this.ProgressDialogue, delay ?? 1, nil, dlg_player, option_choice);
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