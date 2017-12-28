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

#include "luxrays/utils/serializationutils.h"

using namespace std;
using namespace luxrays;

//------------------------------------------------------------------------------
// SerializationOuputFile
//------------------------------------------------------------------------------

SerializationOuputFile::SerializationOuputFile(const std::string &fileName) :
		outArchive(NULL) {
	outFile.exceptions(BOOST_IFSTREAM::failbit | BOOST_IFSTREAM::badbit | BOOST_IFSTREAM::eofbit);
	outFile.open(fileName.c_str(), BOOST_OFSTREAM::binary | BOOST_OFSTREAM::trunc);

	// Enable compression
	outStream.push(boost::iostreams::gzip_compressor(4));
	outStream.push(outFile);

	// Use portable archive
	outArchive = new eos::polymorphic_portable_oarchive(outStream);
}

SerializationOuputFile::~SerializationOuputFile() {
	delete outArchive;
}

eos::polymorphic_portable_oarchive &SerializationOuputFile::GetArchive() {
	return *outArchive;
}

bool SerializationOuputFile::IsGood() {
	return outStream.good();
}

void SerializationOuputFile::Flush() {
	outStream.flush();
}

std::streampos SerializationOuputFile::GetPosition() {
	return outFile.tellp();
}

//------------------------------------------------------------------------------
// SerializationInputFile
//------------------------------------------------------------------------------

SerializationInputFile::SerializationInputFile(const std::string &fileName) :
		inArchive(NULL) {
	inFile.exceptions(BOOST_IFSTREAM::failbit | BOOST_IFSTREAM::badbit | BOOST_IFSTREAM::eofbit);
	inFile.open(fileName.c_str(), BOOST_IFSTREAM::binary);

	// Enable compression
	inStream.push(boost::iostreams::gzip_decompressor());
	inStream.push(inFile);

	// Use portable archive
	inArchive = new eos::polymorphic_portable_iarchive(inStream);
	
}

SerializationInputFile::~SerializationInputFile() {
	delete inArchive;
}

eos::polymorphic_portable_iarchive &SerializationInputFile::GetArchive() {
	return *inArchive;
}

bool SerializationInputFile::IsGood() {
	return inStream.good();
}
