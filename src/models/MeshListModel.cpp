#include "MeshListModel.h"

#include <models/LinesMeshModel.h>

MeshListModel::MeshListModel(MeshList meshes, QObject *parent)
    : QAbstractListModel(parent)
    , m_meshes(std::move(meshes))
{
	for (const auto &mesh : m_meshes) {
		m_mesheModels.emplace_back(std::make_unique<LinesMeshModel>(mesh));
	}
}

MeshListModel::~MeshListModel() = default;

QHash<int, QByteArray> MeshListModel::roleNames() const
{
	return {
	    {LinesMeshModelRole, "linesMeshModel"},
	};
}

int MeshListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) {
		return 0;
	}

	return static_cast<int>(m_mesheModels.size());
}

QVariant MeshListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) {
		return {};
	}

	const auto row = index.row();

	switch (role) {
	case LinesMeshModelRole:
		return QVariant::fromValue(m_mesheModels.at(row).get());
	default:
		return {};
	}
}

const MeshList &MeshListModel::rawData() const
{
	return m_meshes;
}
