protected func InitializePlayer(int plr)
{
	// Set zoom to full map size.
	SetPlayerZoomByViewRange(plr, 300, nil, PLRZOOM_Direct);
	
	// No FoW to see everything happening.
	SetFoW(false, plr);
		
	// Start!
	LaunchTest(1);
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

/*-- The actual tests --*/


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
	if (Test().user->GetMenu())
	{
		if (!Test().test1_displayed_menu)
		{
			pass("Opened menu.");
			Test().test1_displayed_menu = true;
			return false;
		}
	}
	else
	{
		if (Test().test1_displayed_menu)
		{
			pass("Closed menu.");
			return true;
		}
		
		Test().test1_counter += 1;
		
		if (Test().test1_counter >= 60)
		{
			fail("Failed to open the menu.");
			FailTest();
		}
	
		return false;
	}
}

global func Test1_OnFinished()
{
	if (Test().dialogue) Test().dialogue->RemoveObject();
}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

global func Test2_OnStart()
{
	Log("Test for DlgText(): Single call of the function");

	Test().dialogue = Test().target->SetDialogueEx("TestDlgText2");
	Test().dialogue->Interact(Test().user);

	return true;
}

global func Test2_Completed()
{
	Test().test2_max_internal = Max(Test().test2_max_internal, Test().dialogue.dlg_internal);

	if (Test().user->GetMenu())
	{
		if (!Test().test2_displayed_menu)
		{
			pass("Opened menu.");
			Test().test2_displayed_menu = true;
			return false;
		}
	}
	else
	{
		if (Test().test2_displayed_menu)
		{
			pass("Closed menu.");
			if (doTest("Test should have displayed 2 entries. Internal dialogue counter was %d, expected %d.", Test().test2_max_internal, 2))
			{
				return true;
			}
			else
			{
				FailTest();
				return false;
			}
		}
		
		Test().test2_counter += 1;
		
		if (Test().test2_counter >= 60)
		{
			fail("Failed to open the menu.");
			FailTest();
		}
	
		return false;
	}
}

global func Test2_OnFinished()
{
	if (Test().dialogue) Test().dialogue->RemoveObject();
}
