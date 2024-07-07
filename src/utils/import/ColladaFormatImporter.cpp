#include "ColladaFormatImporter.h"

#include <ranges>

#include <QFile>
#include <QLoggingCategory>
#include <QXmlStreamReader>

namespace {

Q_LOGGING_CATEGORY(xml_importer, "utils.xml_importer", QtInfoMsg);

using namespace Qt::StringLiterals;

QString getId(QXmlStreamReader &reader)
{
	return reader.attributes().value("id").toString();
}

std::vector<QVector3D> readFloatArray(QXmlStreamReader &reader)
{
	constexpr auto stride = 3;
	const auto id = getId(reader);
	const auto count = reader.attributes().value("count").toInt();
	const auto pointsCount = count / stride;
	const auto floatArrayStr = reader.readElementText();

	qCDebug(xml_importer) << std::format(
	    "{} [count={}, pointsCount={}])", qPrintable(reader.name().toString()), count, pointsCount)
	                      << id << floatArrayStr;

	std::vector<QVector3D> result;
	result.reserve(pointsCount);

	for (auto &&vec3 : floatArrayStr.split(' ', Qt::SkipEmptyParts) | std::views::chunk(stride)) {
		result.emplace_back(vec3[0].toFloat(), vec3[1].toFloat(), vec3[2].toFloat());
	}

	Q_ASSERT(result.size() == pointsCount);
	return result;
}

std::vector<std::pair<int, int>> readLines(QXmlStreamReader &reader)
{
	const auto count = reader.attributes().value("count").toInt();

	decltype(readLines(reader)) result;
	result.reserve(count);

	while (reader.readNextStartElement()) {
		if (reader.name() == "p"_L1) {
			const auto lineVertexStr = reader.readElementText();

			qCDebug(xml_importer)
			    << std::format("{} [count={}])", qPrintable(reader.name().toString()), count)
			    << lineVertexStr;

			int lastSecondVertex = -1;
			for (const auto &&lineVertices :
			     lineVertexStr.split(' ', Qt::SkipEmptyParts) | std::views::chunk(2)) {
				const int first = lineVertices[0].toInt();
				const int second = lineVertices[1].toInt();
				const int firstVertexIndex = lastSecondVertex == second ? second : first;
				lastSecondVertex = lastSecondVertex == second ? first : second;
				result.emplace_back(firstVertexIndex, lastSecondVertex);
			}
		} else {
			reader.skipCurrentElement();
		}
	}

	Q_ASSERT(result.size() == count);
	return result;
}

} // namespace

ColladaFormatImporter::ColladaFormatImporter(const QString path)
    : m_path(path)
{
}

MeshList ColladaFormatImporter::getGeometries()
{
	return m_meshes;
}

void ColladaFormatImporter::importGeometries()
{
	QFile file(m_path);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qCCritical(xml_importer)
		    << "Failed to read" << file.fileName() << ":" << file.errorString();
		return;
	}

	QXmlStreamReader reader(&file);

	readCollada(reader);

	if (reader.hasError()) {
		qCCritical(xml_importer).noquote().nospace()
		    << "Parse Error: '" << file.fileName() << ":" << reader.lineNumber() << ":"
		    << reader.columnNumber() << "' - " << reader.errorString();
		exit(10);
	}
}

void ColladaFormatImporter::readCollada(QXmlStreamReader &reader)
{
	if (reader.readNextStartElement()) {
		if (reader.name() == "COLLADA"_L1) {
			while (reader.readNextStartElement()) {
				if (reader.name() == "library_geometries"_L1) {
					readLibraryGeometries(reader);
				} else {
					reader.skipCurrentElement();
				}
			}
		} else {
			reader.raiseError("Failed to find COLLADA document.");
		}
	}
}

void ColladaFormatImporter::readLibraryGeometries(QXmlStreamReader &reader)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "geometry"_L1) {
			while (reader.readNextStartElement()) {
				if (reader.name() == "mesh"_L1) {
					readMesh(reader);
				}
			}
		}
	}
}

void ColladaFormatImporter::readMesh(QXmlStreamReader &reader)
{
	std::vector<QVector3D> points;
	std::vector<std::pair<int /*startIndx*/, int /*endIndx*/>> linesVertices;

	while (reader.readNextStartElement()) {
		if (reader.name() == "source"_L1) {
			while (reader.readNextStartElement()) {
				if (reader.name() == "float_array"_L1) {
					points = readFloatArray(reader);
					reader.skipCurrentElement();
					break;
				}
			}
		} else if (reader.name() == "lines"_L1) {
			linesVertices = readLines(reader);
		} else {
			reader.skipCurrentElement();
		}
	}

	Mesh mesh;
	mesh.reserve(linesVertices.size());
	for (auto &&[startIndx, endIndx] : linesVertices) {
		mesh.emplace_back(points.at(startIndx), points.at(endIndx));
	}
	m_meshes.emplace_back(std::move(mesh));
}
