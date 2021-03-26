/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#pragma once

#include <SceneAPI/SceneCore/Containers/Scene.h>

namespace AZ::SceneAPI::DataTypes { class IMeshData; }
namespace AZ::SceneAPI::DataTypes { class IMeshVertexUVData; }
namespace AZ::SceneAPI::DataTypes { class IMeshVertexTangentData; }
namespace AZ::SceneAPI::DataTypes { class IMeshVertexBitangentData; }

namespace AZ::TangentGeneration::MikkT
{
    struct MikktCustomData
    {
        AZ::SceneAPI::DataTypes::IMeshData*                 m_meshData;
        AZ::SceneAPI::DataTypes::IMeshVertexUVData*         m_uvData;
        AZ::SceneAPI::DataTypes::IMeshVertexTangentData*    m_tangentData;
        AZ::SceneAPI::DataTypes::IMeshVertexBitangentData*  m_bitangentData;
    };

    // The main generation method.
    bool GenerateTangents(AZ::SceneAPI::Containers::SceneManifest& manifest, AZ::SceneAPI::Containers::SceneGraph& graph, const AZ::SceneAPI::Containers::SceneGraph::NodeIndex& nodeIndex, AZ::SceneAPI::DataTypes::IMeshData* meshData, size_t uvSet);
} // namespace AZ::TangentGeneration::MikkT