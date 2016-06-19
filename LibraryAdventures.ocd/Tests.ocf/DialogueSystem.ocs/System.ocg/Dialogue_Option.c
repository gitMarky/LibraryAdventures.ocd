#include Library_Dialogue
#appendto Library_Dialogue

func Dlg_TestDlgOption1(object player)
{
	DebugDlg();
	DlgText("Testing options.");
	DebugDlg();
	if (DlgOption("Option 1: This will restart the dialogue."))
	{
		DlgText("You chose option 1. Starting again.");
		DlgReset();
//		DlgOptionEnd();
	}
	if (DlgOption("Option 2: This will end the dialogue."))
	{
		DlgText("You chose option 2. Goodbye!");
		DlgOptionEnd();
	}
}

local test_forked_dialogues;


func Dlg_TestDlgOption2(object player)
{
	if (!test_forked_dialogues) test_forked_dialogues = [];

	DlgText("Testing forked options. Go through all the options.");
	if (DlgOption("Option 1: 2 choices on a lower level."))
	{
		DlgText("You chose option 1.");
		if (!test_forked_dialogues[0] && DlgOption("Option 1.1"))
		{
			DlgText("You chose option 1.1");
			if (DlgEvent())
			{
				test_forked_dialogues[0] = true;
			}
			DlgReset();
		}
		if (!test_forked_dialogues[1] && DlgOption("Option 1.2"))
		{
			DlgText("You chose option 1.2");
			if (DlgEvent())
			{
				test_forked_dialogues[1] = true;
			}
			DlgReset();
		}
		DlgOptionEnd();
	}
	if (DlgOption("Option 2: Choices 2 more levels deep."))
	{
		DlgText("You chose option 2.");
		if (!(test_forked_dialogues[2] && test_forked_dialogues[3]) && DlgOption("Option 2.1"))
		{
			DlgText("You chose option 2.1");
			if (DlgEvent())
			{
				test_forked_dialogues[2] = true;
			}
			if (DlgOption("Option 2.1.1"))
			{
				DlgText("You chose option 2.1.1");
				if (DlgEvent())
				{
					test_forked_dialogues[3] = true;
				}
				DlgReset();
			}
			if (DlgOption("Return"))
			{
				DlgReset();
			}
			DlgOptionEnd();
		}
		
		if (test_forked_dialogues[0]
		 && test_forked_dialogues[1]
		 && test_forked_dialogues[2]
		 && test_forked_dialogues[3])
		{
			DlgText("Congratulations, you clicked through all options.");
		}
	}
}


func DebugDlg()
{
	Log(">>> Debug: dlg_internal = %d, dlg_progress = %d, dlg_last_nonoption = %d, dlg_option = %v", dlg_internal, dlg_progress, dlg_last_nonoption, dlg_option);
}