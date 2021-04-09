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

#include <AzToolsFramework/Thumbnails/Thumbnail.h>
#include <AzToolsFramework/Thumbnails/ThumbnailerBus.h>
#include <Thumbnails/Rendering/ThumbnailRendererContext.h>
#include <Thumbnails/Rendering/ThumbnailRendererData.h>

// Disables warning messages triggered by the Qt library
// 4251: class needs to have dll-interface to be used by clients of class 
// 4800: forcing value to bool 'true' or 'false' (performance warning)
AZ_PUSH_DISABLE_WARNING(4251 4800, "-Wunknown-warning-option")
#include <QPixmap>
AZ_POP_DISABLE_WARNING

namespace AZ
{
    namespace LyIntegration
    {
        namespace Thumbnails
        {
            class ThumbnailRendererStep;

            //! Provides custom rendering of material and model thumbnails
            class CommonThumbnailRenderer
                : private AzToolsFramework::Thumbnailer::ThumbnailerRendererRequestBus::MultiHandler
                , private SystemTickBus::Handler
                , public ThumbnailRendererContext
            {
            public:
                AZ_CLASS_ALLOCATOR(CommonThumbnailRenderer, AZ::SystemAllocator, 0)

                CommonThumbnailRenderer();
                ~CommonThumbnailRenderer();

                //! ThumbnailRendererContext overrides...
                void SetStep(Step step) override;
                Step GetStep() const override;
                AZStd::shared_ptr<ThumbnailRendererData> GetData() const override;

            private:
                //! ThumbnailerRendererRequestsBus::Handler interface overrides...
                void RenderThumbnail(AzToolsFramework::Thumbnailer::SharedThumbnailKey thumbnailKey, int thumbnailSize) override;
                bool Installed() const override;

                //! SystemTickBus::Handler interface overrides...
                void OnSystemTick() override;

                AZStd::unordered_map<Step, AZStd::shared_ptr<ThumbnailRendererStep>> m_steps;
                Step m_currentStep = Step::None;
                AZStd::shared_ptr<ThumbnailRendererData> m_data;
            };
        } // namespace Thumbnails
    } // namespace LyIntegration
} // namespace AZ