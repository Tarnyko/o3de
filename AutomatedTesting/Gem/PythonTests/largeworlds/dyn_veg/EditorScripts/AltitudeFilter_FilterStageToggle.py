"""
All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
its licensors.

For complete copyright and license terms please see the LICENSE at the root of this
distribution (the "License"). All use of this software is governed by the License,
or, if provided, by the license below or the license accompanying this file. Do not
remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
"""

import os
import sys
import azlmbr.math as math
import azlmbr.paths

sys.path.append(os.path.join(azlmbr.paths.devroot, "AutomatedTesting", "Gem", "PythonTests"))
import automatedtesting_shared.hydra_editor_utils as hydra
from automatedtesting_shared.editor_test_helper import EditorTestHelper
from largeworlds.large_worlds_utils import editor_dynveg_test_helper as dynveg


class TestAltitudeFilterFilterStageToggle(EditorTestHelper):
    def __init__(self):
        EditorTestHelper.__init__(self, log_prefix="AltitudeFilter_FilterStageToggle", args=["level"])

    def run_test(self):
        """
        Summary:
        Filter Stage toggle affects final vegetation position

        Expected Result:
        Vegetation instances plant differently depending on the Filter Stage setting.
        PostProcess should cause some number of plants that appear above and below the desired altitude range to disappear.

        :return: None
        """

        PREPROCESS_INSTANCE_COUNT = 16
        POSTPROCESS_INSTANCE_COUNT = 13

        # Create empty level
        self.test_success = self.create_level(
            self.args["level"],
            heightmap_resolution=1024,
            heightmap_meters_per_pixel=1,
            terrain_texture_resolution=4096,
            use_terrain=False,
        )

        # Create basic vegetation entity
        position = math.Vector3(512.0, 512.0, 32.0)
        asset_path = os.path.join("Slices", "PinkFlower.dynamicslice")
        vegetation = dynveg.create_vegetation_area("vegetation", position, 16.0, 16.0, 16.0, asset_path)

        # Add a Vegetation Altitude Filter to the vegetation area entity
        vegetation.add_component("Vegetation Altitude Filter")

        # Create Surface for instances to plant on
        dynveg.create_surface_entity("Surface_Entity_Parent", position, 16.0, 16.0, 1.0)

        # Add entity with Mesh to replicate creation of hills
        dynveg.create_mesh_surface_entity_with_slopes("hill", position, 40.0, 40.0, 40.0)

        # Increase Box Shape size to encompass the hills
        vegetation.get_set_test(1, "Box Shape|Box Configuration|Dimensions", math.Vector3(100.0, 100.0, 100.0))

        # Set a Min Altitude of 38 and Max of 40 in Vegetation Altitude Filter
        vegetation.get_set_test(3, "Configuration|Altitude Min", 38.0)
        vegetation.get_set_test(3, "Configuration|Altitude Max", 40.0)

        # Create a new entity as a child of the vegetation area entity with Random Noise Gradient Generator, Gradient Transform Modifier,
        # and Box Shape component
        random_noise = hydra.Entity("random_noise")
        random_noise.create_entity(position, ["Random Noise Gradient", "Gradient Transform Modifier", "Box Shape"])
        random_noise.set_test_parent_entity(vegetation)

        # Add a Vegetation Position Modifier to the vegetation area entity.
        vegetation.add_component("Vegetation Position Modifier")

        # Pin the Random Noise entity to the Gradient Entity Id field of the Position Modifier's Gradient X
        vegetation.get_set_test(4, "Configuration|Position X|Gradient|Gradient Entity Id", random_noise.id)

        # Toggle between PreProcess and PostProcess in Vegetation Altitude Filter
        vegetation.get_set_test(3, "Configuration|Filter Stage", 1)
        result = self.wait_for_condition(lambda: dynveg.validate_instance_count(position, 30.0, PREPROCESS_INSTANCE_COUNT), 2.0)
        self.log(f"Vegetation instances count equal to expected value for PREPROCESS filter stage: {result}")
        vegetation.get_set_test(3, "Configuration|Filter Stage", 2)
        result = self.wait_for_condition(lambda: dynveg.validate_instance_count(position, 30.0, POSTPROCESS_INSTANCE_COUNT), 2.0)
        self.log(f"Vegetation instances count equal to expected value for POSTPROCESS filter stage: {result}")


test = TestAltitudeFilterFilterStageToggle()
test.run()