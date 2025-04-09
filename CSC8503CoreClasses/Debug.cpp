#include "Debug.h"
#include <windows.h>
#include "Window.h" 
#include <psapi.h>
using namespace NCL;

std::vector<Debug::DebugStringEntry>	Debug::stringEntries;
std::vector<Debug::DebugLogEntry>	Debug::debugEntries;
std::vector<Debug::DebugLineEntry>		Debug::lineEntries;
std::vector<Debug::DebugTexEntry>		Debug::texEntries;
std::vector<Debug::DebugCollisionInfo> Debug::collisionLogs;


SimpleFont* Debug::debugFont = nullptr;

const Vector4 Debug::RED = Vector4(1, 0, 0, 1);
const Vector4 Debug::GREEN = Vector4(0, 1, 0, 1);
const Vector4 Debug::BLUE = Vector4(0, 0, 1, 1);

const Vector4 Debug::BLACK = Vector4(0, 0, 0, 1);
const Vector4 Debug::WHITE = Vector4(1, 1, 1, 1);

const Vector4 Debug::YELLOW = Vector4(1, 1, 0, 1);
const Vector4 Debug::MAGENTA = Vector4(1, 0, 1, 1);
const Vector4 Debug::CYAN = Vector4(0, 1, 1, 1);

void Debug::Print(const std::string& text, const Vector2& pos, const Vector4& colour) {
	DebugStringEntry newEntry;

	newEntry.data = text;
	newEntry.position = pos;
	newEntry.colour = colour;

	stringEntries.emplace_back(newEntry);
}

void Debug::DrawLine(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour, float time) {
	DebugLineEntry newEntry;

	newEntry.start = startpoint;
	newEntry.end = endpoint;
	newEntry.colourA = colour;
	newEntry.colourB = colour;
	newEntry.time = time;

	lineEntries.emplace_back(newEntry);
}

void Debug::DrawTex(const Texture& t, const Vector2& pos, const Vector2& scale, const Vector4& colour) {
	DebugTexEntry newEntry;

	newEntry.t = &t;
	newEntry.position = pos;
	newEntry.scale = scale;
	newEntry.colour = colour;

	texEntries.push_back(newEntry);
}

void Debug::DrawAxisLines(const Matrix4& modelMatrix, float scaleBoost, float time) {
	Matrix4 local = modelMatrix;
	local.SetColumn(3, Vector4(0, 0, 0, 1));

	Vector3 fwd = local * Vector4(0, 0, -1, 1.0f);
	Vector3 up = local * Vector4(0, 1, 0, 1.0f);
	Vector3 right = local * Vector4(1, 0, 0, 1.0f);

	Vector3 worldPos = modelMatrix.GetColumn(3);

	DrawLine(worldPos, worldPos + (right * scaleBoost), Debug::RED, time);
	DrawLine(worldPos, worldPos + (up * scaleBoost), Debug::GREEN, time);
	DrawLine(worldPos, worldPos + (fwd * scaleBoost), Debug::BLUE, time);
}

void Debug::UpdateRenderables(float dt) {
	int trim = 0;
	float currentTime = GetTime();  // Get the current time

	// Remove expired string entries
	for (int i = 0; i < debugEntries.size(); ) {
		DebugLogEntry* entry = &debugEntries[i];
		if (currentTime - entry->timestamp > 10.0f) {  // 10 seconds have passed
			trim++;
			debugEntries[i] = debugEntries[debugEntries.size() - trim];
		}
		else {
			++i;
		}
		if (i + trim >= debugEntries.size()) {
			break;
		}
	}
	debugEntries.resize(debugEntries.size() - trim);

	trim = 0;
	for (int i = 0; i < lineEntries.size(); ) {
		DebugLineEntry* e = &lineEntries[i];
		e->time -= dt;
		if (e->time < 0) {
			trim++;
			lineEntries[i] = lineEntries[lineEntries.size() - trim];
		}
		else {
			++i;
		}
		if (i + trim >= lineEntries.size()) {
			break;
		}
	}
	lineEntries.resize(lineEntries.size() - trim);
	trim = 0;
	for (int i = 0; i < collisionLogs.size(); ) {
		collisionLogs[i].time -= dt;
		if (collisionLogs[i].time < 0) {
			trim++;
			collisionLogs[i] = collisionLogs[collisionLogs.size() - trim];
		}
		else {
			++i;
		}
		if (i + trim >= collisionLogs.size()) {
			break;
		}
	}
	collisionLogs.resize(collisionLogs.size() - trim);
	stringEntries.clear();
	texEntries.clear();
}

SimpleFont* Debug::GetDebugFont() {
	return debugFont;
}

void Debug::CreateDebugFont(const std::string& dataFile, Texture& tex) {
	debugFont = new SimpleFont(dataFile, tex);
}

const std::vector<Debug::DebugStringEntry>& Debug::GetDebugStrings() {
	return stringEntries;
}

const std::vector<Debug::DebugLineEntry>& Debug::GetDebugLines() {
	return lineEntries;
}

const std::vector<Debug::DebugTexEntry>& Debug::GetDebugTex() {
	return texEntries;
}


void Debug::DrawDebugMenu() {
	// Set the window position to the top-right corner of the screen
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 300, 0), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Once); // Set width, adjust height automatically

	// Create a window to hold the debug info
	if (ImGui::Begin("Debug Menu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {

		if (ImGui::CollapsingHeader("Debug Info")) {
			for (const auto& s : debugEntries) {
				// Push the color before rendering the text
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(s.colour.x, s.colour.y, s.colour.z, s.colour.w));

				// Display the text with the specified color
				ImGui::Text(s.data.c_str());

				// Pop the style color after rendering the text
				ImGui::PopStyleColor();
			}
		}

		if (ImGui::CollapsingHeader("Performance")) {
			ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
			float featureTime = 3.2f; // Example value
			ImGui::Text("Feature timing: %.2f ms", featureTime);
		}

		if (ImGui::CollapsingHeader("Memory")) {
			size_t memoryUsage = GetCurrentMemoryUsageMB();
			size_t virtualMemoryUsage = GetVirtualMemoryUsageMB();
			ImGui::Text("Memory footprint: %zu MB", memoryUsage);
			ImGui::Text("Virtual memory usage: %zu MB", virtualMemoryUsage);
		}

		if (ImGui::CollapsingHeader("Collisions")) {
			for (const auto& log : collisionLogs) {
				ImGui::Text("%s collided with %s", log.objectA.c_str(), log.objectB.c_str());
				ImGui::Text("Contact: (%.2f, %.2f, %.2f)", log.contactPoint.x, log.contactPoint.y, log.contactPoint.z);
				ImGui::Text("Normal: (%.2f, %.2f, %.2f)", log.normal.x, log.normal.y, log.normal.z);
				ImGui::Text("Penetration: %.2f", log.penetrationDepth);
				ImGui::Separator();
			}
		}
		if (ImGui::CollapsingHeader("Debug Buttons")) {
			if (ImGui::Button("Load Level")) {
				 
				Debug::PrintDebugInfo({ "Loading level...", Vector4(1, 1, 1, 1) });
			}
		}

		ImGui::End(); // Close the window
	}
}


size_t Debug::GetCurrentMemoryUsageMB() {
	PROCESS_MEMORY_COUNTERS_EX pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
		return pmc.WorkingSetSize / (1024 * 1024); // Convert bytes to MB
	}
	return 0;
}

size_t  Debug::GetVirtualMemoryUsageMB() {
	PROCESS_MEMORY_COUNTERS_EX pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
		return pmc.PrivateUsage / (1024 * 1024); // Convert bytes to MB
	}
	return 0;
}

void Debug::PrintDebugInfo(DebugLogEntry string) {
	DebugLogEntry newEntry = string;
	newEntry.timestamp = GetTime();
	debugEntries.push_back(newEntry);
}


float Debug::GetTime() {
	using namespace std::chrono;
	auto now = steady_clock::now();
	auto duration = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::duration<float>>(duration).count();  // Time in seconds
}

void Debug::LogCollision(const std::string& objA, const std::string& objB,
	const Vector3& contact, const Vector3& normal, float penetration) {
	for (auto& log : collisionLogs) {
		// Check if the same objects have already collided
		if ((log.objectA == objA && log.objectB == objB) ||
			(log.objectA == objB && log.objectB == objA)) {
			// Update existing log instead of adding a duplicate
			log.contactPoint = contact;
			log.normal = normal;
			log.penetrationDepth = penetration;
			log.time = 5.0f;  // Reset timer for displaying log
			return; // Exit function early
		}
	}

	// If not found, add a new entry
	DebugCollisionInfo log;
	log.objectA = objA;
	log.objectB = objB;
	log.contactPoint = contact;
	log.normal = normal;
	log.penetrationDepth = penetration;
	log.time = 5.0f; // Display log for 5 seconds
	collisionLogs.push_back(log);
}

