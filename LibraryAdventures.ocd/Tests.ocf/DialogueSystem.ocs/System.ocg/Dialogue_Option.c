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
	var completed = test_forked_dialogues[0]
	 			 && test_forked_dialogues[1]
				 && test_forked_dialogues[2];
	
	if (completed)
	{
		DlgText("Congratulations, you clicked through all options.");
	}
	else
	{
		DlgText("Testing forked options. Go through all the options.");
	
		if (!(test_forked_dialogues[0] && test_forked_dialogues[1]) && DlgOption("Option 1: 2 choices on a lower level."))
		{
			DlgText("You chose option 1.");
			if (!test_forked_dialogues[0] && DlgOption("Option 1.1"))
			{
				Log("* Handling choice 1.1");
				DlgText("You chose option 1.1");
				if (DlgReset())
				{
					test_forked_dialogues[0] = true;
				}
				DlgOptionEnd();
			}
			if (!test_forked_dialogues[1] && DlgOption("Option 1.2"))
			{
				Log("* Handling choice 1.2");
				DlgText("You chose option 1.2");
				if (DlgReset())
				{
					test_forked_dialogues[1] = true;
				}
				DlgOptionEnd();
			}
			DlgOptionEnd();
		}
		
		if (!test_forked_dialogues[2] && DlgOption("Option 2: Choices 2 more levels deep."))
		{
			Log("* Handling choice 2");
			DlgText("You chose option 2.");
			if (DlgOption("Option 2.1"))
			{
				Log("* Handling choice 2.1");
				DlgText("You chose option 2.1");
				if (DlgOption("Option 2.1.1"))
				{
					Log("* Handling choice 2.1.1");
					DlgText("You chose option 2.1.1");
					if (DlgReset())
					{
						test_forked_dialogues[2] = true;
					}
					DlgOptionEnd();
				}
				if (DlgOption("Return"))
				{
					DlgReset();
					DlgOptionEnd();
				}
				DlgOptionEnd();
			}
			DlgOptionEnd();
		}
	}	
}


func Dlg_TestDlgOption3()
{
	DlgText("This is an introduction.");
	if (DlgOption("Option 1"))
	{
		DlgText("Continuing from option 1");
		DlgOptionEnd();
	}
	if (DlgOption("Option 2"))
	{
		DlgText("Continuing from option 2");
		DlgOptionEnd();
	}
	if (DlgOption("Option 3"))
	{
		DlgText("Continuing from option 3");
		DlgOptionEnd();
	}
	DlgText("The end.");
	//if (DlgOption("Restart (this option will always be present)"))
	//{
	//	DlgReset();
	//}
}


func DebugDlg()
{
	Log(">>> Debug: dlg_internal = %d, dlg_progress = %d, dlg_option = %v", dlg_internal, dlg_progress, dlg_option);
}