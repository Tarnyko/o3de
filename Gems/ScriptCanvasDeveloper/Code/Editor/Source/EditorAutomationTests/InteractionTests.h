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

#include <QLineEdit>
#include <QMenu>
#include <QString>
#include <QTableView>

#include <GraphCanvas/Components/Nodes/NodeBus.h>
#include <GraphCanvas/Components/SceneBus.h>
#include <GraphCanvas/Components/Slots/SlotBus.h>
#include <GraphCanvas/Components/VisualBus.h>
#include <GraphCanvas/Utils/ConversionUtils.h>

#include <ScriptCanvas/Bus/RequestBus.h>

#include <ScriptCanvasDeveloperEditor/EditorAutomation/EditorAutomationTest.h>

#include <ScriptCanvasDeveloperEditor/EditorAutomation/EditorAutomationStates/CreateElementsStates.h>

namespace ScriptCanvasDeveloper
{
    /**
        EditorautomationTest that will test out the AltClick to delete elements functionality for nodes, connected nodes, and connections
    */
    class AltClickDeleteTest
        : public EditorAutomationTest
        , public GraphCanvas::SceneNotificationBus::Handler
    {
    public:
        AltClickDeleteTest();
        ~AltClickDeleteTest() override = default;
    };

    /**
        EditorautomationTest that will test out the cut/copy/paste functions
    */
    class CutCopyPasteDuplicateTest
        : public EditorAutomationTest
        , public GraphCanvas::SceneNotificationBus::Handler
    {
        class CheckpointState
            : public CustomActionState
        {
        public:
            CheckpointState(AZStd::string checkpoint);
            ~CheckpointState() override = default;

            void OnCustomAction() override {}
        };

    public:
        CutCopyPasteDuplicateTest(QString nodeName);
        ~CutCopyPasteDuplicateTest() override = default;

        // SceneNotificationBus
        void OnNodeAdded(const AZ::EntityId& nodeId, bool isPaste) override;
        void OnNodeRemoved(const AZ::EntityId& nodeId) override;
        ///

        // EditorAutomationTests
        void OnStateComplete(int stateId) override;
        ////

    private:

        void ProcessCreationSet();

        AutomationStateModelId m_originalNodeId;

        GraphCanvas::NodeId   m_removalTarget;
        AZStd::unordered_set< GraphCanvas::NodeId > m_createdSet;

        CreateNodeFromContextMenuState* m_createNodeState = nullptr;

        CheckpointState* m_cutPasteCheckpoint = nullptr;
        CheckpointState* m_copyPasteCheckpoint = nullptr;
        CheckpointState* m_copyPasteCopyCheckpoint = nullptr;
        CheckpointState* m_duplicateCheckpoint = nullptr;
    };
}