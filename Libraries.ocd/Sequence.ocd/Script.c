/**
	A sequence that runs in the background.
	
	@author Marky
	@credits Sven2 for the sequence object
	@version 0.1.0
*/

local seq_name;
local seq_progress;
local started;

/* Simple interface */

public func SetProgress(progress)
{
	if (progress == nil)
	{
		FatalError("Sequence progress cannot be nil!");
	}
	if (GetType(progress) != C4V_Int
	 && GetType(progress) != C4V_String)
	{
		FatalError(Format("Sequence progress may only be int or string, your are assigning %v: %v", GetType(progress), progress));
	}
	if (GetType(progress) == C4V_Int && progress < 0)
	{
		FatalError(Format("Sequence progress may not be negative! You specified: %d", progress));
	}

	seq_progress = progress;
}

public func GetProgress()
{
	return seq_progress;
}

public func IsBackgroundSequence(string name)
{
	if (name == nil)
	{
		return true;
	}
	else
	{
		return seq_name == name;
	}
}

public func IsActive()
{
	return started;
}


/* Start and stop */

public func Start(string name, progress, ...)
{
	if (started) Stop();

	// Force global coordinates for the script execution.
	SetPosition(0, 0);

	// Store sequence name and progress.
	this.seq_name = name;
	this.seq_progress = progress;

	SequenceCall("Init");	// Call init function of this scene - difference to start function is that it is called before any player joins.
	JoinPlayers();
	started = true;
	SequenceCall("Start"); 	// Call start function of this scene.
	return true;
}

func SequenceCall(string name, ...)
{
	var fn_name = Format("%s_%s", seq_name, name);
	if (!Call(fn_name, ...))
		GameCall(fn_name, this, ...);
}

public func Remove()
{
	Stop(true);
}

public func Stop(bool remove)
{
	if (started)
	{
		for (var i = 0; i<GetPlayerCount(C4PT_User); ++i)
		{
			var plr = GetPlayerByIndex(i, C4PT_User);
			// Per-player sequence callback.
			RemovePlayer(plr);
		}
		started = false;
		// Call stop function of this scene.
		SequenceCall("Stop");
	}
	if (!remove) RemoveObject();
	return true;
}

protected func Destruction()
{
	Stop(true);
	return;
}

/*-- Handling players --*/

func JoinPlayers()
{
	// Join all players: disable player controls and call join player of this scene.
	for (var i = 0; i < GetPlayerCount(C4PT_User); ++i)
	{
		var plr = GetPlayerByIndex(i, C4PT_User);
		JoinPlayer(plr);
	}
}

protected func InitializePlayer(int plr)
{
	JoinPlayer(plr);
	return true;
}

public func RemovePlayer(int plr)
{
	// Called by sequence if it ends and by engine if player leaves.
	SequenceCall("RemovePlayer", plr);
	return true;
}

public func JoinPlayer(int plr)
{
	// Per-player sequence callback.
	SequenceCall("JoinPlayer", plr);
	return true;
}

/*-- Sequence callbacks --*/

public func ScheduleNext(int delay, next_idx)
{
	return ScheduleCall(this, this.CallNext, delay, 0, next_idx);
}

public func ScheduleSame(int delay) { return ScheduleNext(delay, seq_progress); }

public func CallNext(next_idx)
{
	// Start conversation context.
	// Update dialogue progress first.
	if (GetType(next_idx)) 
		seq_progress = next_idx; 
	else 
		++seq_progress;
	// Then call relevant functions.
	SequenceCall(seq_progress);
	return true;
}

/*-- Saving --*/

// No scenario saving.
public func SaveScenarioObject(props) { return false; }


/*-- Global helper functions --*/

// Starts the specified sequence at indicated progress.
global func StartBackgroundSequence(string name, progress, ...)
{
	var seq = CreateObject(BackgroundSequence, 0, 0, NO_OWNER);
	seq->Start(name, progress, ...);
	return seq;
}

// Stops the currently active sequence.
global func StopBackgroundSequence(string name)
{
	var seq = FindObject(Find_ID(BackgroundSequence), Find_Func("IsBackgroundSequence", name));
	if (!seq) 
		return false;
	if (seq->IsActive())
	{
		return seq->Stop();
	}
	else
	{
		return true;
	}
}

// Returns background sequences with certain criteria.
global func GetBackgroundSequences(...)
{
	var seq = FindObjects(Find_ID(BackgroundSequence), ...);
	return seq;
}
