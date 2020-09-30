#include "SpeedFlipPlugin.h"

#include <fstream>
#include <sstream>
#include <math.h>

#define PI 3.14159265

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
	lastMsg = std::chrono::system_clock::now();
	state = 0;
	inputHistory.clear();
	std::stringstream logBuffer;
	logBuffer << "SpeedFlip Plugin Loaded!";
	cvarManager->log(logBuffer.str());
	for (int i = 0; i < 10; i++) {
		popups.push_back(new Popup({ "" }));
	}
	gameWrapper->RegisterDrawable(std::bind(&SpeedFlipPlugin::Render, this, std::placeholders::_1));
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
	// Game time accounts for time dilation etc.
	float curGameTime = gameWrapper->GetGameEventAsServer().GetSecondsElapsed();

	if (gameWrapper->GetLocalCar().GetbSuperSonic() && !isSupersonic) {
		popups[9]->text = "SS: " + std::to_string(curGameTime - startTime) + ", " + std::to_string(curGameTime - jumpTime);
		isSupersonic = true;
	}

	// 0 = reset, 1 = accel, 2 = left stick, 3 = 1st jump, 4 = top right, 5 = 2nd jump, 6 = bottom/cancel, 7 =
	if (state == 0) {
		if (!started && (ci->Throttle > 0 || ci->ActivateBoost > 0)) {
			started = true;
			prevTime = curGameTime;
			startTime = curGameTime;
			state = 1;
		}
	}
	else if (state == 1) {
		if (ci->Yaw > 0.7 || ci->Yaw < -0.7) {
			state = 2;
			popups[state]->text = std::to_string(curGameTime - prevTime);
			prevTime = curGameTime;
			renderPopup();
		}
	}
	else if (state == 2) {
		// because the ci naming is a lie. ActivateBoost is jump when I tested it
		if (ci->ActivateBoost== 1) {
			state = 3;
			popups[state]->text = std::to_string(curGameTime - prevTime);
			prevTime = curGameTime;
			jumpTime = curGameTime;
			renderPopup();
		}
	}
	else if (state == 3) {
		if ((ci->Yaw < -0.1 && ci->Yaw > -0.5) || (ci->Yaw > 0.1 && ci->Yaw < 0.5) && ci->ActivateBoost == 0) {
			state = 4;
			popups[state]->text = std::to_string(curGameTime - prevTime);
			prevTime = curGameTime;
			renderPopup();
		}
	}
	else if (state == 4) {
		if (ci->ActivateBoost == 1) {
			state = 5;
			popups[state]->text = std::to_string(curGameTime - prevTime) + " ANGLE: " + std::to_string(-atan(ci->Yaw / ci->Pitch) * 180 / PI);
			prevTime = curGameTime;
			renderPopup();
		}
	}
	else if (state == 5) {
		if (ci->Pitch > 0.9) {
			state = 6;
			popups[state]->text = std::to_string(curGameTime - prevTime);
			prevTime = curGameTime;
			renderPopup();
		}
	}

	

	if (cvarManager->getCvar(CVAR_PLUGIN_ENABLED).getBoolValue() && started) {
		inputHistory.push_back({ curGameTime, *ci });
	}
}

void SpeedFlipPlugin::OnHitBall(std::string eventName)
{
	if (!gameWrapper->IsInCustomTraining()) {
		return;
	}
	started = false;
	// save();
	inputHistory.clear();
	cvarManager->log("SUCCESSFULLY HIT THE BALL!!!");
}

void SpeedFlipPlugin::OnReset(std::string eventName)
{
	if (!gameWrapper->IsInCustomTraining()) {
		return;
	}
	started = false;
	isSupersonic = false;
	state = 0;
	// save();
	inputHistory.clear();
}

static std::string inputToString(float ts, ControllerInput ci)
{
	std::stringstream ss;
	ss << ts;
	ss << ",ActivateBoost=" << ci.ActivateBoost;
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
		for (auto input : inputHistory)
		{
			myfile << inputToString(input.timeStamp, input.ci);
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

void SpeedFlipPlugin::renderPopup() {
	// Override lastMsg, so that the render will show for the next 4s
	lastMsg = std::chrono::system_clock::now();
}

void SpeedFlipPlugin::Render(CanvasWrapper canvas)
{
	if (!gameWrapper->IsInCustomTraining() || popups.empty() || std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - lastMsg).count() > 4)
		return;

	auto screenSize = canvas.GetSize();
	for (int i = 0; i < popups.size(); i++)
	{
		auto pop = popups.at(i);
		if (pop->startLocation.X < 0)
		{
			pop->startLocation = { (int)(screenSize.X * 0.35), (int)(screenSize.Y * 0.1 + i * 0.035 * screenSize.Y) };
		}

		Vector2 drawLoc = { pop->startLocation.X, pop->startLocation.Y };
		canvas.SetPosition(drawLoc);
		canvas.SetColor(pop->color.R, pop->color.G, pop->color.B, 255);
		canvas.DrawString(pop->text, 3, 3);
	}
}