#include "BasisPointsModel.h"

#include <QVector3D>

BasisPointsModel::BasisPointsModel(QString nameTemplate, QObject *parent)
    : QAbstractListModel(parent)
    , m_nameTemplate(std::move(nameTemplate))
{
	m_basises = {
	    {1.2, 1.4, 0},
	    {1.7, 2.3, 0},
	    {2.7, 2.1, 0},
	    {2.8, 1.2, 0},
	    {1.9, 0.8, 0},
	};
}

BasisPointsModel::~BasisPointsModel() = default;

QHash<int, QByteArray> BasisPointsModel::roleNames() const
{
	return {
	    {NameRole, "name"},
	    {PositionRole, "position"},
	    {PointScaleRole, "pointScale"},
	    {NamePointSizeRole, "namePointSize"},
	};
}

int BasisPointsModel::rowCount(const QModelIndex &parent) const
{
	return static_cast<int>(m_basises.size());
}

QVariant BasisPointsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) {
		return {};
	}

	const auto row = index.row();

	switch (role) {
	case NameRole:
		return m_nameTemplate.arg(row + 1);
	case PositionRole:
		return m_basises.at(row);
	case PointScaleRole:
		return 0.4;
	case NamePointSizeRole:
		return 4;
	default:
		return {};
	}
}

bool BasisPointsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) {
		return {};
	}

	const auto row = index.row();

	switch (role) {
	case PositionRole:
		m_basises.at(row) = value.value<QVector3D>();
		emit dataChanged(index, index, {PositionRole});
		return true;
	};
	return {};
}

QVariantList BasisPointsModel::rawData() const
{
	QVariantList res;
	std::ranges::copy(m_basises, std::back_inserter(res));
	return res;
}
