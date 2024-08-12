#include "MainController.h"

#include <array>
#include <ranges>
#include <string_view>
#include <thread>

#include <models/LinesMeshModel.h>
#include <utils/import/ColladaFormatImporter.h>
#include <utils/MathUtils.h>

namespace {

namespace v = std::views;
using namespace std::string_view_literals;

std::optional<Line> getLineById(const Mesh &mesh, const LineId &lineId)
{
	const auto foundLine = std::ranges::find_if(mesh, [&lineId](const auto &line) {
		return line.id == lineId;
	});
	return foundLine != mesh.cend() ? std::optional(*foundLine) : std::nullopt;
}

bool selected(const LinesMeshModel::Selected &selectedLines, const Line &line)
{
	return std::ranges::find(selectedLines, line.id, [](const auto &s) { return s.second; })
	       != selectedLines.end();
}

} // namespace

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

		for (const auto &[line, selectedLine] : v::zip(outMesh, selectedLines)) {
			const auto &[selectedLineIndex, _] = selectedLine;
			linesMeshModel->setData(
			    selectedLineIndex,
			    QVariant::fromValue<Line>(line),
			    LinesMeshModel::LineGeometryRole);
		}
	}

	////////////////////

	for (int i = 0; i < m_meshListModel->rowCount({}) && i < outMeshes.size(); ++i) {
		LinesMeshModel *linesMeshModel =
		    m_meshListModel->index(i)
		        .data(MeshListModel::LinesMeshModelRole)
		        .value<LinesMeshModel *>();

		const Mesh &inMesh = m_meshes[i];
		const Mesh &outMesh = outMeshes[i];

		// TODO: Instead of `selectedLines` use outMesh (outMesh must contain only selectedLines)
		const auto &selectedLines = selectedLinesInMeshList[i];
		//

		for (const auto &[_, selectedLineId] : selectedLines) {
			// For each selected line find all connected lines (connected to p1 / p2)

			const auto selectedLineOld = getLineById(inMesh, selectedLineId);
			const auto selectedLineNew = getLineById(outMesh, selectedLineId);
			assert(selectedLineOld && selectedLineNew);
			assert(selectedLineOld->id == selectedLineId && selectedLineNew->id == selectedLineId);

			std::list<LineId> adjustedLinesInMeshe;

			for (const auto &[selectedLineOldP, selectedLineNewP, selP] : //
			     v::zip(
			         std::array{selectedLineOld->p1, selectedLineOld->p2},
			         std::array{selectedLineNew->p1, selectedLineNew->p2},
			         std::array{"selP1"sv, "selP2"sv} // For case 1: See doc/case_1)
			         ) //
			) {
				// For each Line where р1/р2 == р update its р1/р2 to new р1`/р2`

				for (const auto &[lineInInMesh, index] :
				     v::zip(inMesh, v::iota(0, static_cast<int>(inMesh.size()))) //
				) {
					if (selected(selectedLines, lineInInMesh)) {
						// Skip lines, that have been transformed (selected lines)
						continue;
					}
					if (std::ranges::contains(adjustedLinesInMeshe, lineInInMesh.id)) {
						qWarning() << "Skip " << index;
						continue;
					}

					if (selectedLineOldP == lineInInMesh.p1) {
						qWarning()
						    << "p1" << lineInInMesh.p1 << " -> "
						    << (selP == "selP1" ? "(selectedLineOld->p1)" : "(selectedLineOld->p2)")
						    << selectedLineNewP << index;

						auto line = lineInInMesh;
						line.p1 = selectedLineNewP;

						// line.p1 = selectedLineOldP.distanceToPoint(selectedLineNew->p1)
						//                   <= selectedLineOldP.distanceToPoint(selectedLineNew->p2)
						//               ? selectedLineNew->p1
						//               : selectedLineNew->p2;
						//  line.p1 = selectedLineNew->p2;
						//?????????????????line.p1 = dist min(selectedLineNew->p1,
						//  selectedLineNew->p2)
						linesMeshModel->updateLine(index, line);
						adjustedLinesInMeshe.push_back(lineInInMesh.id);
					} else if (selectedLineOldP == lineInInMesh.p2) {
						// qWarning()
						//     << "p2" << lineInInMesh.p2 << " -> "
						//     << (selP == "selP1" ? "(selectedLineOld->p1)" : "(selectedLineOld->p2)")
						//     << selectedLineNewP << index;

						auto line = lineInInMesh;
						line.p2 = selectedLineNewP;
						// line.p2 = selectedLineOldP.distanceToPoint(selectedLineNew->p1)
						//                   <= selectedLineOldP.distanceToPoint(selectedLineNew->p2)
						//               ? selectedLineNew->p1
						//               : selectedLineNew->p2;
						// line.p2 = selectedLineNew->p2;
						linesMeshModel->updateLine(index, line);
						adjustedLinesInMeshe.push_back(lineInInMesh.id);
					}
				}
			}
		}
	}
	qWarning() << "11111111111111111111111111111111111111111111111111111111111111111111";
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
