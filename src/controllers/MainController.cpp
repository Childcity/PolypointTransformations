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
using namespace std::chrono_literals;

auto cTimeout = 700;
auto cLine1DeltaX = 0.2;

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

auto startAnimationDelay = new QElapsedTimer();
auto elapsedTime = new QElapsedTimer();

MainController::MainController(QObject *parent)
    : QObject(parent)
{
	m_origBasises = std::make_unique<BasisPointsModel>("B%1");
	m_resBasises = std::make_unique<BasisPointsModel>("B%1`");

	connect(m_origBasises.get(), &BasisPointsModel::dataChanged, this, [this] {
		qDebug() << "ORIG -------------------------";
		int bi = 0;
		for (auto b : m_origBasises->rawData()) {
			auto bv = b.value<QVector3D>();
			qDebug() << " {" << bv.x() << ", " << bv.y() << ", 0},";
		}
		qDebug() << "-------------------------";
	});

	connect(
	    m_resBasises.get(),
	    &BasisPointsModel::dataChanged,
	    this,
	    [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles) {
		    if (bottomRight.row() != 4) {
			    return;
		    }

		    qDebug() << topLeft << bottomRight << elapsedTime->elapsed();
		    elapsedTime->restart();

		    QTimer::singleShot(
		        startAnimationDelay->elapsed() < 1000 ? 3000 : cTimeout,
		        Qt::TimerType::PreciseTimer,
		        [this] {
			        applyPolydotTransformationsForSelected(
			            m_origBasises->rawData(), m_resBasises->rawData());
			        m_tmpBasises = m_resBasises.get();
			        emit outBasisPointsModelChanged();

			        QTimer::singleShot(cTimeout * 2, Qt::TimerType::PreciseTimer, [this] {
				        auto &mesh = m_meshes.front();
				        qDebug() << "mesh[1].p2" << mesh[1].p2.x();
				        mesh[1].p2.setX(mesh[1].p2.x() + cLine1DeltaX);
				        cLine1DeltaX += 0.3;
				        mesh[2].p1 = mesh[1].p2;
				        m_tmpBasises = m_origBasises.get();
				        emit outBasisPointsModelChanged();
				        QMetaObject::invokeMethod(
				            this, &MainController::initMeshes, Qt::QueuedConnection);
			        });
		        });
	    },
	    Qt::QueuedConnection);
	connect(
	    this,
	    &MainController::loadComplete,
	    this,
	    [this] {
		    // m_resBasises->setData(m_resBasises->index(0), QVector3D{0.9 * 3, 0.9 * 3, 0});
		    // m_resBasises->setData(m_resBasises->index(1), QVector3D{0.9 * 3, 2.1 * 3, 0});
		    // m_resBasises->setData(m_resBasises->index(2), QVector3D{2.1 * 3, 2.1 * 3, 0});
		    // m_resBasises->setData(m_resBasises->index(3), QVector3D{2.1 * 3, 0.9 * 3, 0});
		    // m_resBasises->setData(m_resBasises->index(4), QVector3D{1.5 * 3, 1.5 * 3, 0});
		    // m_resBasises->setData(m_resBasises->index(0), QVector3D{1, 1, 0});
		    // m_resBasises->setData(m_resBasises->index(1), QVector3D{1, 2, 0});
		    // m_resBasises->setData(m_resBasises->index(2), QVector3D{2.2, 2.4, 0});
		    // m_resBasises->setData(m_resBasises->index(3), QVector3D{2, 1, 0});
		    // m_resBasises->setData(m_resBasises->index(4), QVector3D{2.7, 1.5, 0});
		    // m_resBasises->setData(m_resBasises->index(1), QVector3D{2, 2, 0});
		    // m_resBasises->setData(m_resBasises->index(2), QVector3D{1, 2, 0});
		    // m_resBasises->setData(m_resBasises->index(3), QVector3D{1.5, 1.5, 0});
		    // m_resBasises->setData(m_resBasises->index(4), QVector3D{2, 1, 0});
		    m_resBasises->setData(m_resBasises->index(0), QVector3D{0.233062 * 3, 1.42754, 0});
		    m_resBasises->setData(m_resBasises->index(1), QVector3D{0.460022 * 3, 2.92124, 0});
		    m_resBasises->setData(m_resBasises->index(2), QVector3D{1.04417 * 3, 2.63525, 0});
		    m_resBasises->setData(m_resBasises->index(3), QVector3D{1.09037 * 3, 1.03, 0});
		    m_resBasises->setData(m_resBasises->index(4), QVector3D{0.636043 * 3, 0.332303, 0});
	    },
	    Qt::QueuedConnection);

	loadMeshes();
	m_tmpBasises = m_origBasises.get();
	// m_tmpBasises = m_resBasises.get();
	initMeshes();

	startAnimationDelay->start();
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

	// std::jthread([this] {
	const auto path = "C:\\Users\\Ariel\\Documents\\AAScetch\\exp\\Untitled.dae";
	ColladaFormatImporter importer(path);
	importer.importGeometries();
	m_meshes = importer.getGeometries();
	// QMetaObject::invokeMethod(this, &MainController::initMeshes, Qt::QueuedConnection);
	//}).detach();
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

	qDebug() << "-------------------------";
	int bi = 0;
	for (auto b : resBasises) {
		auto bv = b.value<QVector3D>();
		qDebug() << "m_resBasises->setData(m_resBasises->index(" << bi++ << "), QVector3D{"
		         << (bv.x() / 3.f) << " * 3, " << (bv.y()) << ", 0});";
	}
	qDebug() << "-------------------------";

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
						// qWarning() << "Skip " << index;
						continue;
					}

					if (selectedLineOldP == lineInInMesh.p1) {
						// qWarning()
						//     << "p1" << lineInInMesh.p1 << " -> "
						//     << (selP == "selP1" ? "(selectedLineOld->p1)" : "(selectedLineOld->p2)")
						//     << selectedLineNewP << index;

						auto line = lineInInMesh;
						line.p1 = selectedLineNewP;

						linesMeshModel->updateLine(index, line);
						adjustedLinesInMeshe.push_back(lineInInMesh.id);
					} else if (selectedLineOldP == lineInInMesh.p2) {
						// qWarning()
						//     << "p2" << lineInInMesh.p2 << " -> "
						//     << (selP == "selP1" ? "(selectedLineOld->p1)" : "(selectedLineOld->p2)")
						//     << selectedLineNewP << index;

						auto line = lineInInMesh;
						line.p2 = selectedLineNewP;

						linesMeshModel->updateLine(index, line);
						adjustedLinesInMeshe.push_back(lineInInMesh.id);
					}
				}
			}
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

BasisPointsModel *MainController::inBasisPointsModel() const
{
	return {};
	return m_origBasises.get();
}

BasisPointsModel *MainController::outBasisPointsModel() const
{
	return m_tmpBasises;
	return m_resBasises.get();
}
