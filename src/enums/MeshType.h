#pragma once

#include <QQmlEngine>

namespace types {
Q_NAMESPACE

enum class MeshType
{
	ClosedMesh,
	StreightLineMesh,
};
Q_ENUM_NS(MeshType)
QML_NAMED_ELEMENT(MeshType)

} // namespace types

using MeshType = types::MeshType;
