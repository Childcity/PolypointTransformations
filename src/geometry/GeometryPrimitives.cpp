#include "GeometryPrimitives.h"

#include <QLoggingCategory>
#include <QVector3D>

#include <utils/GaussJordanElimination.h>

namespace {

Q_LOGGING_CATEGORY(geometry, "utils.geometry", QtInfoMsg)

using PType = float;
constexpr int cPointDataCount = 3;
constexpr int cPointDataSize = cPointDataCount * sizeof(PType);

// for (auto i = 0; i < vertexes.size(); i += sizeof(int)) {
//	auto number = std::span(vertexes.begin() + i, sizeof(int));
//	qDebug() << qFromLittleEndian<int>(number.data());
// }

// using v = QVector2D;
// const auto lines = std::vector{
//     v{0, 0},
//     v{5, 5},
//     v{6, 5},
//     v{1, 2},
// };

// namespace std {

// template<size_t N>
// span(const QByteArray &b) -> span<const char>;

//} // namespace std

float toF(double v)
{
	return static_cast<float>(v);
}

void setPoint(QByteArray &data, int pointIndex, QVector3D point)
{
	PType *ptr = reinterpret_cast<PType *>(data.begin());
	std::advance(ptr, pointIndex * cPointDataCount);
	*ptr++ = static_cast<PType>(point.x());
	*ptr++ = static_cast<PType>(point.y());
	*ptr++ = static_cast<PType>(point.z());
}

QString toString(QVector3D vec)
{
	return QString("X:%1 Y:%2 Z:%3")
	    .arg(
	        QString::number(vec.x(), 10, 3),
	        QString::number(vec.y(), 10, 3),
	        QString::number(vec.z(), 10, 3));
}

} // namespace

Line Line::FromStreightLine(StreightLine line)
{
	if (line.isNull()) {
		qCWarning(geometry) << __func__ << "line.isNull()";
		return {};
	}

	auto fuzzyIsNull = [](auto d) {
		return qAbs(d) <= 0.000001;
	};

	const auto [A, B, C] = std::tuple{line.A, line.B, line.C};

	if (fuzzyIsNull(B)) { // The line is parallel to Oy
		auto x = [A, B, C](double y) -> float {
			return toF((-B * y - C) / A);
		};

		const auto y1 = -100., y2 = 100.;
		return {
		    .p1 = {x(y1), toF(y1), 0}, //
		    .p2 = {x(y2), toF(y2), 0} //
		};
	} else {
		auto y = [A, B, C](double x) -> float {
			return toF((-A * x - C) / B);
		};

		const auto x1 = -100., x2 = 100.;
		return {
		    .p1 = {toF(x1), y(x1), 0}, //
		    .p2 = {toF(x2), y(x2), 0} //
		};
	}
}

StreightLine Line::toStraightLine() const
{
	return StreightLine::FromLine(*this);
}

bool Line::isNull() const
{
	return qFuzzyIsNull(p1.distanceToPoint(p2));
}

StreightLine StreightLine::FromLine(Line line)
{
	// https://math.stackexchange.com/questions/637922/how-can-i-find-coefficients-a-b-c-given-two-points
	return {
	    line.p1.y() - line.p2.y(),
	    line.p2.x() - line.p1.x(),
	    line.p1.x() * line.p2.y() - line.p2.x() * line.p1.y(),
	};
}

bool StreightLine::isNull() const
{
	return qFuzzyIsNull(length());
}

double StreightLine::length() const
{
	return std::hypot(A, B, 0.f); // std::sqrt(A * A + B * B);
}

bool StreightLine::isNormilized() const
{
	const double len = length();
	return qFuzzyIsNull(len - 1.f) || qFuzzyIsNull(len);
}

StreightLine StreightLine::normilized() const
{
	if (isNormilized()) {
		return *this;
	}
	const auto len = length();
	return {A / len, B / len, C / len};
}

double StreightLine::signDistanceToPoint(QVector3D p) const
{
	if (!isNormilized()) {
		qFatal("distanceToPoint must be called for normilized line!");
	}
	return A * p.x() + B * p.y() + C;
}

QVector3D StreightLine::intersect(StreightLine other)
{
	namespace gauss = gauss_jordan_elimination;
	const auto [A1, B1, C1] = this->toTuple();
	const auto [A2, B2, C2] = other.toTuple();
	const auto point = gauss::SolveSystem({
	    {A1, B1, -C1},
	    {A2, B2, -C2},
	});
	assert(point.size() == 2);
	return {toF(point[0]), toF(point[1]), 0};
}

LineGeometry::LineGeometry(StreightLine line)
{
	m_line = Line::FromStreightLine(line);
}

LineGeometry::LineGeometry(Line line)
    : m_line(std::move(line))
{
}

LineId LineGeometry::id() const
{
	return m_line.id;
}

QVector3D LineGeometry::p1() const
{
	return m_line.p1;
}

void LineGeometry::setP1(QVector3D newP1)
{
	if (m_line.p1 == newP1) {
		return;
	}
	m_line.p1 = newP1;

	emit p1Changed();
}

QVector3D LineGeometry::p2() const
{
	return m_line.p2;
}

void LineGeometry::setP2(QVector3D newP2)
{
	if (m_line.p2 == newP2) {
		return;
	}
	m_line.p2 = newP2;

	emit p2Changed();
}

bool LineGeometry::selected() const
{
	return m_isSelected;
}

void LineGeometry::setSelected(bool selected)
{
	if (m_isSelected == selected) {
		return;
	}
	m_isSelected = selected;
}

StreightLine LineGeometry::toStraightLine() const
{
	return StreightLine::FromLine(m_line);
}

Line LineGeometry::toLine() const
{
	return m_line;
}

QDebug operator<<(QDebug dbg, const Line &line)
{
	QDebugStateSaver stateSaver(dbg);
	dbg.nospace()
	    << QString("Line{%1, %2, %3}")
	           .arg(
	               line.id.toString(QUuid::Id128).first(4), //
	               toString(line.p1),
	               toString(line.p2));
	return dbg;
}

QDebug operator<<(QDebug dbg, const StreightLine &sLine)
{
	QDebugStateSaver stateSaver(dbg);
	dbg << std::format("Line{{A={:.2}, B={:.2}, C={:.2}}}", sLine.A, sLine.B, sLine.C);
	return dbg;
}

QDebug operator<<(QDebug dbg, const LineGeometry &geom)
{
	QDebugStateSaver stateSaver(dbg);
	dbg.nospace()
	    << QString("Line{%1, %2, %3}")
	           .arg(
	               geom.id().toString(QUuid::Id128).first(4),
	               toString(geom.p1()),
	               toString(geom.p2()));
	return dbg;
}

// bool initialize(Qt3DCore::QEntity *parent)
// {
// 	meshRenderer = new Qt3DRender::QGeometryRenderer;
// 	geometry = new Qt3DRender::QGeometry(meshRenderer);

// 	vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
// 	indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);

// 	int lineSize = 4;
// 	int hLineSize = ((qAbs(netX1 - netX0) / netMajorStep) + 1) * lineSize * 3;
// 	int vLineSize = ((qAbs(netZ1 - netZ0) / netMajorStep) + 1) * lineSize * 3;
// 	int vertexNum = hLineSize + vLineSize;

// 	float* vertexRawData = new float[vertexNum];
// 	int idx = 0;
// 	QColor majorColor = QColor(220,220,220);
// 	QColor minorColor = QColor(243,243,243);
// 	for(float x = netX0; x <= netX1; x += netMajorStep)
// 	{
// 		vertexRawData[idx++] = x; vertexRawData[idx++] = netY; vertexRawData[idx++] = netZ0;
// 		vertexRawData[idx++] = majorColor.redF(); vertexRawData[idx++] = majorColor.greenF();
// vertexRawData[idx++] = majorColor.blueF(); 		vertexRawData[idx++] = x; vertexRawData[idx++] =
// netY; vertexRawData[idx++] = netZ1; 		vertexRawData[idx++] = majorColor.redF();
// vertexRawData[idx++] = majorColor.greenF(); vertexRawData[idx++] = majorColor.blueF();
// 	}

// 	for(float z = netZ0; z <= netZ1; z += netMajorStep)
// 	{
// 		vertexRawData[idx++] = netX0; vertexRawData[idx++] = netY; vertexRawData[idx++] = z;
// 		vertexRawData[idx++] = majorColor.redF(); vertexRawData[idx++] = majorColor.greenF();
// vertexRawData[idx++] = majorColor.blueF(); 		vertexRawData[idx++] = netX1; vertexRawData[idx++]
// = netY; vertexRawData[idx++] = z; 		vertexRawData[idx++] = majorColor.redF();
// vertexRawData[idx++] = majorColor.greenF(); vertexRawData[idx++] = majorColor.blueF();
// 	}

// 	QByteArray ba;
// 	int bufferSize = vertexNum * sizeof(float);
// 	ba.resize(bufferSize);
// 	memcpy(ba.data(), reinterpret_cast<const char*>(vertexRawData), bufferSize);
// 	vertexDataBuffer->setData(ba);

// 	int stride = 6 * sizeof(float);

// 	       // Attributes
// 	Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute();
// 	positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
// 	positionAttribute->setBuffer(vertexDataBuffer);
// 	positionAttribute->setDataType(Qt3DRender::QAttribute::Float);
// 	positionAttribute->setDataSize(3);
// 	positionAttribute->setByteOffset(0);
// 	positionAttribute->setByteStride(stride);
// 	positionAttribute->setCount(vertexNum / 2);
// 	positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

// 	Qt3DRender::QAttribute *colorAttribute = new Qt3DRender::QAttribute();
// 	colorAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
// 	colorAttribute->setBuffer(vertexDataBuffer);
// 	colorAttribute->setDataType(Qt3DRender::QAttribute::Float);
// 	colorAttribute->setDataSize(3);
// 	colorAttribute->setByteOffset(3 * sizeof(float));
// 	colorAttribute->setByteStride(stride);
// 	colorAttribute->setCount(vertexNum / 2);
// 	colorAttribute->setName(Qt3DRender::QAttribute::defaultColorAttributeName());

// 	geometry->addAttribute(positionAttribute);
// 	geometry->addAttribute(colorAttribute);

// 	meshRenderer->setInstanceCount(1);
// 	meshRenderer->setIndexOffset(0);
// 	meshRenderer->setFirstInstance(0);
// 	meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
// 	meshRenderer->setGeometry(geometry);
// 	meshRenderer->setVertexCount(vertexNum / 2);

// 	material = new Qt3DExtras::QPerVertexColorMaterial(parentEntity);
// 	transform = new Qt3DCore::QTransform;
// 	transform->setScale(1.0f);

// 	Qt3DCore::QEntity *entity = new Qt3DCore::QEntity(parentEntity);
// 	entity->addComponent(meshRenderer);
// 	entity->addComponent(transform);
// 	entity->addComponent(material);

// 	entity->setParent(parentEntity);

// 	return true;
// }
