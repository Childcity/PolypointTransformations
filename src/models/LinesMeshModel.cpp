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
	    {SelectedRole, "selected"},
	};
}

int LinesMeshModel::rowCount(const QModelIndex &parent) const
{
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
	case SelectedRole:
		return m_mesh.at(row)->selected();
	default:
		return {};
	}
}

bool LinesMeshModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) {
		return {};
	}

	const auto row = index.row();

	switch (role) {
	case SelectedRole:
		m_mesh.at(row)->setSelected(value.toBool());
		emit dataChanged(index, index, {SelectedRole});
		return true;
	default:
		return {};
	};
}
