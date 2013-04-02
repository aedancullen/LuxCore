/***************************************************************************
 *   Copyright (C) 2007 by Anthony Pajot   
 *   anthony.pajot@etu.enseeiht.fr
 *
 * This file is part of FlexRay
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/

#include "luxrays/accelerators/qbvhaccel.h"
#include "luxrays/core/utils.h"
#include "luxrays/core/context.h"
#ifdef LUXRAYS_DISABLE_OPENCL
#include "luxrays/core/intersectiondevice.h"
#else
#include "luxrays/core/oclintersectiondevice.h"
#include "luxrays/kernels/kernels.h"
#endif

namespace luxrays {

/***************************************************/

#if !defined(LUXRAYS_DISABLE_OPENCL)

class OpenCLQBVHKernels : public OpenCLKernels {
public:
	OpenCLQBVHKernels(OpenCLIntersectionDevice *dev, const u_int kernelCount, u_int s) :
		OpenCLKernels(dev, kernelCount), trisBuff(NULL), qbvhBuff(NULL) {
		stackSize = s;

		const Context *deviceContext = device->GetContext();
		const std::string &deviceName(device->GetName());
		cl::Context &oclContext = device->GetOpenCLContext();
		cl::Device &oclDevice = device->GetOpenCLDevice();

		// Compile the source
		std::stringstream params;
		params << "-D QBVH_STACK_SIZE=" << stackSize;

		std::string code(
			luxrays::ocl::KernelSource_point_types +
			luxrays::ocl::KernelSource_vector_types +
			luxrays::ocl::KernelSource_ray_types +
			luxrays::ocl::KernelSource_bbox_types);
		code += luxrays::ocl::KernelSource_qbvh;
		cl::Program::Sources source(1, std::make_pair(code.c_str(), code.length()));
		cl::Program program = cl::Program(oclContext, source);
		try {
			VECTOR_CLASS<cl::Device> buildDevice;
			buildDevice.push_back(oclDevice);
			program.build(buildDevice, params.str().c_str());
		} catch (cl::Error err) {
			cl::STRING_CLASS strError = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(oclDevice);
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
				"] QBVH compilation error:\n" << strError.c_str());

			throw err;
		}

		for (u_int i = 0; i < kernelCount; ++i) {
			kernels[i] = new cl::Kernel(program, "Intersect");
			kernels[i]->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE,
				&workGroupSize);
			//LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			//	"] QBVH kernel work group size: " << workGroupSize);

			kernels[i]->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE,
				&workGroupSize);
			//LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			//	"] Suggested work group size: " << workGroupSize);

			if (device->GetDeviceDesc()->GetForceWorkGroupSize() > 0) {
				workGroupSize = device->GetDeviceDesc()->GetForceWorkGroupSize();
				//LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
				//	"] Forced work group size: " << workGroupSize);
			} else if (workGroupSize > 256) {
				// Otherwise I will probably run out of local memory
				workGroupSize = 256;
				//LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
				//	"] Cap work group size to: " << workGroupSize);
			}
		}
	}
	virtual ~OpenCLQBVHKernels() { FreeBuffers(); }

	virtual void FreeBuffers();
	void SetBuffers(cl::Buffer *trisBuff, cl::Buffer *qbvhBuff);
	virtual void UpdateDataSet(const DataSet *newDataSet) { assert(false); }
	virtual void EnqueueRayBuffer(cl::CommandQueue &oclQueue, const u_int kernelIndex,
		cl::Buffer &rBuff, cl::Buffer &hBuff, const u_int rayCount,
		const VECTOR_CLASS<cl::Event> *events, cl::Event *event);

protected:
	// QBVH fields
	cl::Buffer *trisBuff;
	cl::Buffer *qbvhBuff;
};

class OpenCLQBVHImageKernels : public OpenCLKernels {
public:
	OpenCLQBVHImageKernels(OpenCLIntersectionDevice *dev, const u_int kernelCount, u_int s) :
		OpenCLKernels(dev, kernelCount), trisBuff(NULL), qbvhBuff(NULL) {
		stackSize = s;

		const Context *deviceContext = device->GetContext();
		const std::string &deviceName(device->GetName());
		cl::Context &oclContext = device->GetOpenCLContext();
		cl::Device &oclDevice = device->GetOpenCLDevice();

		// Compile the source
		std::stringstream params;
		params << "-D USE_IMAGE_STORAGE -D QBVH_STACK_SIZE=" << stackSize;

		std::string code(
			luxrays::ocl::KernelSource_point_types +
			luxrays::ocl::KernelSource_vector_types +
			luxrays::ocl::KernelSource_ray_types +
			luxrays::ocl::KernelSource_bbox_types);
		code += luxrays::ocl::KernelSource_qbvh;
		cl::Program::Sources source(1, std::make_pair(code.c_str(), code.length()));
		cl::Program program = cl::Program(oclContext, source);
		try {
			VECTOR_CLASS<cl::Device> buildDevice;
			buildDevice.push_back(oclDevice);
			program.build(buildDevice, params.str().c_str());
		} catch (cl::Error err) {
			cl::STRING_CLASS strError = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(oclDevice);
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
				"] QBVH Image Storage compilation error:\n" <<
				strError.c_str());

			throw err;
		}

		for (u_int i = 0; i < kernelCount; ++i) {
			kernels[i] = new cl::Kernel(program, "Intersect");
			kernels[i]->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE,
				&workGroupSize);
			//LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			//	"] QBVH Image Storage kernel work group size: " <<
			//	workGroupSize);

			kernels[i]->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE,
				&workGroupSize);
			//LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			//	"] Suggested work group size: " << workGroupSize);

			if (device->GetDeviceDesc()->GetForceWorkGroupSize() > 0) {
				workGroupSize = device->GetDeviceDesc()->GetForceWorkGroupSize();
				//LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
				//	"] Forced work group size: " << workGroupSize);
			} else if (workGroupSize > 256) {
				// Otherwise I will probably run out of local memory
				workGroupSize = 256;
				//LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
				//	"] Cap work group size to: " << workGroupSize);
			}
		}
	}
	virtual ~OpenCLQBVHImageKernels() { FreeBuffers(); }

	virtual void FreeBuffers();
	void SetBuffers(cl::Image2D *trisBuff, cl::Image2D *qbvhBuff);
	virtual void UpdateDataSet(const DataSet *newDataSet) { assert(false); }
	virtual void EnqueueRayBuffer(cl::CommandQueue &oclQueue, const u_int kernelIndex,
		cl::Buffer &rBuff, cl::Buffer &hBuff, const u_int rayCount,
		const VECTOR_CLASS<cl::Event> *events, cl::Event *event);

protected:
	// QBVH with image storage fields
	cl::Image2D *trisBuff;
	cl::Image2D *qbvhBuff;
};

void OpenCLQBVHKernels::FreeBuffers() {
	BOOST_FOREACH(cl::Kernel *kernel, kernels) {
		delete kernel;
		kernel = NULL;
	}

	device->FreeMemory(trisBuff->getInfo<CL_MEM_SIZE>());
	delete trisBuff;
	trisBuff = NULL;
	device->FreeMemory(qbvhBuff->getInfo<CL_MEM_SIZE>());
	delete qbvhBuff;
	qbvhBuff = NULL;
}

void OpenCLQBVHKernels::SetBuffers(cl::Buffer *t, cl::Buffer *q) {
	trisBuff = t;
	qbvhBuff = q;

	// Set arguments
	BOOST_FOREACH(cl::Kernel *kernel, kernels) {
		kernel->setArg(2, *qbvhBuff);
		kernel->setArg(3, *trisBuff);
		// Check if we have enough local memory
		if (stackSize * workGroupSize * sizeof(cl_int) >
			device->GetOpenCLDevice().getInfo<CL_DEVICE_LOCAL_MEM_SIZE>())
			throw std::runtime_error("Not enough OpenCL device local memory available for the required work group size"
				" and QBVH stack depth (try to reduce the work group size and/or the stack depth)");

		kernel->setArg(5, stackSize * workGroupSize * sizeof(cl_int), NULL);
	}
}

void OpenCLQBVHKernels::EnqueueRayBuffer(cl::CommandQueue &oclQueue, const u_int kernelIndex,
		cl::Buffer &rBuff, cl::Buffer &hBuff, const u_int rayCount,
		const VECTOR_CLASS<cl::Event> *events, cl::Event *event) {
	kernels[kernelIndex]->setArg(0, rBuff);
	kernels[kernelIndex]->setArg(1, hBuff);
	kernels[kernelIndex]->setArg(4, rayCount);
	oclQueue.enqueueNDRangeKernel(*kernels[kernelIndex], cl::NullRange,
		cl::NDRange(rayCount), cl::NDRange(workGroupSize), events,
		event);
}

void OpenCLQBVHImageKernels::FreeBuffers() {
	BOOST_FOREACH(cl::Kernel *kernel, kernels) {
		delete kernel;
		kernel = NULL;
	}

	device->FreeMemory(trisBuff->getInfo<CL_MEM_SIZE>());
	delete trisBuff;
	trisBuff = NULL;
	device->FreeMemory(qbvhBuff->getInfo<CL_MEM_SIZE>());
	delete qbvhBuff;
	qbvhBuff = NULL;
}

void OpenCLQBVHImageKernels::SetBuffers(cl::Image2D *t, cl::Image2D *q) {
	trisBuff = t;
	qbvhBuff = q;

	// Set arguments
	BOOST_FOREACH(cl::Kernel *kernel, kernels) {
		kernel->setArg(2, *qbvhBuff);
		kernel->setArg(3, *trisBuff);
		// Check if we have enough local memory
		if (stackSize * workGroupSize * sizeof(cl_int) >
			device->GetOpenCLDevice().getInfo<CL_DEVICE_LOCAL_MEM_SIZE>())
			throw std::runtime_error("Not enough OpenCL device local memory available for the required work group size"
				" and QBVH stack depth (try to reduce the work group size and/or the stack depth)");
		kernel->setArg(5, stackSize * workGroupSize * sizeof(cl_int), NULL);
	}
}

void OpenCLQBVHImageKernels::EnqueueRayBuffer(cl::CommandQueue &oclQueue, const u_int kernelIndex,
		cl::Buffer &rBuff, cl::Buffer &hBuff, const u_int rayCount,
	const VECTOR_CLASS<cl::Event> *events, cl::Event *event) {
	kernels[kernelIndex]->setArg(0, rBuff);
	kernels[kernelIndex]->setArg(1, hBuff);
	kernels[kernelIndex]->setArg(4, rayCount);
	oclQueue.enqueueNDRangeKernel(*kernels[kernelIndex], cl::NullRange,
		cl::NDRange(rayCount), cl::NDRange(workGroupSize), events,
		event);
}

OpenCLKernels *QBVHAccel::NewOpenCLKernels(OpenCLIntersectionDevice *device,
		const u_int kernelCount, const u_int stackSize, const bool disableImageStorage) const {
	const Context *deviceContext = device->GetContext();
	cl::Context &oclContext = device->GetOpenCLContext();
	const std::string &deviceName(device->GetName());

	LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
		"] QBVH max. stack size: " << stackSize);

	OpenCLDeviceDescription *deviceDesc = device->GetDeviceDesc();
	bool useImage = true;
	size_t nodeWidth, nodeHeight, leafWidth, leafHeight;
	// Check if I can use image to store the data set
	if (disableImageStorage) {
		LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			"] Disable forced for QBVH scene storage inside image");
		useImage = false;
	} else if (!deviceDesc->HasImageSupport() ||
		(!(deviceDesc->GetType() & DEVICE_TYPE_OPENCL_GPU))) {
		LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			"] OpenCL image support is not available");
		useImage = false;
	} else {
		LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			"] OpenCL image support is available");

		// Check if the scene is small enough to be stored inside an image
		const size_t maxWidth = deviceDesc->GetImage2DMaxWidth();
		const size_t maxHeight = deviceDesc->GetImage2DMaxHeight();

		// Calculate the required image size for the storage

		// 7 pixels required for the storage of a QBVH node
		const size_t nodePixelRequired = nNodes * 7;
		nodeWidth = Min(RoundUp(static_cast<u_int>(sqrtf(nodePixelRequired)), 7u),  0x7fffu);
		nodeHeight = nodePixelRequired / nodeWidth +
			(((nodePixelRequired % nodeWidth) == 0) ? 0 : 1);

		// 10 pixels required for the storage of QBVH Triangles
		const size_t leafPixelRequired = nQuads * 10;
		leafWidth = Min(RoundUp(static_cast<u_int>(sqrtf(leafPixelRequired)), 10u), 32760u);
		leafHeight = leafPixelRequired / leafWidth +
			(((leafPixelRequired % leafWidth) == 0) ? 0 : 1);

		LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			"] OpenCL max. image buffer size: " <<
			maxWidth << "x" << maxHeight);

		LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			"] QBVH node image buffer size: " <<
			nodeWidth << "x" << nodeHeight);
		LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			"] QBVH triangle image buffer size: " <<
			leafWidth << "x" << leafHeight);

		if ((nodeWidth > maxWidth) || (nodeHeight > maxHeight) ||
			(leafWidth > maxWidth) || (leafHeight > maxHeight)) {
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
				"] OpenCL image max. image size supported is too small");
			useImage = false;
		} else {
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
				"] Enabled QBVH scene storage inside image");
			useImage = true;
		}
	}
	if (!useImage) {
		// Allocate buffers
		LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			"] QBVH buffer size: " <<
			(sizeof(QBVHNode) * nNodes / 1024) << "Kbytes");
		cl::Buffer *qbvhBuff = new cl::Buffer(oclContext,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(QBVHNode) * nNodes, nodes);
		device->AllocMemory(qbvhBuff->getInfo<CL_MEM_SIZE>());

		LR_LOG(deviceContext, "[OpenCL device::" << deviceName <<
			"] QuadTriangle buffer size: " <<
			(sizeof(QuadTriangle) * nQuads / 1024) << "Kbytes");
		cl::Buffer *trisBuff = new cl::Buffer(oclContext,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(QuadTriangle) * nQuads, prims);
		device->AllocMemory(trisBuff->getInfo<CL_MEM_SIZE>());

		// Setup kernels
		OpenCLQBVHKernels *kernels = new OpenCLQBVHKernels(device, kernelCount, stackSize);
		kernels->SetBuffers(trisBuff, qbvhBuff);

		return kernels;
	} else {
		// Allocate image buffers
		u_int *inodes = new u_int[nodeWidth * nodeHeight * 4];
		for (size_t i = 0; i < nNodes; ++i) {
			u_int *pnodes = (u_int *)(nodes + i);
			const size_t offset = i * 7 * 4;

			for (size_t j = 0; j < 6 * 4; ++j)
				inodes[offset + j] = pnodes[j];

			for (size_t j = 0; j < 4; ++j) {
				int index = nodes[i].children[j];

				if (QBVHNode::IsEmpty(index)) {
					inodes[offset + 6 * 4 + j] = index;
				} else if (QBVHNode::IsLeaf(index)) {
					int32_t count = QBVHNode::FirstQuadIndex(index) * 10;
					// "/ 10" in order to not waste bits
					const unsigned short x = static_cast<unsigned short>((count % leafWidth) / 10);
					const unsigned short y = static_cast<unsigned short>(count / leafWidth);
					((int32_t *)inodes)[offset + 6 * 4 + j] =  0x80000000 |
						(((static_cast<int32_t>(QBVHNode::NbQuadPrimitives(index)) - 1) & 0xf) << 27) |
						(static_cast<int32_t>((x << 16) | y) & 0x07ffffff);
				} else {
					index *= 7;
					// "/ 7" in order to not waste bits
					const unsigned short x = static_cast<unsigned short>((index % nodeWidth) / 7);
					const unsigned short y = static_cast<unsigned short>(index / nodeWidth);
					inodes[offset + 6 * 4 + j] = (x << 16) | y;
				}
			}
		}
		cl::Image2D *qbvhBuff = new cl::Image2D(oclContext,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			cl::ImageFormat(CL_RGBA, CL_UNSIGNED_INT32),
			nodeWidth, nodeHeight, 0, inodes);
		device->AllocMemory(qbvhBuff->getInfo<CL_MEM_SIZE>());
		delete[] inodes;

		u_int *iprims = new u_int[leafWidth * leafHeight * 4];
		memcpy(iprims, prims, sizeof(QuadTriangle) * nQuads);
		cl::Image2D *trisBuff = new cl::Image2D(oclContext,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			cl::ImageFormat(CL_RGBA, CL_UNSIGNED_INT32),
			leafWidth, leafHeight, 0, iprims);
		device->AllocMemory(trisBuff->getInfo<CL_MEM_SIZE>());
		delete[] iprims;

		// Setup kernels
		OpenCLQBVHImageKernels *kernels = new OpenCLQBVHImageKernels(device, kernelCount, stackSize);
		kernels->SetBuffers(trisBuff, qbvhBuff);

		return kernels;
	}
}

#else

OpenCLKernels *NewOpenCLKernels(OpenCLIntersectionDevice *device, const u_int kernelCount,
		const u_int stackSize, const bool disableImageStorage) const {
	return NULL;
}

#endif

QBVHAccel::QBVHAccel(const Context *context,
		u_int mp, u_int fst, u_int sf) : fullSweepThreshold(fst),
		skipFactor(sf), maxPrimsPerLeaf(mp), ctx(context) {
	initialized = false;
	preprocessedMesh = NULL;
	mesh = NULL;
	meshIDs = NULL;
	meshTriangleIDs = NULL;
	maxDepth = 0;
}

QBVHAccel::~QBVHAccel() {
	if (initialized) {
		FreeAligned(prims);
		FreeAligned(nodes);

		if (preprocessedMesh) {
			preprocessedMesh->Delete();
			delete preprocessedMesh;
		}
		delete[] meshIDs;
		delete[] meshTriangleIDs;
	}
}

void QBVHAccel::Init(const std::deque<const Mesh *> &meshes, const u_int totalVertexCount,
		const u_int totalTriangleCount) {
	assert (!initialized);

	preprocessedMesh = TriangleMesh::Merge(totalVertexCount, totalTriangleCount,
			meshes, &meshIDs, &meshTriangleIDs);
	assert (preprocessedMesh->GetTotalVertexCount() == totalVertexCount);
	assert (preprocessedMesh->GetTotalTriangleCount() == totalTriangleCount);

	LR_LOG(ctx, "Total vertices memory usage: " << totalVertexCount * sizeof(Point) / 1024 << "Kbytes");
	LR_LOG(ctx, "Total triangles memory usage: " << totalTriangleCount * sizeof(Triangle) / 1024 << "Kbytes");

	Init(preprocessedMesh);
}

void QBVHAccel::Init(const Mesh *m) {
	assert (!initialized);

	mesh = m;
	const u_int totalTriangleCount = mesh->GetTotalTriangleCount();

	// Temporary data for building
	u_int *primsIndexes = new u_int[totalTriangleCount + 3]; // For the case where
	// the last quad would begin at the last primitive
	// (or the second or third last primitive)

	// The number of nodes depends on the number of primitives,
	// and is bounded by 2 * nPrims - 1.
	// Even if there will normally have at least 4 primitives per leaf,
	// it is not always the case => continue to use the normal bounds.
	nNodes = 0;
	maxNodes = 1;
	for (u_int layer = ((totalTriangleCount + maxPrimsPerLeaf - 1) / maxPrimsPerLeaf + 3) / 4; layer != 1; layer = (layer + 3) / 4)
		maxNodes += layer;
	nodes = AllocAligned<QBVHNode>(maxNodes);
	for (u_int i = 0; i < maxNodes; ++i)
		nodes[i] = QBVHNode();

	// The arrays that will contain
	// - the bounding boxes for all triangles
	// - the centroids for all triangles
	BBox *primsBboxes = new BBox[totalTriangleCount];
	Point *primsCentroids = new Point[totalTriangleCount];
	// The bouding volume of all the centroids
	BBox centroidsBbox;

	const Point *verts = mesh->GetVertices();
	const Triangle *triangles = mesh->GetTriangles();

	// Fill each base array
	for (u_int i = 0; i < totalTriangleCount; ++i) {
		// This array will be reorganized during construction.
		primsIndexes[i] = i;

		// Compute the bounding box for the triangle
		primsBboxes[i] = triangles[i].WorldBound(verts);
		primsBboxes[i].Expand(MachineEpsilon::E(primsBboxes[i]));
		primsCentroids[i] = (primsBboxes[i].pMin + primsBboxes[i].pMax) * .5f;

		// Update the global bounding boxes
		worldBound = Union(worldBound, primsBboxes[i]);
		centroidsBbox = Union(centroidsBbox, primsCentroids[i]);
	}

	// Arbitrarily take the last primitive for the last 3
	primsIndexes[totalTriangleCount] = totalTriangleCount - 1;
	primsIndexes[totalTriangleCount + 1] = totalTriangleCount - 1;
	primsIndexes[totalTriangleCount + 2] = totalTriangleCount - 1;

	// Recursively build the tree
	LR_LOG(ctx, "Building QBVH, primitives: " << totalTriangleCount << ", initial nodes: " << maxNodes);

	nQuads = 0;
	BuildTree(0, totalTriangleCount, primsIndexes, primsBboxes, primsCentroids,
			worldBound, centroidsBbox, -1, 0, 0);

	prims = AllocAligned<QuadTriangle>(nQuads);
	nQuads = 0;
	PreSwizzle(0, primsIndexes);

	LR_LOG(ctx, "QBVH completed with " << nNodes << "/" << maxNodes << " nodes");
	LR_LOG(ctx, "Total QBVH memory usage: " << nNodes * sizeof(QBVHNode) / 1024 << "Kbytes");
	LR_LOG(ctx, "Total QBVH QuadTriangle count: " << nQuads);
	LR_LOG(ctx, "Max. QBVH Depth: " << maxDepth);

	// Release temporary memory
	delete[] primsBboxes;
	delete[] primsCentroids;
	delete[] primsIndexes;

	initialized = true;
}

/***************************************************/

void QBVHAccel::BuildTree(u_int start, u_int end, u_int *primsIndexes,
		BBox *primsBboxes, Point *primsCentroids, const BBox &nodeBbox,
		const BBox &centroidsBbox, int32_t parentIndex, int32_t childIndex, int depth) {
	maxDepth = (depth >= maxDepth) ? depth : maxDepth; // Set depth so we know how much stack we need later.

	// Create a leaf ?
	//********
	if (depth > 64 || end - start <= maxPrimsPerLeaf) {
		if (depth > 64) {
			LR_LOG(ctx, "Maximum recursion depth reached while constructing QBVH, forcing a leaf node");
			if (end - start > 64) {
				LR_LOG(ctx, "QBVH unable to handle geometry, too many primitives in leaf");
			}
		}
		CreateTempLeaf(parentIndex, childIndex, start, end, nodeBbox);
		return;
	}

	int32_t currentNode = parentIndex;
	int32_t leftChildIndex = childIndex;
	int32_t rightChildIndex = childIndex + 1;

	// Number of primitives in each bin
	int bins[NB_BINS];
	// Bbox of the primitives in the bin
	BBox binsBbox[NB_BINS];

	//--------------
	// Fill in the bins, considering all the primitives when a given
	// threshold is reached, else considering only a portion of the
	// primitives for the binned-SAH process. Also compute the bins bboxes
	// for the primitives. 

	for (u_int i = 0; i < NB_BINS; ++i)
		bins[i] = 0;

	u_int step = (end - start < fullSweepThreshold) ? 1 : skipFactor;

	// Choose the split axis, taking the axis of maximum extent for the
	// centroids (else weird cases can occur, where the maximum extent axis
	// for the nodeBbox is an axis of 0 extent for the centroids one.).
	const int axis = centroidsBbox.MaximumExtent();

	// Precompute values that are constant with respect to the current
	// primitive considered.
	const float k0 = centroidsBbox.pMin[axis];
	const float k1 = NB_BINS / (centroidsBbox.pMax[axis] - k0);

	// If the bbox is a point, create a leaf, hoping there are not more
	// than 64 primitives that share the same center.
	if (k1 == INFINITY) {
		if (end - start > 64)
			LR_LOG(ctx, "QBVH unable to handle geometry, too many primitives with the same centroid");
		CreateTempLeaf(parentIndex, childIndex, start, end, nodeBbox);
		return;
	}

	// Create an intermediate node if the depth indicates to do so.
	// Register the split axis.
	if (depth % 2 == 0) {
		currentNode = CreateIntermediateNode(parentIndex, childIndex, nodeBbox);
		leftChildIndex = 0;
		rightChildIndex = 2;
	}

	for (u_int i = start; i < end; i += step) {
		u_int primIndex = primsIndexes[i];

		// Binning is relative to the centroids bbox and to the
		// primitives' centroid.
		const int binId = Min(NB_BINS - 1, Floor2Int(k1 * (primsCentroids[primIndex][axis] - k0)));

		bins[binId]++;
		binsBbox[binId] = Union(binsBbox[binId], primsBboxes[primIndex]);
	}

	//--------------
	// Evaluate where to split.

	// Cumulative number of primitives in the bins from the first to the
	// ith, and from the last to the ith.
	int nbPrimsLeft[NB_BINS];
	int nbPrimsRight[NB_BINS];
	// The corresponding cumulative bounding boxes.
	BBox bboxesLeft[NB_BINS];
	BBox bboxesRight[NB_BINS];

	// The corresponding volumes.
	float vLeft[NB_BINS];
	float vRight[NB_BINS];

	BBox currentBboxLeft, currentBboxRight;
	int currentNbLeft = 0, currentNbRight = 0;

	for (int i = 0; i < NB_BINS; ++i) {
		//-----
		// Left side
		// Number of prims
		currentNbLeft += bins[i];
		nbPrimsLeft[i] = currentNbLeft;
		// Prims bbox
		currentBboxLeft = Union(currentBboxLeft, binsBbox[i]);
		bboxesLeft[i] = currentBboxLeft;
		// Surface area
		vLeft[i] = currentBboxLeft.SurfaceArea();

		//-----
		// Right side
		// Number of prims
		int rightIndex = NB_BINS - 1 - i;
		currentNbRight += bins[rightIndex];
		nbPrimsRight[rightIndex] = currentNbRight;
		// Prims bbox
		currentBboxRight = Union(currentBboxRight, binsBbox[rightIndex]);
		bboxesRight[rightIndex] = currentBboxRight;
		// Surface area
		vRight[rightIndex] = currentBboxRight.SurfaceArea();
	}

	int minBin = -1;
	float minCost = INFINITY;
	// Find the best split axis,
	// there must be at least a bin on the right side
	for (int i = 0; i < NB_BINS - 1; ++i) {
		float cost = vLeft[i] * nbPrimsLeft[i] +
				vRight[i + 1] * nbPrimsRight[i + 1];
		if (cost < minCost) {
			minBin = i;
			minCost = cost;
		}
	}

	//-----------------
	// Make the partition, in a "quicksort partitioning" way,
	// the pivot being the position of the split plane
	// (no more binId computation)
	// track also the bboxes (primitives and centroids)
	// for the left and right halves.

	// The split plane coordinate is the coordinate of the end of
	// the chosen bin along the split axis
	float splitPos = centroidsBbox.pMin[axis] + (minBin + 1) *
			(centroidsBbox.pMax[axis] - centroidsBbox.pMin[axis]) / NB_BINS;


	BBox leftChildBbox, rightChildBbox;
	BBox leftChildCentroidsBbox, rightChildCentroidsBbox;

	u_int storeIndex = start;
	for (u_int i = start; i < end; ++i) {
		u_int primIndex = primsIndexes[i];

		if (primsCentroids[primIndex][axis] <= splitPos) {
			// Swap
			primsIndexes[i] = primsIndexes[storeIndex];
			primsIndexes[storeIndex] = primIndex;
			++storeIndex;

			// Update the bounding boxes,
			// this triangle is on the left side
			leftChildBbox = Union(leftChildBbox, primsBboxes[primIndex]);
			leftChildCentroidsBbox = Union(leftChildCentroidsBbox, primsCentroids[primIndex]);
		} else {
			// Update the bounding boxes,
			// this triangle is on the right side.
			rightChildBbox = Union(rightChildBbox, primsBboxes[primIndex]);
			rightChildCentroidsBbox = Union(rightChildCentroidsBbox, primsCentroids[primIndex]);
		}
	}

	// Build recursively
	BuildTree(start, storeIndex, primsIndexes, primsBboxes, primsCentroids,
			leftChildBbox, leftChildCentroidsBbox, currentNode,
			leftChildIndex, depth + 1);
	BuildTree(storeIndex, end, primsIndexes, primsBboxes, primsCentroids,
			rightChildBbox, rightChildCentroidsBbox, currentNode,
			rightChildIndex, depth + 1);
}

/***************************************************/

void QBVHAccel::CreateTempLeaf(int32_t parentIndex, int32_t childIndex,
		u_int start, u_int end, const BBox &nodeBbox) {
	// The leaf is directly encoded in the intermediate node.
	if (parentIndex < 0) {
		// The entire tree is a leaf
		nNodes = 1;
		parentIndex = 0;
	}

	// Encode the leaf in the original way,
	// it will be transformed to a preswizzled format in a post-process.

	u_int nbPrimsTotal = end - start;

	QBVHNode &node = nodes[parentIndex];

	node.SetBBox(childIndex, nodeBbox);


	// Next multiple of 4, divided by 4
	u_int quads = (nbPrimsTotal + 3) / 4;

	// Use the same encoding as the final one, but with a different meaning.
	node.InitializeLeaf(childIndex, quads, start);

	nQuads += quads;
}

void QBVHAccel::PreSwizzle(int32_t nodeIndex, u_int *primsIndexes) {
	for (int i = 0; i < 4; ++i) {
		if (nodes[nodeIndex].ChildIsLeaf(i))
			CreateSwizzledLeaf(nodeIndex, i, primsIndexes);
		else
			PreSwizzle(nodes[nodeIndex].children[i], primsIndexes);
	}
}

void QBVHAccel::CreateSwizzledLeaf(int32_t parentIndex, int32_t childIndex,
		u_int *primsIndexes) {
	QBVHNode &node = nodes[parentIndex];
	if (node.LeafIsEmpty(childIndex))
		return;
	const u_int startQuad = nQuads;
	const u_int nbQuads = node.NbQuadsInLeaf(childIndex);

	u_int primOffset = node.FirstQuadIndexForLeaf(childIndex);
	u_int primNum = nQuads;

	const Point *vertices = mesh->GetVertices();
	const Triangle *triangles = mesh->GetTriangles();

	for (u_int q = 0; q < nbQuads; ++q) {
		new (&prims[primNum]) QuadTriangle(triangles, vertices, primsIndexes[primOffset],
				primsIndexes[primOffset + 1], primsIndexes[primOffset + 2], primsIndexes[primOffset + 3]);

		++primNum;
		primOffset += 4;
	}
	nQuads += nbQuads;
	node.InitializeLeaf(childIndex, nbQuads, startQuad);
}

/***************************************************/

bool QBVHAccel::Intersect(const Ray *initialRay, RayHit *rayHit) const {
	Ray ray(*initialRay);
	rayHit->SetMiss();

	//------------------------------
	// Prepare the ray for intersection
	QuadRay ray4(ray);
	__m128 invDir[3];
	invDir[0] = _mm_set1_ps(1.f / ray.d.x);
	invDir[1] = _mm_set1_ps(1.f / ray.d.y);
	invDir[2] = _mm_set1_ps(1.f / ray.d.z);

	int signs[3];
	ray.GetDirectionSigns(signs);

	//------------------------------
	// Main loop
	int todoNode = 0; // the index in the stack
	int32_t nodeStack[64];
	nodeStack[0] = 0; // first node to handle: root node

	while (todoNode >= 0) {
		// Leaves are identified by a negative index
		if (!QBVHNode::IsLeaf(nodeStack[todoNode])) {
			QBVHNode &node = nodes[nodeStack[todoNode]];
			--todoNode;

			// It is quite strange but checking here for empty nodes slows down the rendering
			const int32_t visit = node.BBoxIntersect(ray4, invDir, signs);

			switch (visit) {
				case (0x1 | 0x0 | 0x0 | 0x0):
					nodeStack[++todoNode] = node.children[0];
					break;
				case (0x0 | 0x2 | 0x0 | 0x0):
					nodeStack[++todoNode] = node.children[1];
					break;
				case (0x1 | 0x2 | 0x0 | 0x0):
					nodeStack[++todoNode] = node.children[0];
					nodeStack[++todoNode] = node.children[1];
					break;
				case (0x0 | 0x0 | 0x4 | 0x0):
					nodeStack[++todoNode] = node.children[2];
					break;
				case (0x1 | 0x0 | 0x4 | 0x0):
					nodeStack[++todoNode] = node.children[0];
					nodeStack[++todoNode] = node.children[2];
					break;
				case (0x0 | 0x2 | 0x4 | 0x0):
					nodeStack[++todoNode] = node.children[1];
					nodeStack[++todoNode] = node.children[2];
					break;
				case (0x1 | 0x2 | 0x4 | 0x0):
					nodeStack[++todoNode] = node.children[0];
					nodeStack[++todoNode] = node.children[1];
					nodeStack[++todoNode] = node.children[2];
					break;
				case (0x0 | 0x0 | 0x0 | 0x8):
					nodeStack[++todoNode] = node.children[3];
					break;
				case (0x1 | 0x0 | 0x0 | 0x8):
					nodeStack[++todoNode] = node.children[0];
					nodeStack[++todoNode] = node.children[3];
					break;
				case (0x0 | 0x2 | 0x0 | 0x8):
					nodeStack[++todoNode] = node.children[1];
					nodeStack[++todoNode] = node.children[3];
					break;
				case (0x1 | 0x2 | 0x0 | 0x8):
					nodeStack[++todoNode] = node.children[0];
					nodeStack[++todoNode] = node.children[1];
					nodeStack[++todoNode] = node.children[3];
					break;
				case (0x0 | 0x0 | 0x4 | 0x8):
					nodeStack[++todoNode] = node.children[2];
					nodeStack[++todoNode] = node.children[3];
					break;
				case (0x1 | 0x0 | 0x4 | 0x8):
					nodeStack[++todoNode] = node.children[0];
					nodeStack[++todoNode] = node.children[2];
					nodeStack[++todoNode] = node.children[3];
					break;
				case (0x0 | 0x2 | 0x4 | 0x8):
					nodeStack[++todoNode] = node.children[1];
					nodeStack[++todoNode] = node.children[2];
					nodeStack[++todoNode] = node.children[3];
					break;
				case (0x1 | 0x2 | 0x4 | 0x8):
					nodeStack[++todoNode] = node.children[0];
					nodeStack[++todoNode] = node.children[1];
					nodeStack[++todoNode] = node.children[2];
					nodeStack[++todoNode] = node.children[3];
					break;
			}
		} else {
			//----------------------
			// It is a leaf,
			// all the informations are encoded in the index
			const int32_t leafData = nodeStack[todoNode];
			--todoNode;

			if (QBVHNode::IsEmpty(leafData))
				continue;

			// Perform intersection
			const u_int nbQuadPrimitives = QBVHNode::NbQuadPrimitives(leafData);

			const u_int offset = QBVHNode::FirstQuadIndex(leafData);

			for (u_int primNumber = offset; primNumber < (offset + nbQuadPrimitives); ++primNumber)
				prims[primNumber].Intersect(ray4, ray, rayHit);
		}//end of the else
	}

	return !rayHit->Miss();
}

}