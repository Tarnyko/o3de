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

#include <AzToolsFramework/UI/Prefab/PrefabUiHandler.h>

#include <AzToolsFramework/UI/Outliner/EntityOutlinerListModel.hxx>

#include <QAbstractItemModel>
#include <QPainter>
#include <QPainterPath>
#include <QTreeView>

namespace AzToolsFramework
{
    const QColor PrefabUiHandler::m_prefabCapsuleColor = QColor("#1E252F");
    const QColor PrefabUiHandler::m_prefabCapsuleEditColor = QColor("#4A90E2");

    PrefabUiHandler::PrefabUiHandler()
    {
        m_prefabEditInterface = AZ::Interface<Prefab::PrefabEditInterface>::Get();

        if (m_prefabEditInterface == nullptr)
        {
            AZ_Assert(false, "PrefabUiHandler - could not get PrefabEditInterface on PrefabUiHandler construction.");
            return;
        }
    }

    void PrefabUiHandler::PaintItemBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        if (!painter)
        {
            AZ_Warning("PrefabUiHandler", false, "PrefabUiHandler - painter is nullptr, can't draw Prefab outliner background.");
            return;
        }

        AZ::EntityId entityId(index.data(EntityOutlinerListModel::EntityIdRole).value<AZ::u64>());
        const bool isFirstColumn = index.column() == EntityOutlinerListModel::ColumnName;
        const bool isLastColumn = index.column() == EntityOutlinerListModel::ColumnLockToggle;
        const bool hasVisibleChildren = index.data(EntityOutlinerListModel::ExpandedRole).value<bool>() && index.model()->hasChildren(index);

        QColor backgroundColor = m_prefabCapsuleColor;
        if (m_prefabEditInterface->IsOwningPrefabBeingEdited(entityId))
        {
            backgroundColor = m_prefabCapsuleEditColor;
        }

        QPainterPath backgroundPath;
        backgroundPath.setFillRule(Qt::WindingFill);

        if (isFirstColumn || isLastColumn)
        {
            // Rounded rect to have rounded borders on top
            backgroundPath.addRoundedRect(option.rect, m_prefabCapsuleRadius, m_prefabCapsuleRadius);

            if (hasVisibleChildren)
            {
                // Regular rect, half height, to square the bottom borders
                QRect bottomRect = option.rect;
                bottomRect.setTop(bottomRect.top() + (bottomRect.height() / 2));
                backgroundPath.addRect(bottomRect);
            }
            
            // Regular rect, half height, to square the opposite border
            QRect squareRect = option.rect;
            if (isFirstColumn)
            {
                squareRect.setLeft(option.rect.left() + (option.rect.width() / 2));
            }
            else if (isLastColumn)
            {
                squareRect.setWidth(option.rect.width() / 2);
            }
            backgroundPath.addRect(squareRect);
        }
        else
        {
            backgroundPath.addRect(option.rect);
        }

        painter->fillPath(backgroundPath.simplified(), backgroundColor);
    }

    void PrefabUiHandler::PaintDescendantForeground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index,
        const QModelIndex& descendantIndex) const
    {
        if (!painter)
        {
            AZ_Warning("PrefabUiHandler", false, "PrefabUiHandler - painter is nullptr, can't draw Prefab outliner background.");
            return;
        }

        AZ::EntityId entityId(index.data(EntityOutlinerListModel::EntityIdRole).value<AZ::u64>());
        const QTreeView* outlinerTreeView(qobject_cast<const QTreeView*>(option.widget));
        const int ancestorLeft = outlinerTreeView->visualRect(index).left() + (m_prefabBorderThickness / 2);
        const int curveRectSize = m_prefabCapsuleRadius * 2;
        const bool isFirstColumn = descendantIndex.column() == EntityOutlinerListModel::ColumnName;
        const bool isLastColumn = descendantIndex.column() == EntityOutlinerListModel::ColumnLockToggle;

        QColor borderColor = m_prefabCapsuleColor;
        if (m_prefabEditInterface->IsOwningPrefabBeingEdited(entityId))
        {
            borderColor = m_prefabCapsuleEditColor;
        }

        QPen borderLinePen(borderColor, m_prefabBorderThickness);

        // Find the rect that extends fully to the left
        QRect fullRect = option.rect;
        fullRect.setLeft(ancestorLeft);
        // Adjust option.rect to account for the border thickness
        QRect rect = option.rect;
        rect.setLeft(rect.left() + (m_prefabBorderThickness / 2));

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(borderLinePen);

        if (IsLastVisibleChild(index, descendantIndex))
        {
            // This is the last visible entity in the prefab, so close the container

            if (isFirstColumn)
            {
                // Left border, curve on the bottom left, bottom border

                // Define curve start, end and size
                QPoint curveStart = fullRect.bottomLeft();
                curveStart.setY(curveStart.y() - m_prefabCapsuleRadius);
                QPoint curveEnd = fullRect.bottomLeft();
                curveEnd.setX(curveEnd.x() + m_prefabCapsuleRadius);
                QRect curveRect = QRect(fullRect.left(), fullRect.bottom() - curveRectSize, curveRectSize, curveRectSize);

                // Curved Corner
                QPainterPath curvedCorner;
                curvedCorner.moveTo(fullRect.topLeft());
                curvedCorner.lineTo(curveStart);
                curvedCorner.arcTo(curveRect, 180, 90);
                curvedCorner.lineTo(fullRect.bottomRight());
                painter->drawPath(curvedCorner);

            }
            else if (isLastColumn)
            {
                // Right border, curve on the bottom right, bottom border

                // Define curve start, end and size
                QPoint curveStart = fullRect.bottomRight();
                curveStart.setY(curveStart.y() - m_prefabCapsuleRadius);
                QRect curveRect = QRect(fullRect.right() - curveRectSize, fullRect.bottom() - curveRectSize, curveRectSize, curveRectSize);

                // Curved Corner
                QPainterPath curvedCorner;
                curvedCorner.moveTo(fullRect.topRight());
                curvedCorner.lineTo(curveStart);
                curvedCorner.arcTo(curveRect, 0, -90);
                curvedCorner.lineTo(rect.bottomLeft());
                painter->drawPath(curvedCorner);
            }
            else
            {
                // Bottom Border
                painter->drawLine(rect.bottomLeft(), rect.bottomRight());
            }
        }
        else
        {
            if (isFirstColumn)
            {
                // Left Border
                painter->drawLine(fullRect.topLeft(), fullRect.bottomLeft());
            }

            if (isLastColumn)
            {
                // Right Border
                painter->drawLine(fullRect.topRight(), fullRect.bottomRight());
            }
        }

        painter->restore();
    }

    bool PrefabUiHandler::IsLastVisibleChild(const QModelIndex& parent, const QModelIndex& child)
    {
        QModelIndex lastVisibleItemIndex = GetLastVisibleChild(parent);
        QModelIndex index = child;

        // GetLastVisibleChild returns an index set to the ColumnName column
        if (index.column() != EntityOutlinerListModel::ColumnName)
        {
            index = index.siblingAtColumn(EntityOutlinerListModel::ColumnName);
        }

        return index == lastVisibleItemIndex;
    }

    QModelIndex PrefabUiHandler::GetLastVisibleChild(const QModelIndex& parent)
    {
        auto model = parent.model();
        QModelIndex index = parent;

        // The parenting information for the index are stored in the ColumnName column
        if (index.column() != EntityOutlinerListModel::ColumnName)
        {
            index = index.siblingAtColumn(EntityOutlinerListModel::ColumnName);
        }

        return Internal_GetLastVisibleChild(model, index);
    }

    QModelIndex PrefabUiHandler::Internal_GetLastVisibleChild(const QAbstractItemModel* model, const QModelIndex& index)
    {
        if (!model->hasChildren(index) || !index.data(EntityOutlinerListModel::ExpandedRole).value<bool>())
        {
            return index;
        }

        int childCount = index.data(EntityOutlinerListModel::ChildCountRole).value<int>();
        QModelIndex lastChild = model->index(childCount - 1, EntityOutlinerListModel::ColumnName, index);

        return Internal_GetLastVisibleChild(model, lastChild);
    }
}