
#include "DevMode.h"
#include "Engine.h"
#include "Console.h"
#include "Game/Events.h"
#include "Misc/CommandLine.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

bool gDevMode = false;

#define LOG_MAX_ONSCREEN 6
#define LOG_DISPLAY_TIME 5.f
#define LOG_DISPLAY_FADE_TIME 1.f

static int logBufferIndex = 0; // rolling buffer index
static TPair<double, FConsoleMsg> logBuffer[LOG_MAX_ONSCREEN] = {};

class CDevMode
{
public:
	CDevMode()
	{
		Events::OnEngineInit.Bind(this, &CDevMode::OnInit);
	}

	void OnInit()
	{
		if (FCommandLine::HasParam("-dev"))
		{
			gDevMode = true;
			Events::OnUpdate.Bind(this, &CDevMode::Update);
			CConsole::GetLogEvent().Bind(this, &CDevMode::OnLog);
		}
	}

	void OnLog(const FConsoleMsg& msg)
	{
		double time = 0;
		if (gEngine)
			time = gEngine->GetRuntime();

		logBuffer[logBufferIndex] = { time, msg };
		logBufferIndex = (logBufferIndex + 1) % LOG_MAX_ONSCREEN;
	}

	void Update()
	{
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::Begin("##consoleLog", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
		for (int i = 0; i < LOG_MAX_ONSCREEN; i++)
		{
			auto& log = logBuffer[i];
			if (log.Key < 0)
				continue;
			if (gEngine->GetRuntime() - log.Key > LOG_DISPLAY_TIME)
			{
				log.Key = -1;
				continue;
			}
			//ImGui::RenderText(ImVec2(10, 10 + (i * 25)), log.Value.msg.c_str());
			// Fade out the log as it approaches the end of its display time.
			float alpha = 1.f;
			if (gEngine)
			{
				double time = gEngine->GetRuntime();
				if (time - log.Key > LOG_DISPLAY_TIME - LOG_DISPLAY_FADE_TIME)
				{
					alpha = 1.f - float((time - log.Key - (LOG_DISPLAY_TIME - LOG_DISPLAY_FADE_TIME)) / LOG_DISPLAY_FADE_TIME);
				}
			}
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ImGui::Text(log.Value.msg.c_str());
			ImGui::PopStyleVar();
		}
		ImGui::End();
	}
} static _devModeInit;
