
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
	++dlg_internal[dlg_internal_layer];
	
	var debug_log = Format("L(%d)/I(%d): %s", dlg_internal_layer, dlg_internal[dlg_layer], text); 
	
	if (IsBeingProcessed(debug_log))
	{
		BroadcastDialogue({ Prototype = DlgMessage(), text = text, sender = speaker ?? dlg_target, receiver = dlg_player});
	}
}


/**
 Adds a dialogue option.
 @par text this text will be displayed.
 @example Did you ever want to be in a game show?
  {@code
     Dlg_Options(object player)
     {
         DlgText("Choose one:");
         if (DlgOption("Door A"))
         {
         	DlgText("Your choice was door A!");
         	if (DlgEvent()) ConfirmChoice("A");
         	DlgOptionEnd();
         }
         if (DlgOption("Door B"))
         {
         	DlgText("Your choice was door B!");
         	if (DlgEvent()) ConfirmChoice("B");
         	DlgOptionEnd();
         }
         if (DlgOption("Door D"))
         {
         	DlgText("Your choice was door C!");
         	if (DlgEvent()) ConfirmChoice("C");
         	DlgOptionEnd();
         }
         // if the dialoge options do not branch any further, then the dialogue will continue here
         DlgText("This door does not contain the price:");
         if (DlgEvent()) RevealDoorWithGoat();
         DlgText("Would you like to choose the other door?");
         if (DlgOption("Yes"))
         {
         	if (DlgEvent()) SwitchChoice();
         	DlgOptionEnd();
         }
         if (DlgOption("No"))
         {
         	if (DlgEvent()) StayWithChoice();
         	DlgOptionEnd();
         }
     }
 }
 @related {@link Library_Dialogue#DlgOptionEnd}
 */
public func DlgOption(string text)
{
	++dlg_internal_option[dlg_internal_layer];

	var display_option = IsLayerCorrect() && IsAtThisPosition();
	var was_chosen = dlg_option[dlg_internal_layer] == dlg_internal_option[dlg_internal_layer];
	
	var debug_log = Format("L(%d)/I(%d): %s, display = %v, chosen = %v", dlg_internal_layer, dlg_internal[dlg_layer], text, display_option, was_chosen); 
		
	if (display_option)
	{
		BroadcastOption({ Prototype = DlgMessage(), text = text, receiver = dlg_player, option_choice = dlg_internal_option[dlg_internal_layer]}); // the progress has to be set here, if the option is chosen!
	}
	
	if (was_chosen)
	{
		++dlg_internal_layer;
	}
	LogDlgSelection(was_chosen, debug_log, ">");
	
	return was_chosen;
}


/**
 This tells the dialogue engine that the contents of an option are ended.
 Unintended effects may occurr if you do not put this at the end of an
 {@code if(DlgOption(...))}-block.
 @related {@link Library_Dialogue#DlgOption} 
 */
public func DlgOptionEnd()
{
	++dlg_internal[dlg_internal_layer];
	var debug_log = Format("L(%d)/I(%d): DlgOptionEnd()", dlg_internal_layer, dlg_internal[dlg_layer]); 

	if (IsBeingProcessed(debug_log))
	{
		ResetDialogue(dlg_layer);
		ProgressDialogueDelayed();
	}
	
	--dlg_internal_layer;
}


/**
 Resets the dialogue, that is, the dialogue will start at the beginning again.
 @example This will create an endless loop:
 {@code
     Dlg_Loop(object player)
     {
         DlgText("Are you tired of this message yet?");
         if (DlgReset()) // starts the dialogue again
         {
         	Log("The player did not quit until loop #%d", loop_counter++); // we will log how patient the player was :p
         }
     }
 }
 */
public func DlgReset()
{
	++dlg_internal[dlg_internal_layer];
	var debug_log = Format("L(%d)/I(%d): DlgReset()", dlg_internal_layer, dlg_internal[dlg_layer]); 

	var execute_event = false;
	if (IsBeingProcessed(debug_log))
	{
		execute_event = true;
		ResetDialogue();
		ProgressDialogueDelayed();
	}
	
	return execute_event;
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
	var debug_log = Format("L(%d)/I(%d): DlgEvent()", dlg_layer, dlg_internal[dlg_layer]); 

	var execute_event = false;
	if (IsBeingProcessed(debug_log))
	{
		execute_event = true;
		ProgressDialogueDelayed();
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
	if (layer == nil || layer == 0)
	{
		dlg_layer = 0;
		dlg_progress = [];
		dlg_internal = [];
		dlg_internal_option = [];
		dlg_option = [];
	}
	else
	{
		dlg_progress[layer] = 0;

		// advance progress in the previous layer
		dlg_layer -= 1;
		dlg_internal_option = [];
		dlg_option[dlg_layer] = nil;
		dlg_progress[layer] = nil;
	}
}


private func InDialogue(object player)
{
	return player->GetMenu() == Dialogue
	    || player->GetMenu() == DialogueEx;
}


private func IsBeingProcessed(string debug_message)
{
	var result = IsLayerCorrect() && IsAtThisPosition();
	LogDlgSelection(result, debug_message);	
	return result;
}


private func IsLayerCorrect()
{
	return dlg_internal_layer == dlg_layer;
}

private func IsAtThisPosition()
{
	return dlg_internal[dlg_layer] == dlg_progress[dlg_layer];
}

private func LogDlgSelection(bool result, string debug_message, string selected_string)
{
	selected_string = selected_string ?? "*";

	if (debug_message)
	{
		if (result)
			Log("%s %s", selected_string, debug_message);
		else
			Log("  %s", debug_message);
	}
	return result;
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
	
	if (dlg_progress[dlg_layer] > dlg_internal[dlg_layer])
	{
		ResetDialogue();
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
	DoBroadcast("OnBroadcastDialogue", message);
}


public func BroadcastOption(proplist message)
{
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