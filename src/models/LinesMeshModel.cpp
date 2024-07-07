#include "LinesMeshModel.h"

LinesMeshModel::LinesMeshModel(const Mesh &mesh, QObject *parent)
    : QAbstractListModel(parent)
{
	for (const auto &line : mesh) {
		m_mesh.emplace_back(std::make_unique<LineGeometry>(line));
	}
}

LinesMeshModel::~LinesMeshModel() = default;

QHash<int, QByteArray> LinesMeshModel::roleNames() const
{
	return {
	    {LineGeometryRole, "lineGeometry"},
	};
}

int LinesMeshModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) {
		return 0;
	}

	return static_cast<int>(m_mesh.size());
}

QVariant LinesMeshModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) {
		return {};
	}

	const auto row = index.row();

	switch (role) {
	case LineGeometryRole:
		return QVariant::fromValue(m_mesh.at(row).get());
	default:
		return {};
	}
}
