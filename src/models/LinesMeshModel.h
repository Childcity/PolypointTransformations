#pragma once

#include <QAbstractListModel>

#include "GeometryPrimitives.h"

class LinesMeshModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum LinesMeshRoles
	{
		LineGeometryRole = Qt::UserRole + 1,
		SelectedRole,
	};

	explicit LinesMeshModel(const Mesh &mesh, QObject *parent = nullptr);
	~LinesMeshModel() override;

	QHash<int, QByteArray> roleNames() const override;
	int rowCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
	std::vector<std::unique_ptr<class LineGeometry>> m_mesh;
};
