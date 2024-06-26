#include "MainController.h"

MainController::MainController(QObject *parent)
    : QObject(parent)
{
}

MainController::~MainController() = default;

QVector3D MainController::globalScale() const
{
	return m_globalScale;
}
