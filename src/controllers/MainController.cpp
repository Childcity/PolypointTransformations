#include "MainController.h"

#include <ranges>
#include <thread>

#include <models/LinesMeshModel.h>
#include <utils/import/ColladaFormatImporter.h>
#include <utils/MathUtils.h>

MainController::MainController(QObject *parent)
    : QObject(parent)
{
	m_origBasises = std::make_unique<BasisPointsModel>();
	m_resBasises = std::make_unique<BasisPointsModel>();

	connect(
	    m_resBasises.get(),
	    &BasisPointsModel::dataChanged,
	    this,
	    [this] {
		    applyPolydotTransformationsForSelected(
		        m_origBasises->rawData(), m_resBasises->rawData());
	    },
	    Qt::QueuedConnection);
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

void MainController::applyPolydotTransformationsForSelected(
    QVariantList origBasises, QVariantList resBasises)
{
	if (!m_meshListModel || !m_meshListModel->rowCount({})) {
		return;
	}

	MeshList mesheListToTransform;
	std::vector<LinesMeshModel::Selected> selectedLinesInMeshList;

	// Collect selected lines ids and take that lines from original object stored in m_meshes
	for (int i = 0; i < m_meshListModel->rowCount({}); ++i) {
		const auto linesMeshModel =
		    m_meshListModel->index(i)
		        .data(MeshListModel::LinesMeshModelRole)
		        .value<LinesMeshModel *>();

		assert(linesMeshModel);

		auto selectedLines = linesMeshModel->selected();
		if (selectedLines.empty()) {
			continue;
		}

		const Mesh &mesh = m_meshes[i];
		Mesh meshToTransform;

		for (const auto &[_, selectedLineId] : selectedLines) {
			const auto foundLine = std::ranges::find_if(mesh, [&selectedLineId](const auto &line) {
				return line.id == selectedLineId;
			});
			if (foundLine != mesh.cend()) {
				meshToTransform.push_back(*foundLine);
			}
		}

		selectedLinesInMeshList.push_back(std::move(selectedLines));
		mesheListToTransform.push_back(std::move(meshToTransform));

		// currently logic works only for selection in bound of 1 mesh
		assert(mesheListToTransform.size() == 1);
	}

	assert(mesheListToTransform.size() == selectedLinesInMeshList.size());
	if (mesheListToTransform.size() == 0) {
		qWarning() << "Nothing to transform!";
		return;
	}

	auto transformer = [&](const Mesh &mesh) {
		switch (m_meshType) {
		case MeshType::ClosedMesh:
			return MathUtils::getPolydotTransformedMesh(mesh, origBasises, resBasises);
		case MeshType::StreightLineMesh:
			return MathUtils::getPolydotTransformedStreightLineMesh(mesh, origBasises, resBasises);
		}
		std::unreachable();
	};

	MeshList outMeshes;
	outMeshes.reserve(mesheListToTransform.size());

	std::ranges::transform(mesheListToTransform, std::back_inserter(outMeshes), transformer);

	// Update changed lines in the model
	for (int i = 0; i < m_meshListModel->rowCount({}) && i < outMeshes.size(); ++i) {
		auto linesMeshModel =
		    m_meshListModel->index(i)
		        .data(MeshListModel::LinesMeshModelRole)
		        .value<LinesMeshModel *>();

		const Mesh &outMesh = outMeshes[i];
		const auto &selectedLines = selectedLinesInMeshList[i];

		for (const auto &[line, selectedLine] : std::views::zip(outMesh, selectedLines)) {
			const auto &[selectedLineIndex, _] = selectedLine;
			linesMeshModel->setData(
			    selectedLineIndex,
			    QVariant::fromValue<Line>(line),
			    LinesMeshModel::LineGeometryRole);
		}
	}
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

BasisPointsModel *MainController::basisPointsModel() const
{
	return m_resBasises.get();
}
