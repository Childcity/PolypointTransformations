#pragma once

#include <QQmlEngine>
#include <QVector3D>

class MainController : public QObject
{
	Q_OBJECT
	QML_NAMED_ELEMENT(MainController)

	Q_PROPERTY(QVector3D globalScale READ globalScale CONSTANT FINAL)

public:
	explicit MainController(QObject *parent = nullptr);
	~MainController() override;

	QVector3D globalScale() const;

signals:

private:
	QVector3D m_globalScale = {100, 100, 100};
};
