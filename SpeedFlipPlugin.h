#pragma once
#pragma comment( lib, "bakkesmod.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"

class SpeedFlipPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	/*
	Whether plugin is currently active
	*/
	std::shared_ptr<bool> pluginActive = std::make_shared<bool>(false);

	bool started;
	std::vector<ControllerInput> inputHistory;

	void save();
public:
	void onLoad() override;
	void onUnload() override;

	void OnInput(CarWrapper cw, void* params);
	void OnHitBall(std::string eventName);
	void OnReset(std::string eventName);
};
