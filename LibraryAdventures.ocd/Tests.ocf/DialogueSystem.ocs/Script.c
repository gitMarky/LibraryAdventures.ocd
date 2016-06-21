protected func InitializePlayer(int plr)
{
	// Set zoom to full map size.
	SetPlayerZoomByViewRange(plr, 300, nil, PLRZOOM_Direct);
	
	// No FoW to see everything happening.
	SetFoW(false, plr);
		
	// Start!
	LaunchTest(8);
	return;
}

/*-- Test Control --*/

static const FX_TEST_CONTROL = "IntTestControl";

// Aborts the current test and launches the specified test instead.
global func LaunchTest(int nr)
{
	// Get the control effect.
	var effect = GetEffect(FX_TEST_CONTROL, nil);
	if (!effect)
	{
		// Create a new control effect and launch the test.
		effect = AddEffect(FX_TEST_CONTROL, nil, 100, 2);
		effect.test_number = nr;
		effect.launched = false;
		effect.player = GetPlayerByIndex(0, C4PT_User);
		effect.user = GetHiRank(effect.player);
		effect.user->SetPosition(LandscapeWidth() / 2, effect.user->GetY());		
		effect.target = CreateObject(Clonk, 20 + LandscapeWidth() / 2, effect.user->GetY(), NO_OWNER);
		effect.target->SetColor(RGB(255, 0, 255));
		return;
	}
	// Finish the currently running test.
	Call(Format("~Test%d_OnFinished", effect.test_number));
	// Start the requested test by just setting the test number and setting 
	// effect.launched to false, effect will handle the rest.
	effect.test_number = nr;
	effect.launched = false;
	return;
}

// Calling this function skips the current test, does not work if last test has been ran already.
global func SkipTest()
{
	Test().tests_skipped++;
	NextTest();
}

global func FailTest()
{
	Test().tests_failed++;
	NextTest();
}

global func NextTest()
{
	// Get the control effect.
	var effect = Test();
	if (!effect)
		return;
	// Finish the previous test.
	Call(Format("~Test%d_OnFinished", effect.test_number));
	// Start the next test by just increasing the test number and setting 
	// effect.launched to false, effect will handle the rest.
	effect.test_number++;
	effect.launched = false;
	return;
}

global func Test()
{
	// Get the control effect.
	var effect = GetEffect(FX_TEST_CONTROL, nil);
	return effect;
}

global func SetTestDefaults()
{
	Test().opened_menu = false;
	Test().opened_menu_counter = 0;
	Test().closed_menu = false;
	Test().closed_menu_counter = 0;
	Test().max_internal = -1;
	if (Test().dialogue) Test().dialogue->RemoveObject();
}

/*-- Test Effect --*/

global func FxIntTestControlStart(object target, proplist effect, int temporary)
{
	if (temporary)
		return FX_OK;
	// Set default interval.
	effect.Interval = 2;
	return FX_OK;
}

global func FxIntTestControlTimer(object target, proplist effect)
{
	// Launch new test if needed.
	if (!effect.launched)
	{
		// Log test start.
		Log("=====================================");
		Log("Test %d started:", effect.test_number);
		SetTestDefaults();
		// Start the test if available, otherwise finish test sequence.
		if (!Call(Format("~Test%d_OnStart", effect.test_number)))
		{
			Log("Test %d not available, the previous test was the last test.", effect.test_number);
			Log("=====================================");
			
			if (effect.tests_skipped <= 0 && effect.tests_failed <= 0)
			{
				Log("All tests have been successfully completed!");
			}
			else
			{
				Log("%d tests were skipped.", effect.tests_skipped);
				Log("%d tests failed.", effect.tests_failed);
			}
			return FX_Execute_Kill;
		}
		effect.launched = true;
	}		
	// Check whether the current test has been finished.
	if (Call(Format("Test%d_Completed", effect.test_number)))
	{
		effect.launched = false;
		// Call the test on finished function.
		Call(Format("~Test%d_OnFinished", effect.test_number));
		// Log result and increase test number.
		Log("Test %d successfully completed.", effect.test_number);
		effect.test_number++;
	}
	return FX_OK;
}

global func pass(string message)
{
	Log(Format("[Passed] %s", message));
}

global func fail(string message)
{
	Log(Format("[Failed] %s", message));
}

global func doTest(string message, actual, expected)
{
	var passed = actual == expected;
	
	var log_message = Format(message, actual, expected);
	
	if (passed)
		pass(log_message);
	else
		fail(log_message);
		
	return passed;
}

global func askUser(string message)
{
	return false;
}

/*-- The actual tests --*/


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func MenuWasOpened()
{
	if (Test().opened_menu || Test().user->GetMenu())
	{
		if (!Test().opened_menu)
		{
			pass("Opened menu.");
			Test().opened_menu = true;
		}
		
		return true;
	}


	Test().opened_menu_counter += 1;
	
	if (Test().opened_menu_counter >= 60)
	{
		fail("Failed to open the menu.");
		FailTest();
	}
	
	return false;
}


global func MenuWasClosed()
{
	if (Test().opened_menu)
	{
		if (!Test().user->GetMenu()) Test().closed_menu_counter += 1;

		if (Test().closed_menu_counter >= 10)
		{
			pass("Closed menu.");
			return true;
		}
	}

	return false;
}

global func CountDialogueProgress()
{
	Test().max_internal = Max(Test().max_internal, Test().dialogue.dlg_internal[Test().dialogue.dlg_layer]);
}

global func GetDialogueProgress()
{
	return Test().max_internal;
}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func Test1_OnStart()
{
	Log("Test for DlgText(): Single call of the function");

	Test().dialogue = Test().target->SetDialogueEx("TestDlgText1");
	Test().dialogue->Interact(Test().user);

	return true;
}

global func Test1_Completed()
{
	CountDialogueProgress();
	
	if (MenuWasOpened() && MenuWasClosed())
	{
		if (doTest("Test should have displayed 1 entry. Internal dialogue counter was %d, expected %d.", GetDialogueProgress(), 1))
		{
			return true;
		}
		else
		{
			FailTest();
			return false;
		}
	}
	else
	{
		return false;
	}
}

global func Test1_OnFinished(){}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func Test2_OnStart()
{
	Log("Test for DlgText(): Two calls of the function in a row");

	Test().dialogue = Test().target->SetDialogueEx("TestDlgText2");
	Test().dialogue->Interact(Test().user);

	return true;
}

global func Test2_Completed()
{
	CountDialogueProgress();
	
	if (MenuWasOpened() && MenuWasClosed())
	{
		if (doTest("Test should have displayed 2 entries. Internal dialogue counter was %d, expected %d.", GetDialogueProgress(), 2))
		{
			return true;
		}
		else
		{
			FailTest();
			return false;
		}
	}
	else
	{
		return false;
	}
}

global func Test2_OnFinished(){}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func Test3_OnStart()
{
	Log("Test for DlgText(): Changing portraits");

	Test().dialogue = Test().target->SetDialogueEx("TestDlgText3");
	Test().dialogue->Interact(Test().user);

	return true;
}

global func Test3_Completed()
{
	CountDialogueProgress();
	
	if (MenuWasOpened() && MenuWasClosed())
	{
		if (doTest("Test should have displayed 2 entries. Internal dialogue counter was %d, expected %d.", GetDialogueProgress(), 2)
		 && askUser("Did the portrait change in the second message?"))
		{
			return true;
		}
		else
		{
			FailTest();
			return false;
		}
	}
	else
	{
		return false;
	}
}

global func Test3_OnFinished(){}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func Test4_OnStart()
{
	Log("Test for DlgEvent(): Just the event");

	Test().dialogue = Test().target->SetDialogueEx("TestDlgEvent1");
	Test().dialogue->Interact(Test().user);

	return true;
}

global func Test4_Completed()
{
	var rock = FindObject(Find_ID(Rock));
	if (rock)
	{
		pass("A rock was created");
		rock->RemoveObject();
		return true;
	}
	else
	{
		return false;
	}
}

global func Test4_OnFinished(){}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func Test5_OnStart()
{
	Log("Test for DlgEvent(): Multiple calls, with text in between.");

	Test().dialogue = Test().target->SetDialogueEx("TestDlgEvent2");
	Test().dialogue->Interact(Test().user);

	return true;
}

global func Test5_Completed()
{
	if (MenuWasOpened())
	{
		if (!Test().test5_rock)
		{
			if (FindObject(Find_ID(Rock)))
			{
				Test().test5_rock = true;
				pass("Rock was created");
			}
		}
		else
		{
			if (MenuWasClosed())
			{
				if (doTest("Rock should be removed. Got %v, expected %v.", FindObject(Find_ID(Rock)), nil))
					return true;
				else
				{
					FailTest();
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	
	}
	else
	{
		return false;
	}
}

global func Test5_OnFinished(){}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func Test6_OnStart()
{
	Log("Test for DlgReset(): Countdown");

	Test().dialogue = Test().target->SetDialogueEx("TestDlgReset");
	Test().dialogue->Interact(Test().user);

	return true;
}

global func Test6_Completed()
{
	if (MenuWasOpened() && MenuWasClosed())
	{
		if (doTest("Test countdown was at %d, expected %d.", Test().dialogue.test_reset_counter, 0))
		{
			return true;
		}
		else
		{
			FailTest();
			return false;
		}
	}
	else
	{
		return false;
	}
}

global func Test6_OnFinished(){}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func Test7_OnStart()
{
	Log("Test for DlgOption(): Single dialogue with 2 options");

	Test().dialogue = Test().target->SetDialogueEx("TestDlgOption1");
	Test().dialogue->Interact(Test().user);

	return true;
}

global func Test7_Completed()
{
	if (MenuWasOpened() && MenuWasClosed())
	{
		return true;
	}
	else
	{
		return false;
	}
}

global func Test7_OnFinished(){}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func Test9_OnStart()
{
	Log("Test for DlgOption(): Forked/multi-level options. The player has to visit all options");

	Test().dialogue = Test().target->SetDialogueEx("TestDlgOption2");
	Test().dialogue->Interact(Test().user);

	return true;
}

global func Test9_Completed()
{
	if (MenuWasOpened() && MenuWasClosed())
	{
		var results = Test().dialogue.test_forked_dialogues;
		return results[0] && results[1] && results[2];
	}
	else
	{
		return false;
	}
}

global func Test9_OnFinished(){}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func Test8_OnStart()
{
	Log("Test for DlgOption(): Options that are always present?");

	Test().dialogue = Test().target->SetDialogueEx("TestDlgOption3");
	Test().dialogue->Interact(Test().user);

	return true;
}

global func Test8_Completed()
{
	if (MenuWasOpened() && MenuWasClosed())
	{
		return true;
	}
	else
	{
		return false;
	}
}

global func Test8_OnFinished(){}

