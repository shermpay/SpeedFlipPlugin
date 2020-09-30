#pragma once
#pragma comment( lib, "bakkesmod.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"


struct Color
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};
struct Popup
{
	std::string text = "";
	Color color = { 255, 255, 255 };
	Vector2 startLocation = { -1, -1 };
};
struct Input
{
	float timeStamp;
	ControllerInput ci;
};
class SpeedFlipPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	/*
	Whether plugin is currently active
	*/
	std::shared_ptr<bool> pluginActive = std::make_shared<bool>(false);

	std::chrono::system_clock::time_point lastMsg;

	bool started;
	bool isSupersonic;
	int state; // 0 = reset, 1 = accel, 2 = left stick, 3 = 1st jump, 4 = top right, 5 = 2nd jump, 6 = bottom/cancel, 7 = 
	float prevTime;
	float startTime;
	float jumpTime;
	std::vector<Popup*> popups;
	std::vector<Input> inputHistory;

	void save();
	void renderPopup();
	void Render(CanvasWrapper canvas);
public:
	void onLoad() override;
	void onUnload() override;

	void OnInput(CarWrapper cw, void* params);
	void OnHitBall(std::string eventName);
	void OnReset(std::string eventName);
};

static std::string inputToString(float ts, ControllerInput ci);
