#pragma once

#include <QAbstractListModel>

#include "GeometryPrimitives.h"

class MeshListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum MeshListRoles
	{
		LinesMeshModelRole = Qt::UserRole + 1,
	};

	explicit MeshListModel(MeshList meshes, QObject *parent = nullptr);
	~MeshListModel() override;

	QHash<int, QByteArray> roleNames() const override;
	int rowCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;

	const MeshList &rawData() const;

private:
	const MeshList m_meshes;
	std::vector<std::unique_ptr<class LinesMeshModel>> m_mesheModels;
};
