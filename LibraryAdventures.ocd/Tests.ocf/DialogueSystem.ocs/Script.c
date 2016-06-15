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

// Aborts the current test and launches the specified test instead.
global func LaunchTest(int nr)
{
	// Get the control effect.
	var effect = GetEffect("IntTestControl", nil);
	if (!effect)
	{
		// Create a new control effect and launch the test.
		effect = AddEffect("IntTestControl", nil, 100, 2);
		effect.testnr = nr;
		effect.launched = false;
		effect.plr = GetPlayerByIndex(0, C4PT_User);
		effect.user = GetHiRank(effect.plr);
		effect.user->SetPosition(LandscapeWidth() / 2, effect.user->GetY());		
		effect.target = CreateObject(Clonk, 20 + LandscapeWidth() / 2, effect.user->GetY(), NO_OWNER);
		effect.target->SetColor(RGB(255, 0, 255));
		return;
	}
	// Finish the currently running test.
	Call(Format("~Test%d_OnFinished", effect.testnr));
	// Start the requested test by just setting the test number and setting 
	// effect.launched to false, effect will handle the rest.
	effect.testnr = nr;
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
	var effect = GetEffect("IntTestControl", nil);
	if (!effect)
		return;
	// Finish the previous test.
	Call(Format("~Test%d_OnFinished", effect.testnr));
	// Start the next test by just increasing the test number and setting 
	// effect.launched to false, effect will handle the rest.
	effect.testnr++;
	effect.launched = false;
	return;
}

global func Test()
{
	// Get the control effect.
	var effect = GetEffect("IntTestControl", nil);
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
		Log("Test %d started:", effect.testnr);
		// Start the test if available, otherwise finish test sequence.
		if (!Call(Format("~Test%d_OnStart", effect.testnr), effect.plr))
		{
			Log("Test %d not available, the previous test was the last test.", effect.testnr);
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
			return -1;
		}
		effect.launched = true;
	}		
	// Check whether the current test has been finished.
	if (Call(Format("Test%d_Completed", effect.testnr), effect.plr))
	{
		effect.launched = false;
		//RemoveTest();
		// Call the test on finished function.
		Call(Format("~Test%d_OnFinished", effect.testnr), effect.plr);
		// Log result and increase test number.
		Log("Test %d successfully completed.", effect.testnr);
		effect.testnr++;
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

/*-- The actual tests --*/

// Simple test for one steady source and one steady consumer.
global func Test1_OnStart(int plr)
{
	Log("Test for DlgText(): Single call of the function");

	var dialogue = Test().target->SetDialogueEx("TestDlgText1");
	dialogue->Interact(Test().user);

	return true;
}

global func Test1_Completed(int plr)
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

global func Test1_OnFinished(int plr)
{
}
