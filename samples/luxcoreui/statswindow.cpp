/***************************************************************************
 * Copyright 1998-2015 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#include <iostream>
#include <boost/format.hpp>

#include "luxcoreapp.h"

using namespace std;
using namespace luxrays;
using namespace luxcore;

//------------------------------------------------------------------------------
// StatsWindow
//------------------------------------------------------------------------------

void StatsWindow::Draw() {
	if (!opened)
		return;

	ImGui::SetNextWindowPos(ImVec2(0, 200), ImGuiSetCond_Appearing);
	ImGui::SetNextWindowSize(ImVec2(640, 200), ImGuiSetCond_Appearing);

	if (ImGui::Begin(title.c_str(), &opened)) {
		RenderSession *session = app->session;
		RenderConfig *config = app->config;

		const Properties &stats = session->GetStats();

		// Rendering information
		if (ImGui::CollapsingHeader("Rendering information", NULL, true, true)) {
			const string engineType = config->GetProperty("renderengine.type").Get<string>();
			ImGui::Text("Render engine: %s", engineType.c_str());

			const string samplerName = ((engineType == "BIASPATHCPU") || (engineType == "BIASPATHOCL") ||
				(engineType == "RTBIASPATHOCL")) ?
					"N/A" : config->GetProperty("sampler.type").Get<string>();
			ImGui::Text("Sampler: %s", samplerName.c_str());

			ImGui::Text("Rendering time: %dsecs", int(stats.Get("stats.renderengine.time").Get<double>()));
			ImGui::Text("Film resolution: %d x %d", session->GetFilm().GetWidth(), session->GetFilm().GetHeight());
			int frameBufferWidth, frameBufferHeight;
			glfwGetFramebufferSize(app->window, &frameBufferWidth, &frameBufferHeight);
			ImGui::Text("Screen resolution: %d x %d", frameBufferWidth, frameBufferHeight);

#if !defined(LUXRAYS_DISABLE_OPENCL)
			if (engineType == "RTPATHOCL") {
				static float fps = 0.f;
				// This is a simple trick to smooth the fps counter
				const double frameTime = stats.Get("stats.rtpathocl.frame.time").Get<double>();
				const double adjustFactor = (frameTime > 0.1) ? 0.25 : .025;
				fps = Lerp<float>(adjustFactor, fps, (frameTime > 0.0) ? (1.0 / frameTime) : 0.0);

				ImGui::Text("Screen refresh: %d/%dms (%.1ffps)",
						int((fps > 0.f) ? (1000.0 / fps) : 0.0),
						config->GetProperty("screen.refresh.interval").Get<u_int>(),
						fps);
			} else if (engineType == "RTBIASPATHOCL") {
				static float fps = 0.f;
				// This is a simple trick to smooth the fps counter
				const double frameTime = stats.Get("stats.rtbiaspathocl.frame.time").Get<double>();
				const double adjustFactor = (frameTime > 0.1) ? 0.25 : .025;
				fps = Lerp<float>(adjustFactor, fps, (frameTime > 0.0) ? (1.0 / frameTime) : 0.0);

				ImGui::Text("Screen refresh: %d/%dms (%.1ffps)",
						int((fps > 0.f) ? (1000.0 / fps) : 0.0),
						config->GetProperty("screen.refresh.interval").Get<u_int>(),
						fps);
			} else
#endif
			{
				ImGui::Text("Screen refresh: %dms", config->GetProperty("screen.refresh.interval").Get<u_int>());
			}
		}

		if (ImGui::CollapsingHeader("Intersection devices", NULL, true, true)) {
			// Intersection devices
			const Property &deviceNames = stats.Get("stats.renderengine.devices");

			double minPerf = numeric_limits<double>::infinity();
			double totalPerf = 0.0;
			for (u_int i = 0; i < deviceNames.GetSize(); ++i) {
				const string deviceName = deviceNames.Get<string>(i);

				const double perf = stats.Get("stats.renderengine.devices." + deviceName + ".performance.total").Get<double>();
				minPerf = Min(minPerf, perf);
				totalPerf += perf;
			}

			for (u_int i = 0; i < deviceNames.GetSize(); ++i) {
				const string deviceName = deviceNames.Get<string>(i);

				ImGui::Text("[%s][Rays/sec %dK (%dK + %dK)][Prf Idx %.2f][Wrkld %.1f%%][Mem %dM/%dM]",
					deviceName.c_str(),
					int(stats.Get("stats.renderengine.devices." + deviceName + ".performance.total").Get<double>() / 1000.0),
					int(stats.Get("stats.renderengine.devices." + deviceName + ".performance.serial").Get<double>() / 1000.0),
					int(stats.Get("stats.renderengine.devices." + deviceName + ".performance.dataparallel").Get<double>() / 1000.0),
					(stats.Get("stats.renderengine.devices." + deviceName + ".performance.total").Get<double>() / minPerf),
					(100.0 * stats.Get("stats.renderengine.devices." + deviceName + ".performance.total").Get<double>() / totalPerf),
					int(stats.Get("stats.renderengine.devices." + deviceName + ".memory.used").Get<double>() / (1024 * 1024)),
					int(stats.Get("stats.renderengine.devices." + deviceName + ".memory.total").Get<double>() / (1024 * 1024)));

			}
		}
	}
	ImGui::End();
}
