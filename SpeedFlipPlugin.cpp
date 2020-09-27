#include "SpeedFlipPlugin.h"

#include <fstream>
#include <sstream>

BAKKESMOD_PLUGIN(SpeedFlipPlugin, "SpeedFlip Plugin", "0.1", PLUGINTYPE_FREEPLAY)

#define INPUT_HISTORY_LENGTH 1000

#define CVAR_PLUGIN_ENABLED "speedflip_plugin_enabled"

#define SAVEFILE "bakkesmod/data/speedflipplugin.data"

void SpeedFlipPlugin::onLoad()
{
	cvarManager->registerCvar(CVAR_PLUGIN_ENABLED, "1", "Enable Speedflip Plugin", true, true, 0.f, true, 1.f);

	gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.SetVehicleInput", std::bind(&SpeedFlipPlugin::OnInput, this, std::placeholders::_1, std::placeholders::_2));
	gameWrapper->HookEventPost("Function TAGame.Car_TA.EventHitBall", std::bind(&SpeedFlipPlugin::OnHitBall, this, std::placeholders::_1));
	gameWrapper->HookEventPost("Function Engine.Controller.Restart", std::bind(&SpeedFlipPlugin::OnReset, this, std::placeholders::_1));

	started = false;
	inputHistory.clear();
	std::stringstream logBuffer;
	logBuffer << "SpeedFlip Plugin Loaded!";
	cvarManager->log(logBuffer.str());
}

void SpeedFlipPlugin::onUnload()
{
	cvarManager->log("SpeedFlip Plugin Unloaded!");
}

void SpeedFlipPlugin::OnInput(CarWrapper cw, void* params)
{
	if (!gameWrapper->IsInCustomTraining()) {
		return;
	}

	ControllerInput* ci = (ControllerInput*)params;

	if (ci->Throttle > 0 || ci->ActivateBoost > 0) {
		started = true;
	}

	if (cvarManager->getCvar(CVAR_PLUGIN_ENABLED).getBoolValue() && started) {
		inputHistory.push_back(*ci);
	}
}

void SpeedFlipPlugin::OnHitBall(std::string eventName)
{
	if (!gameWrapper->IsInCustomTraining()) {
		return;
	}
	started = false;
	save();
	inputHistory.clear();
	cvarManager->log("SUCCESSFULLY HIT THE BALL!!!");
}

void SpeedFlipPlugin::OnReset(std::string eventName)
{
	if (!gameWrapper->IsInCustomTraining()) {
		return;
	}
	started = false;
	save();
	inputHistory.clear();
}

static std::string inputToString(ControllerInput ci)
{
	std::stringstream ss;
	ss << "ActivateBoost=" << ci.ActivateBoost;
	ss << ",DodgeForward=" << ci.DodgeForward;
	ss << ",DodgeStrafe=" << ci.DodgeStrafe;
	ss << ",Handbrake=" << ci.Handbrake;
	ss << ",HoldingBoost=" << ci.HoldingBoost;
	ss << ",Jump=" << ci.Jump;
	ss << ",Jumped=" << ci.Jumped;
	ss << ",Pitch=" << ci.Pitch;
	ss << ",Roll=" << ci.Roll;
	ss << ",Steer=" << ci.Steer;
	ss << ",Throttle=" << ci.Throttle;
	ss << ",Yaw=" << ci.Yaw;
	return ss.str();
}

void SpeedFlipPlugin::save()
{
	std::ofstream myfile;
	myfile.open(SAVEFILE, std::ofstream::app);
	if (myfile.is_open())
	{
		myfile << "=START=\n";
		for (auto ci : inputHistory)
		{
			myfile << inputToString(ci);
			myfile << "\n";
		}
		myfile << "=END=\n";
	}
	else
	{
		cvarManager->log("Can't write savefile.");
	}
	myfile.close();
}
