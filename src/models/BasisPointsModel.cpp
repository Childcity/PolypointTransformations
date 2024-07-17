#include "BasisPointsModel.h"

#include <QVector3D>

BasisPointsModel::BasisPointsModel(QObject *parent)
    : QAbstractListModel(parent)
{
	m_basises = {
	    {0, 0, 0},
	    {1, 1, 0},
	    {1, 2, 0},
	    {2, 1, 0},
	};
}

BasisPointsModel::~BasisPointsModel() = default;

QHash<int, QByteArray> BasisPointsModel::roleNames() const
{
	return {
	    {NameRole, "name"},
	    {PositionRole, "position"},
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
		return std::format("B{}`", row + 1).c_str();
	case PositionRole:
		return m_basises.at(row);
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
