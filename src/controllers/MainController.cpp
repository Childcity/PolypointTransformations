#include "MainController.h"

#include <utils/import/ColladaFormatImporter.h>
#include <utils/MathUtils.h>

#include <thread>

MainController::MainController(QObject *parent)
    : QObject(parent)
{
}

MainController::~MainController() = default;

MeshListModel *MainController::meshListModel() const
{
	return m_meshListModel.get();
}

QVector3D MainController::globalScale() const
{
	return m_globalScale;
}

void MainController::loadMeshes()
{
	unloadMeshes();

	std::jthread([this] {
		const auto path = "C:\\Users\\Ariel\\Documents\\AAScetch\\exp\\Untitled.dae";
		ColladaFormatImporter importer(path);
		importer.importGeometries();
		m_meshes = importer.getGeometries();
		QMetaObject::invokeMethod(this, &MainController::initMeshes, Qt::QueuedConnection);
	}).detach();
}

void MainController::initMeshes()
{
	m_meshListModel = std::make_unique<MeshListModel>(m_meshes);
	emit loadComplete();
	emit meshListModelChanged();
}

void MainController::unloadMeshes()
{
	m_meshListModel.reset();
	emit meshListModelChanged();
}

void MainController::applyPolydotTransformations(QVariantList origBasises, QVariantList resBasises)
{
	if (!m_meshListModel || !m_meshListModel->rowCount({})) {
		return;
	}

	unloadMeshes();

	auto transformer = [&](Mesh mesh) {
		switch (m_meshType) {
		case MeshType::ClosedMesh:
			return MathUtils::getPolydotTransformedMesh(std::move(mesh), origBasises, resBasises);
		case MeshType::StreightLineMesh:
			return MathUtils::getPolydotTransformedStreightLineMesh(
			    std::move(mesh), origBasises, resBasises);
		}
		std::unreachable();
	};

	MeshList outMeshes;
	outMeshes.reserve(m_meshes.size());

	std::ranges::transform(m_meshes, std::back_inserter(outMeshes), transformer);

	m_meshListModel = std::make_unique<MeshListModel>(std::move(outMeshes));
	emit meshListModelChanged();
}

MeshType MainController::meshType() const
{
	return m_meshType;
}

void MainController::setMeshType(MeshType meshType)
{
	if (m_meshType == meshType) {
		return;
	}
	m_meshType = meshType;
	emit meshTypeChanged();
}
