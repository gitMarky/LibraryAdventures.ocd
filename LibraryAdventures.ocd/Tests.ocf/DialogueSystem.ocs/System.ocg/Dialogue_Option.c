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

func DebugDlg()
{
	Log(">>> Debug: dlg_internal = %d, dlg_progress = %d, dlg_last_nonoption = %d, dlg_option = %v", dlg_internal, dlg_progress, dlg_last_nonoption, dlg_option);
}