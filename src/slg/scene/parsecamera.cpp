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

#include "slg/scene/scene.h"

using namespace std;
using namespace luxrays;
using namespace slg;

void Scene::ParseCamera(const Properties &props) {
	if (!props.HaveNames("scene.camera")) {
		// There is no camera definition
		return;
	}

	Camera *newCamera = Camera::AllocCamera(props);

	// Use the new camera
	delete camera;
	camera = newCamera;

	editActions.AddAction(CAMERA_EDIT);
}