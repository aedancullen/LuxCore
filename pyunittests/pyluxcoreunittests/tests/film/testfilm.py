# -*- coding: utf-8 -*-
################################################################################
# Copyright 1998-2018 by authors (see AUTHORS.txt)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

import time
import unittest
import pyluxcore

from pyluxcoreunittests.tests.utils import *
from pyluxcoreunittests.tests.imagetest import *

class TestFilm(unittest.TestCase):
	def test_Film_ConvTest(self):
		# Load the configuration from file
		props = pyluxcore.Properties("resources/scenes/simple/simple.cfg")

		# Change the render engine to PATHCPU
		props.Set(pyluxcore.Property("renderengine.type", ["PATHCPU"]))
		props.Set(pyluxcore.Property("sampler.type", ["RANDOM"]))
		props.Set(GetDefaultEngineProperties("PATHCPU"))

		# Replace halt condition
		props.Delete("batch.haltdebug")
		# Run at full speed
		props.Delete("native.threads.count")
		props.Set(pyluxcore.Property("batch.haltthreshold", 0.075))
		props.Set(pyluxcore.Property("batch.haltthreshold.step", 16))

		config = pyluxcore.RenderConfig(props)
		session = pyluxcore.RenderSession(config)

		session.Start()
		while True:
			time.sleep(0.5)

			# Update statistics (and run the convergence test)
			session.UpdateStats()

			if session.HasDone():
				# Time to stop the rendering
				break
		session.Stop()

		image = GetImagePipelineImage(session.GetFilm())

		CheckResult(self, image, "Film_ConvTest", False)
