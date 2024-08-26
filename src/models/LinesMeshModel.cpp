#include "LinesMeshModel.h"

#include <ranges>

LinesMeshModel::LinesMeshModel(const Mesh &mesh, QObject *parent)
    : QAbstractListModel(parent)
{
	int i = 0;
	for (const auto &line : mesh) {
		m_mesh.emplace_back(std::make_unique<LineGeometry>(line));
		// if (std::ranges::contains(std::array{2, 3, 5, 8}, i++)) { // case_2 selection
		// if (std::ranges::contains(std::array{0, 18, 1, 2}, i++)) { // 3x3
		//	m_mesh.back()->setSelected(true);
		// }
	}
}

LinesMeshModel::~LinesMeshModel() = default;

QHash<int, QByteArray> LinesMeshModel::roleNames() const
{
	return {
	    {LineGeometryRole, "lineGeometry"},
	    {SelectedRole, "selected"},
	};
}

int LinesMeshModel::rowCount(const QModelIndex &parent) const
{
	return static_cast<int>(m_mesh.size());
}

QVariant LinesMeshModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) {
		return {};
	}

	const auto row = index.row();

	switch (role) {
	case LineGeometryRole:
		return QVariant::fromValue(m_mesh.at(row).get());
	case SelectedRole:
		return m_mesh.at(row)->selected();
	default:
		return {};
	}
}

bool LinesMeshModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) {
		assert(false);
		return {};
	}

	const auto row = index.row();

	switch (role) {
	case LineGeometryRole: {
		const auto &newLine = value.value<Line>();
		updateLine(row, newLine);
		return true;
	}
	case SelectedRole:
		m_mesh.at(row)->setSelected(value.toBool());
		emit dataChanged(index, index, {SelectedRole});
		return true;
	default:
		return {};
	};
}

void LinesMeshModel::updateLine(int row, const Line &line)
{
	auto lineGeom = std::make_unique<LineGeometry>(line);
	lineGeom->setSelected(m_mesh.at(row)->selected());
	m_mesh.at(row) = std::move(lineGeom);
	const auto indx = index(row);
	emit dataChanged(indx, indx, {LineGeometryRole});
}

LinesMeshModel::Selected LinesMeshModel::selected() const
{
	namespace v = std::views;

	Selected res;
	for (const auto &[lineGeom, i] : v::zip(m_mesh, v::iota(0u, m_mesh.size()))) {
		if (lineGeom->selected()) {
			res.emplace_back(index(i), lineGeom->toLine().id);
		}
	}
	return res;
}
