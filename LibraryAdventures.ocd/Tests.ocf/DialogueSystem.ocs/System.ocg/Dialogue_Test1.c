#include Library_Dialogue
#appendto Library_Dialogue

func Dlg_TestDlgText1(object player)
{
	DlgText("Only one text option.");
}

func Dlg_TestDlgText2(object player)
{
	DlgText("Output 1.");
	DlgText("Output 2.");
}

func Dlg_TextDlgText3(object player)
{
	DlgText("Output with portrait of the user.", player);
	DlgText("Output with portrait of the target.", dlg_target);
}
