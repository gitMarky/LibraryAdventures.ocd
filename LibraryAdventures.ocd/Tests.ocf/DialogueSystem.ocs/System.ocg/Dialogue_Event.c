#include Library_Dialogue
#appendto Library_Dialogue

func Dlg_TestDlgEvent1(object player)
{
	if (DlgEvent())
	{
		CreateObject(Rock);
	}
}

func Dlg_TestDlgEvent2(object player)
{
	DlgText("You will have a rock in your inventory after this message.");
	if (DlgEvent())
	{
		player->CreateContents(Rock);
	}
	DlgText("The rock will be gone after this message.");
	if (DlgEvent())
	{
		player->FindContents(Rock)->RemoveObject();
	}
}
