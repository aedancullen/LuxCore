/***************************************************************************
 * Copyright 1998-2018 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxCoreRender.                                   *
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

#include "slg/cameras/swingstereo.h"

using namespace std;
using namespace luxrays;
using namespace slg;

SwingStereoCamera::SwingStereoCamera(const luxrays::Point &o, const luxrays::Point &t,
			const luxrays::Vector &u) : SwingEnvironmentCamera(SWINGSTEREO, o, t, u),
			leftEye(NULL), rightEye(NULL) {
	horizStereoEyesDistance = .0626f;
}

SwingStereoCamera::~SwingStereoCamera() {
	delete leftEye;
	delete rightEye;
}

Matrix4x4 SwingStereoCamera::GetRasterToCameraMatrix(const u_int index) const {
	if (index == 0)
		return leftEye->GetRasterToCameraMatrix();
	else if (index == 1)
		return rightEye->GetRasterToCameraMatrix();
	else
		throw runtime_error("Unknown index in GetRasterToCameraMatrix(): " + ToString(index));
}

Matrix4x4 SwingStereoCamera::GetCameraToWorldMatrix(const u_int index) const {
	if (index == 0)
		return leftEye->GetCameraToWorldMatrix();
	else if (index == 1)
		return rightEye->GetCameraToWorldMatrix();
	else
		throw runtime_error("Unknown index in GetCameraToWorldMatrix(): " + ToString(index));
}

void SwingStereoCamera::Update(const u_int width, const u_int height,
		const u_int *filmSubRegion) {
	/*if (filmSubRegion)
		throw runtime_error("Stereo camera doesn't support subregion rendering");*/

	filmWidth = width;
	filmHeight = height;

	// Used to translate the camera
	dir = target - orig;
	dir = Normalize(dir);

	x = Cross(dir, up);
	x = Normalize(x);

	y = Cross(x, dir);
	y = Normalize(y);

	// Create left eye camera
	delete leftEye;
	leftEye = new SwingEnvironmentCamera(orig, target, up);
	leftEye->clipHither = clipHither;
	leftEye->clipYon = clipYon;
	leftEye->shutterOpen = shutterOpen;
	leftEye->shutterClose = shutterClose;


	leftEye->lensRadius = lensRadius;
	leftEye->focalDistance = focalDistance;
	leftEye->autoFocus = autoFocus;

	leftEye->horizSwingDistance = -horizStereoEyesDistance * .5f;
	
	leftEye->Update(filmWidth, filmHeight / 2, NULL);

	// Create right eye camera
	delete rightEye;
	rightEye = new SwingEnvironmentCamera(orig, target, up);

	rightEye->clipHither = clipHither;
	rightEye->clipYon = clipYon;
	rightEye->shutterOpen = shutterOpen;
	rightEye->shutterClose = shutterClose;


	rightEye->lensRadius = lensRadius;
	rightEye->focalDistance = focalDistance;
	rightEye->autoFocus = autoFocus;

	rightEye->horizSwingDistance = horizStereoEyesDistance * .5f;

	rightEye->Update(filmWidth, filmHeight / 2, NULL);
}

void SwingStereoCamera::GenerateRay(const float filmX, const float filmY,
	Ray *ray, const float u1, const float u2, const float u3) const {
	if (filmY < filmHeight / 2)
		leftEye->GenerateRay(filmX, filmY, ray, u1, u2, u3);
	else
		rightEye->GenerateRay(filmX, filmY - filmHeight / 2, ray, u1, u2, u3);
}

bool SwingStereoCamera::GetSamplePosition(Ray *eyeRay, float *filmX, float *filmY) const {
	// BIDIRCPU/LIGHTCPU don't support stereo rendering
	return leftEye->GetSamplePosition(eyeRay, filmX, filmY);
}

bool SwingStereoCamera::SampleLens(const float time, const float u1, const float u2,
	luxrays::Point *lensPoint) const {
	// BIDIRCPU/LIGHTCPU don't support stereo rendering
	return leftEye->SampleLens(time, u1, u2, lensPoint);
}

float SwingStereoCamera::GetPDF(const Vector &eyeDir, const float filmX, const float filmY) const {
	// BIDIRCPU/LIGHTCPU don't support stereo rendering
	return leftEye->GetPDF(eyeDir, filmX, filmY);
}

Properties SwingStereoCamera::ToProperties() const {
	Properties props = SwingEnvironmentCamera::ToProperties();

	props.Set(Property("scene.camera.type")("swingstereo"));
	props.Set(Property("scene.camera.eyesdistance")(horizStereoEyesDistance));

	return props;
}
