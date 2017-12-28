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

#ifndef _SLG_IMAGEMAP_H
#define	_SLG_IMAGEMAP_H

#include <OpenEXR/half.h>

#include <string>
#include <limits>

#include <boost/serialization/version.hpp>
#include <boost/serialization/export.hpp>

#include "eos/portable_oarchive.hpp"
#include "eos/portable_iarchive.hpp"

#include "luxrays/luxrays.h"
#include "luxrays/core/color/color.h"
#include "luxrays/core/geometry/uv.h"
#include "luxrays/utils/properties.h"
#include "slg/core/namedobject.h"
#include "slg/utils/halfserialization.h"

namespace slg {

// OpenCL data types
namespace ocl {
#include "slg/imagemap/imagemap_types.cl"
}

//------------------------------------------------------------------------------
// ImageMapPixel
//------------------------------------------------------------------------------

template <class T, u_int CHANNELS> class ImageMapPixel {
public:
	ImageMapPixel(T v = 0) {
		for (u_int i = 0; i < CHANNELS; ++i)
			c[i] = v;
	}

	ImageMapPixel(T cs[CHANNELS]) {
		for (u_int i = 0; i < CHANNELS; ++i)
			c[i] = cs[i];
	}

	~ImageMapPixel() { }
	
	u_int GetChannelCount() const { return CHANNELS; }

	float GetFloat() const;
	luxrays::Spectrum GetSpectrum() const;
	float GetAlpha() const;

	void Set(const T *src) {
		for (u_int i = 0; i < CHANNELS; ++i)
			c[i] = src[i];
	}
	void SetFloat(const float v);
	void SetSpectrum(const luxrays::Spectrum &v);

	void ReverseGammaCorrection(const float gamma);

	friend class boost::serialization::access;

	T c[CHANNELS];

private:
	template<class Archive> void serialize(Archive &ar, const u_int version) {
		ar & c;
	}
};

// Mostly used for Boost serialization macros
typedef ImageMapPixel<u_char, 1> ImageMapPixelUChar1;
typedef ImageMapPixel<u_char, 2> ImageMapPixelUChar2;
typedef ImageMapPixel<u_char, 3> ImageMapPixelUChar3;
typedef ImageMapPixel<u_char, 4> ImageMapPixelUChar4;
typedef ImageMapPixel<half, 1> ImageMapPixelHalf1;
typedef ImageMapPixel<half, 2> ImageMapPixelHalf2;
typedef ImageMapPixel<half, 3> ImageMapPixelHalf3;
typedef ImageMapPixel<half, 4> ImageMapPixelHalf4;
typedef ImageMapPixel<float, 1> ImageMapPixelFloat1;
typedef ImageMapPixel<float, 2> ImageMapPixelFloat2;
typedef ImageMapPixel<float, 3> ImageMapPixelFloat3;
typedef ImageMapPixel<float, 4> ImageMapPixelFloat4;

//------------------------------------------------------------------------------
// u_char x 1 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<u_char, 1>::GetFloat() const {
	const float norm = 1.f / std::numeric_limits<u_char>::max();
	return c[0] * norm;
}

template<> inline luxrays::Spectrum ImageMapPixel<u_char, 1>::GetSpectrum() const {
	const float norm = 1.f / std::numeric_limits<u_char>::max();
	return luxrays::Spectrum(c[0] * norm);
}

template<> inline float ImageMapPixel<u_char, 1>::GetAlpha() const {
	return 1.f;
}

template<> inline void ImageMapPixel<u_char, 1>::ReverseGammaCorrection(const float gamma) {
	const u_char maxv = std::numeric_limits<u_char>::max();
	const float norm = 1.f / maxv;
	c[0] = (u_char)floorf(powf(c[0] * norm, gamma) * maxv + .5f);
}

template<> inline void ImageMapPixel<u_char, 1>::SetFloat(const float v) {
	const float maxv = std::numeric_limits<u_char>::max();
	c[0] = (u_char)floorf(v * maxv + .5f);
}

template<> inline void ImageMapPixel<u_char, 1>::SetSpectrum(const luxrays::Spectrum &v) {
	const float maxv = std::numeric_limits<u_char>::max();
	c[0] = (u_char)floorf(v.Y() * maxv + .5f);
}

//------------------------------------------------------------------------------
// u_char x 2 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<u_char, 2>::GetFloat() const {
	const float norm = 1.f / std::numeric_limits<u_char>::max();
	return c[0] * norm;
}

template<> inline luxrays::Spectrum ImageMapPixel<u_char, 2>::GetSpectrum() const {
	const float norm = 1.f / std::numeric_limits<u_char>::max();
	return luxrays::Spectrum(c[0] * norm);
}

template<> inline float ImageMapPixel<u_char, 2>::GetAlpha() const {
	const float norm = 1.f / std::numeric_limits<u_char>::max();
	return c[1] * norm;
}

template<> inline void ImageMapPixel<u_char, 2>::ReverseGammaCorrection(const float gamma) {
	const u_char maxv = std::numeric_limits<u_char>::max();
	const float norm = 1.f / maxv;
	c[0] = (u_char)floorf(powf(c[0] * norm, gamma) * maxv + .5f);
}

template<> inline void ImageMapPixel<u_char, 2>::SetFloat(const float v) {
	const float maxv = std::numeric_limits<u_char>::max();
	c[0] = (u_char)floorf(v * maxv + .5f);
}

template<> inline void ImageMapPixel<u_char, 2>::SetSpectrum(const luxrays::Spectrum &v) {
	const float maxv = std::numeric_limits<u_char>::max();
	c[0] = (u_char)floorf(v.Y() * maxv + .5f);
}

//------------------------------------------------------------------------------
// u_char x 3 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<u_char, 3>::GetFloat() const {
	const float norm = 1.f / std::numeric_limits<u_char>::max();
	return luxrays::Spectrum(c[0] * norm, c[1] * norm, c[2] * norm).Y();
}

template<> inline luxrays::Spectrum ImageMapPixel<u_char, 3>::GetSpectrum() const {
	const float norm = 1.f / std::numeric_limits<u_char>::max();
	return luxrays::Spectrum(c[0] * norm, c[1] * norm, c[2] * norm);
}

template<> inline float ImageMapPixel<u_char, 3>::GetAlpha() const {
	return 1.f;
}

template<> inline void ImageMapPixel<u_char, 3>::ReverseGammaCorrection(const float gamma) {
	const u_char maxv = std::numeric_limits<u_char>::max();
	const float norm = 1.f / maxv;
	c[0] = (u_char)floorf(powf(c[0] * norm, gamma) * maxv + .5f);
	c[1] = (u_char)floorf(powf(c[1] * norm, gamma) * maxv + .5f);
	c[2] = (u_char)floorf(powf(c[2] * norm, gamma) * maxv + .5f);
}

template<> inline void ImageMapPixel<u_char, 3>::SetFloat(const float v) {
	const float maxv = std::numeric_limits<u_char>::max();
	c[0] = (u_char)floorf(v * maxv + .5f);
	c[1] = c[0];
	c[2] = c[0];
}

template<> inline void ImageMapPixel<u_char, 3>::SetSpectrum(const luxrays::Spectrum &v) {
	const float maxv = std::numeric_limits<u_char>::max();
	c[0] = (u_char)floorf(v.c[0] * maxv + .5f);
	c[1] = (u_char)floorf(v.c[1] * maxv + .5f);
	c[2] = (u_char)floorf(v.c[2] * maxv + .5f);
}

//------------------------------------------------------------------------------
// u_char x 4 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<u_char, 4>::GetFloat() const {
	const float norm = 1.f / std::numeric_limits<u_char>::max();
	return luxrays::Spectrum(c[0] * norm, c[1] * norm, c[2] * norm).Y();
}

template<> inline luxrays::Spectrum ImageMapPixel<u_char, 4>::GetSpectrum() const {
	const float norm = 1.f / std::numeric_limits<u_char>::max();
	return luxrays::Spectrum(c[0] * norm, c[1] * norm, c[2] * norm);
}

template<> inline float ImageMapPixel<u_char, 4>::GetAlpha() const {
	const float norm = 1.f / std::numeric_limits<u_char>::max();
	return c[3] * norm;
}

template<> inline void ImageMapPixel<u_char, 4>::ReverseGammaCorrection(const float gamma) {
	const u_char maxv = std::numeric_limits<u_char>::max();
	const float norm = 1.f / maxv;
	c[0] = (u_char)floorf(powf(c[0] * norm, gamma) * maxv + .5f);
	c[1] = (u_char)floorf(powf(c[1] * norm, gamma) * maxv + .5f);
	c[2] = (u_char)floorf(powf(c[2] * norm, gamma) * maxv + .5f);
}

template<> inline void ImageMapPixel<u_char, 4>::SetFloat(const float v) {
	const float maxv = std::numeric_limits<u_char>::max();
	c[0] = (u_char)floorf(v * maxv + .5f);
	c[1] = c[0];
	c[2] = c[0];
}

template<> inline void ImageMapPixel<u_char, 4>::SetSpectrum(const luxrays::Spectrum &v) {
	const float maxv = std::numeric_limits<u_char>::max();
	c[0] = (u_char)floorf(v.c[0] * maxv + .5f);
	c[1] = (u_char)floorf(v.c[1] * maxv + .5f);
	c[2] = (u_char)floorf(v.c[2] * maxv + .5f);
}

//------------------------------------------------------------------------------
// half x 1 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<half, 1>::GetFloat() const {
	return c[0];
}

template<> inline luxrays::Spectrum ImageMapPixel<half, 1>::GetSpectrum() const {
	return luxrays::Spectrum(c[0]);
}

template<> inline float ImageMapPixel<half, 1>::GetAlpha() const {
	return 1.f;
}

template<> inline void ImageMapPixel<half, 1>::ReverseGammaCorrection(const float gamma) {
	c[0] = powf(c[0], gamma);
}

template<> inline void ImageMapPixel<half, 1>::SetFloat(const float v) {
	c[0] = v;
}

template<> inline void ImageMapPixel<half, 1>::SetSpectrum(const luxrays::Spectrum &v) {
	c[0] = v.Y();
}

//------------------------------------------------------------------------------
// half x 2 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<half, 2>::GetFloat() const {
	return c[0];
}

template<> inline luxrays::Spectrum ImageMapPixel<half, 2>::GetSpectrum() const {
	return luxrays::Spectrum(c[0]);
}

template<> inline float ImageMapPixel<half, 2>::GetAlpha() const {
	return c[1];
}

template<> inline void ImageMapPixel<half, 2>::ReverseGammaCorrection(const float gamma) {
	c[0] = powf(c[0], gamma);
}

template<> inline void ImageMapPixel<half, 2>::SetFloat(const float v) {
	c[0] = v;
}

template<> inline void ImageMapPixel<half, 2>::SetSpectrum(const luxrays::Spectrum &v) {
	c[0] = v.Y();
}

//------------------------------------------------------------------------------
// half x 3 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<half, 3>::GetFloat() const {
	return luxrays::Spectrum(c[0], c[1], c[2]).Y();
}

template<> inline luxrays::Spectrum ImageMapPixel<half, 3>::GetSpectrum() const {
	return luxrays::Spectrum(c[0], c[1], c[2]);
}

template<> inline float ImageMapPixel<half, 3>::GetAlpha() const {
	return 1.f;
}

template<> inline void ImageMapPixel<half, 3>::ReverseGammaCorrection(const float gamma) {
	c[0] = powf(c[0], gamma);
	c[1] = powf(c[1], gamma);
	c[2] = powf(c[2], gamma);
}

template<> inline void ImageMapPixel<half, 3>::SetFloat(const float v) {
	c[0] = v;
	c[1] = v;
	c[2] = v;
}

template<> inline void ImageMapPixel<half, 3>::SetSpectrum(const luxrays::Spectrum &v) {
	c[0] = v.c[0];
	c[1] = v.c[1];
	c[2] = v.c[2];
}

//------------------------------------------------------------------------------
// half x 4 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<half, 4>::GetFloat() const {
	return luxrays::Spectrum(c[0], c[1], c[2]).Y();
}

template<> inline luxrays::Spectrum ImageMapPixel<half, 4>::GetSpectrum() const {
	return luxrays::Spectrum(c[0], c[1], c[2]);
}

template<> inline float ImageMapPixel<half, 4>::GetAlpha() const {
	return c[3];
}

template<> inline void ImageMapPixel<half, 4>::ReverseGammaCorrection(const float gamma) {
	c[0] = powf(c[0], gamma);
	c[1] = powf(c[1], gamma);
	c[2] = powf(c[2], gamma);
}

template<> inline void ImageMapPixel<half, 4>::SetFloat(const float v) {
	c[0] = v;
	c[1] = v;
	c[2] = v;
}

template<> inline void ImageMapPixel<half, 4>::SetSpectrum(const luxrays::Spectrum &v) {
	c[0] = v.c[0];
	c[1] = v.c[1];
	c[2] = v.c[2];
}


//------------------------------------------------------------------------------
// float x 1 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<float, 1>::GetFloat() const {
	return c[0];
}

template<> inline luxrays::Spectrum ImageMapPixel<float, 1>::GetSpectrum() const {
	return luxrays::Spectrum(c[0]);
}

template<> inline float ImageMapPixel<float, 1>::GetAlpha() const {
	return 1.f;
}

template<> inline void ImageMapPixel<float, 1>::ReverseGammaCorrection(const float gamma) {
	c[0] = powf(c[0], gamma);
}

template<> inline void ImageMapPixel<float, 1>::SetFloat(const float v) {
	c[0] = v;
}

template<> inline void ImageMapPixel<float, 1>::SetSpectrum(const luxrays::Spectrum &v) {
	c[0] = v.Y();
}

//------------------------------------------------------------------------------
// float x 2 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<float, 2>::GetFloat() const {
	return c[0];
}

template<> inline luxrays::Spectrum ImageMapPixel<float, 2>::GetSpectrum() const {
	return luxrays::Spectrum(c[0]);
}

template<> inline float ImageMapPixel<float, 2>::GetAlpha() const {
	return c[1];
}

template<> inline void ImageMapPixel<float, 2>::ReverseGammaCorrection(const float gamma) {
	c[0] = powf(c[0], gamma);
}

template<> inline void ImageMapPixel<float, 2>::SetFloat(const float v) {
	c[0] = v;
}

template<> inline void ImageMapPixel<float, 2>::SetSpectrum(const luxrays::Spectrum &v) {
	c[0] = v.Y();
}

//------------------------------------------------------------------------------
// float x 3 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<float, 3>::GetFloat() const {
	return luxrays::Spectrum(c[0], c[1], c[2]).Y();
}

template<> inline luxrays::Spectrum ImageMapPixel<float, 3>::GetSpectrum() const {
	return luxrays::Spectrum(c[0], c[1], c[2]);
}

template<> inline float ImageMapPixel<float, 3>::GetAlpha() const {
	return 1.f;
}

template<> inline void ImageMapPixel<float, 3>::ReverseGammaCorrection(const float gamma) {
	c[0] = powf(c[0], gamma);
	c[1] = powf(c[1], gamma);
	c[2] = powf(c[2], gamma);
}

template<> inline void ImageMapPixel<float, 3>::SetFloat(const float v) {
	c[0] = v;
	c[1] = v;
	c[2] = v;
}

template<> inline void ImageMapPixel<float, 3>::SetSpectrum(const luxrays::Spectrum &v) {
	c[0] = v.c[0];
	c[1] = v.c[1];
	c[2] = v.c[2];
}

//------------------------------------------------------------------------------
// float x 4 specialization
//------------------------------------------------------------------------------

template<> inline float ImageMapPixel<float, 4>::GetFloat() const {
	return luxrays::Spectrum(c[0], c[1], c[2]).Y();
}

template<> inline luxrays::Spectrum ImageMapPixel<float, 4>::GetSpectrum() const {
	return luxrays::Spectrum(c[0], c[1], c[2]);
}

template<> inline float ImageMapPixel<float, 4>::GetAlpha() const {
	return c[3];
}

template<> inline void ImageMapPixel<float, 4>::ReverseGammaCorrection(const float gamma) {
	c[0] = powf(c[0], gamma);
	c[1] = powf(c[1], gamma);
	c[2] = powf(c[2], gamma);
}

template<> inline void ImageMapPixel<float, 4>::SetFloat(const float v) {
	c[0] = v;
	c[1] = v;
	c[2] = v;
}

template<> inline void ImageMapPixel<float, 4>::SetSpectrum(const luxrays::Spectrum &v) {
	c[0] = v.c[0];
	c[1] = v.c[1];
	c[2] = v.c[2];
}
//------------------------------------------------------------------------------
// ImageMapStorage
//------------------------------------------------------------------------------

class ImageMapStorage {
public:
	typedef enum {
		BYTE,
		HALF,
		FLOAT,
		
		// This one isn't a real storage type and is used only as argument
		// of ImageMap constructor
		AUTO
	} StorageType;

	typedef enum {
		DEFAULT,
		RED,
		GREEN,
		BLUE,
		ALPHA,
		MEAN,
		WEIGHTED_MEAN,
		RGB
	} ChannelSelectionType;

	ImageMapStorage(const u_int w, const u_int h) {
		width = w;
		height = h;
	}
	virtual ~ImageMapStorage() { }

	virtual ImageMapStorage *SelectChannel(const ChannelSelectionType selectionType) const = 0;

	virtual StorageType GetStorageType() const = 0;
	virtual u_int GetChannelCount() const = 0;
	virtual size_t GetMemorySize() const = 0;
	virtual void *GetPixelsData() const = 0;

	virtual float GetFloat(const luxrays::UV &uv) const = 0;
	virtual float GetFloat(const u_int index) const = 0;
	virtual luxrays::Spectrum GetSpectrum(const luxrays::UV &uv) const = 0;
	virtual luxrays::Spectrum GetSpectrum(const u_int index) const = 0;
	virtual float GetAlpha(const luxrays::UV &uv) const = 0;
	virtual float GetAlpha(const u_int index) const = 0;
	virtual luxrays::UV GetDuv(const luxrays::UV &uv) const = 0;
	virtual luxrays::UV GetDuv(const u_int index) const = 0;

	virtual void ReverseGammaCorrection(const float gamma) = 0;

	virtual ImageMapStorage *Copy() const = 0;

	static StorageType String2StorageType(const std::string &type);
	static std::string StorageType2String(const StorageType type);
	static ChannelSelectionType String2ChannelSelectionType(const std::string &type);

	friend class boost::serialization::access;

	u_int width, height;

protected:
	// Used by serialization
	ImageMapStorage() { }

	template<class Archive> void serialize(Archive &ar, const u_int version) {
		ar & width;
		ar & height;
	}
};

template <class T, u_int CHANNELS> class ImageMapStorageImpl : public ImageMapStorage {
public:
	ImageMapStorageImpl(ImageMapPixel<T, CHANNELS> *ps, const u_int w,
			const u_int h) : ImageMapStorage(w, h), pixels(ps) { }
	virtual ~ImageMapStorageImpl() { delete[] pixels; }

	virtual ImageMapStorage *SelectChannel(const ChannelSelectionType selectionType) const;

	virtual StorageType GetStorageType() const;
	virtual u_int GetChannelCount() const { return CHANNELS; }
	virtual size_t GetMemorySize() const { return width * height * CHANNELS * sizeof(T); };
	virtual void *GetPixelsData() const { return pixels; }

	virtual float GetFloat(const luxrays::UV &uv) const;
	virtual float GetFloat(const u_int index) const;
	virtual luxrays::Spectrum GetSpectrum(const luxrays::UV &uv) const;
	virtual luxrays::Spectrum GetSpectrum(const u_int index) const;
	virtual float GetAlpha(const luxrays::UV &uv) const;
	virtual float GetAlpha(const u_int index) const;
	virtual luxrays::UV GetDuv(const luxrays::UV &uv) const;
	virtual luxrays::UV GetDuv(const u_int index) const;

	virtual void ReverseGammaCorrection(const float gamma);

	virtual ImageMapStorage *Copy() const;

	friend class boost::serialization::access;

private:
	// Used by serialization
	ImageMapStorageImpl() {
		pixels = NULL;
	}

	const ImageMapPixel<T, CHANNELS> *GetTexel(const int s, const int t) const;

	template<class Archive> void save(Archive &ar, const unsigned int version) const {
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ImageMapStorage);

		const u_int size = width * height;
		ar & size;

		for (u_int i = 0; i < size; ++i)
			ar & pixels[i];
	}

	template<class Archive>	void load(Archive &ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ImageMapStorage);

		u_int size;
		ar & size;
		pixels = new ImageMapPixel<T, CHANNELS>[size];

		for (u_int i = 0; i < size; ++i)
			ar & pixels[i];
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

	ImageMapPixel<T, CHANNELS> *pixels;
};

// Mostly used for Boost serialization macros
typedef ImageMapStorageImpl<u_char, 1> ImageMapStorageImplUChar1;
typedef ImageMapStorageImpl<u_char, 2> ImageMapStorageImplUChar2;
typedef ImageMapStorageImpl<u_char, 3> ImageMapStorageImplUChar3;
typedef ImageMapStorageImpl<u_char, 4> ImageMapStorageImplUChar4;
typedef ImageMapStorageImpl<half, 1> ImageMapStorageImplHalf1;
typedef ImageMapStorageImpl<half, 2> ImageMapStorageImplHalf2;
typedef ImageMapStorageImpl<half, 3> ImageMapStorageImplHalf3;
typedef ImageMapStorageImpl<half, 4> ImageMapStorageImplHalf4;
typedef ImageMapStorageImpl<float, 1> ImageMapStorageImplFloat1;
typedef ImageMapStorageImpl<float, 2> ImageMapStorageImplFloat2;
typedef ImageMapStorageImpl<float, 3> ImageMapStorageImplFloat3;
typedef ImageMapStorageImpl<float, 4> ImageMapStorageImplFloat4;

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<u_char, 1>::GetStorageType() const {
	return ImageMapStorage::BYTE;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<u_char, 2>::GetStorageType() const {
	return ImageMapStorage::BYTE;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<u_char, 3>::GetStorageType() const {
	return ImageMapStorage::BYTE;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<u_char, 4>::GetStorageType() const {
	return ImageMapStorage::BYTE;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<half, 1>::GetStorageType() const {
	return ImageMapStorage::HALF;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<half, 2>::GetStorageType() const {
	return ImageMapStorage::HALF;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<half, 3>::GetStorageType() const {
	return ImageMapStorage::HALF;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<half, 4>::GetStorageType() const {
	return ImageMapStorage::HALF;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<float, 1>::GetStorageType() const {
	return ImageMapStorage::FLOAT;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<float, 2>::GetStorageType() const {
	return ImageMapStorage::FLOAT;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<float, 3>::GetStorageType() const {
	return ImageMapStorage::FLOAT;
}

template<> inline ImageMapStorage::StorageType ImageMapStorageImpl<float, 4>::GetStorageType() const {
	return ImageMapStorage::FLOAT;
}

template <class T> ImageMapStorage *AllocImageMapStorage(const u_int channels,
		const u_int width, const u_int height) {
	const u_int pixelCount = width * height;

	switch (channels) {
		case 1:
			return new ImageMapStorageImpl<T, 1>(new ImageMapPixel<T, 1>[pixelCount], width, height);
		case 2:
			return new ImageMapStorageImpl<T, 2>(new ImageMapPixel<T, 2>[pixelCount], width, height);
		case 3:
			return new ImageMapStorageImpl<T, 3>(new ImageMapPixel<T, 3>[pixelCount], width, height);
		case 4:
			return new ImageMapStorageImpl<T, 4>(new ImageMapPixel<T, 4>[pixelCount], width, height);
		default:
			return NULL;
	}
}

//------------------------------------------------------------------------------
// ImageMap
//------------------------------------------------------------------------------

class ImageMapCache;

class ImageMap : public NamedObject {
public:
	ImageMap(const std::string &fileName, const float gamma,
		const ImageMapStorage::StorageType storageType);
	~ImageMap();

	void Preprocess();

	void SelectChannel(const ImageMapStorage::ChannelSelectionType selectionType);
	void ReverseGammaCorrection();
	
	float GetGamma() const { return gamma; }
	u_int GetChannelCount() const { return pixelStorage->GetChannelCount(); }
	u_int GetWidth() const { return pixelStorage->width; }
	u_int GetHeight() const { return pixelStorage->height; }
	const ImageMapStorage *GetStorage() const { return pixelStorage; }

	float GetFloat(const luxrays::UV &uv) const { return pixelStorage->GetFloat(uv); }
	luxrays::Spectrum GetSpectrum(const luxrays::UV &uv) const { return pixelStorage->GetSpectrum(uv); }
	float GetAlpha(const luxrays::UV &uv) const { return pixelStorage->GetAlpha(uv); }
	luxrays::UV GetDuv(const luxrays::UV &uv) const { return pixelStorage->GetDuv(uv); }

	void Resize(const u_int newWidth, const u_int newHeight);

	std::string GetFileExtension() const;
	void WriteImage(const std::string &fileName) const;

	float GetSpectrumMean() const { return imageMean; }
	float GetSpectrumMeanY() const { return imageMeanY; }

	ImageMap *Copy() const;
	luxrays::Properties ToProperties(const std::string &prefix, const bool includeBlobImg) const;
	
	// The following 3 methods always return an ImageMap with FLOAT storage
	static ImageMap *Merge(const ImageMap *map0, const ImageMap *map1, const u_int channels);
	static ImageMap *Merge(const ImageMap *map0, const ImageMap *map1, const u_int channels,
		const u_int width, const u_int height);
	static ImageMap *Resample(const ImageMap *map, const u_int channels,
		const u_int width, const u_int height);
	static ImageMap *FromProperties(const luxrays::Properties &props, const std::string &prefix);

	template <class T> static ImageMap *AllocImageMap(const float gamma, const u_int channels,
		const u_int width, const u_int height) {
		ImageMapStorage *imageMapStorage = AllocImageMapStorage<T>(channels, width, height);

		return new ImageMap(imageMapStorage, gamma);
	}

	luxrays::Properties ToProperties(const std::string &prefix);

	friend class boost::serialization::access;

private:
	// Used by serialization
	ImageMap();
	ImageMap(ImageMapStorage *pixels, const float gamma);

	float CalcSpectrumMean() const;
	float CalcSpectrumMeanY() const;

	template<class Archive> void save(Archive &ar, const unsigned int version) const {
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);

		// The image is internally stored always with a 1.0 gamma
		const float imageMapStorageGamma = 1.f;
		ar & imageMapStorageGamma;
		ar & pixelStorage;
		ar & imageMean;
		ar & imageMeanY;
	}
	template<class Archive>	void load(Archive &ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);

		ar & gamma;
		ar & pixelStorage;
		ar & imageMean;
		ar & imageMeanY;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

	float gamma;
	ImageMapStorage *pixelStorage;

	// Cached image information
	float imageMean, imageMeanY;
};

}

BOOST_SERIALIZATION_ASSUME_ABSTRACT(slg::ImageMapStorage)

BOOST_CLASS_VERSION(slg::ImageMapPixelUChar1, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelUChar2, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelUChar3, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelUChar4, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelHalf1, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelHalf2, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelHalf3, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelHalf4, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelFloat1, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelFloat2, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelFloat3, 1)
BOOST_CLASS_VERSION(slg::ImageMapPixelFloat4, 1)

BOOST_CLASS_VERSION(slg::ImageMapStorageImplUChar1, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplUChar2, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplUChar3, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplUChar4, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplHalf1, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplHalf2, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplHalf3, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplHalf4, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplFloat1, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplFloat2, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplFloat3, 1)
BOOST_CLASS_VERSION(slg::ImageMapStorageImplFloat4, 1)

BOOST_CLASS_VERSION(slg::ImageMap, 2)

BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelUChar1)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelUChar2)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelUChar3)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelUChar4)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelHalf1)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelHalf2)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelHalf3)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelHalf4)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelFloat1)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelFloat2)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelFloat3)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapPixelFloat4)

BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplUChar1)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplUChar2)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplUChar3)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplUChar4)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplHalf1)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplHalf2)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplHalf3)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplHalf4)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplFloat1)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplFloat2)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplFloat3)
BOOST_CLASS_EXPORT_KEY(slg::ImageMapStorageImplFloat4)

BOOST_CLASS_EXPORT_KEY(slg::ImageMap)

#endif	/* _SLG_IMAGEMAP_H */
