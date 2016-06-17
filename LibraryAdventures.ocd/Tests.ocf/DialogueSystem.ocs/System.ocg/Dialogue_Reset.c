#include Library_Dialogue
#appendto Library_Dialogue

local test_reset_counter = 3;

func Dlg_TestDlgReset(object player)
{
	DlgText(Format("Countdown: %d", test_reset_counter));
	if (DlgEvent()) test_reset_counter--;
	if (test_reset_counter > 0)
	{
		DlgReset();
	}
}
