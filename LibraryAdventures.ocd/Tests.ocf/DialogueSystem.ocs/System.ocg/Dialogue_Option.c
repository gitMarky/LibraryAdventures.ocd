#include Library_Dialogue
#appendto Library_Dialogue

func Dlg_TestDlgOption1(object player)
{
	DlgText("Testing options.");
	if (DlgOption("Option 1: This will restart the dialogue."))
	{
		DlgText("You chose option 1. Starting again.");
		DlgReset();
	}
	if (DlgOption("Option 2: This will end the dialogue."))
	{
		DlgText("You chose option 2. Goodbye!");
	}
}
