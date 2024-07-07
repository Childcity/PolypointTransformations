#pragma once

#include <QAbstractListModel>

class BasisPointsModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum BasisPointRoles
	{
		NameRole = Qt::UserRole + 1,
		PositionRole,
	};

	explicit BasisPointsModel(QObject *parent = nullptr);
	~BasisPointsModel() override;

	QHash<int, QByteArray> roleNames() const override;
	int rowCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = PositionRole) override;

private:
	std::vector<QVector3D> m_basises;
};
