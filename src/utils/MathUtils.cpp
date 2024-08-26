#include "MathUtils.h"

#include <bitset>
#include <ranges>

#include <QLoggingCategory>
#include <QQmlEngine>

#include <geometry/GeometryPrimitives.h>

#include "GaussJordanElimination.h"

namespace {

namespace v = std::views;

Q_LOGGING_CATEGORY(polydot_line, "utils.polydot_line", QtInfoMsg)
Q_LOGGING_CATEGORY(polydot_mesh, "utils.polydot_mesh", QtInfoMsg)

constexpr double eps = std::numeric_limits<double>().epsilon();

constexpr bool fuzzyCompare(auto p1, auto p2)
{
	return (std::abs(p1 - p2) * 100000. <= std::min(qAbs(p1), qAbs(p2)));
}

//  By|          .
//  Ay| |B|   .       .
//    |    .     .
//    |  .  .
//  1 |.        |A|
//    |_________________
//     1        Bx    Ax

// Dot Product
// https://math.stackexchange.com/questions/805954/what-does-the-dot-product-of-two-vectors-represent
// Ax*Bx + Ay*By = |A||B|cos(th)

double signDistanceToLine(
    QVector3D point, // Point
    QVector3D linePoint, // Point on the Line
    QVector3D direction // Line direction vector
)
{
	auto signLength = [](QVector3D vec) {
		// TODO: recheck if it is correct way to get a signA!
		auto sign = vec.x() + vec.y() + vec.z() < eps ? -1.f : 1.f;
		return sign * std::hypot(vec.x(), vec.y(), vec.z());
	};

	if (direction.isNull()) {
		auto delta = point - linePoint;
		return signLength(delta);
	}

	QVector3D p = linePoint + QVector3D::dotProduct(point - linePoint, direction) * direction;
	auto delta = point - p;
	return signLength(delta);
}

} // namespace

MathUtils &MathUtils::Get()
{
	static MathUtils *pthis;
	static std::once_flag flag;

	std::call_once(flag, [] {
		pthis = new MathUtils; //
	});

	return *pthis;
}

QObject *MathUtils::Get(QQmlEngine *, QJSEngine *)
{
	return &MathUtils::Get();
}

MathUtils::MathUtils(QObject *parent)
    : QObject(parent)
{
}

StreightLine MathUtils::getPolydotTransformedLine(
    const Line &baseLine, const QVariantList &origBasises, const QVariantList &resBasises)
{
	namespace solve = gauss_jordan_elimination;

	double a1{}, b1{}, c1{}, r1{}, //
	    a2{}, b2{}, c2{}, r2{}, //
	    a3{}, b3{}, c3{}, r3{};

	// https://brilliant.org/wiki/3d-coordinate-geometry-equation-of-a-line/
	// Subtracting the position vectors of the two points gives the direction vector
	const StreightLine baseLineNormilized = baseLine.toStraightLine().normilized();
	QVector3D baseLineDirection = baseLine.p1 - baseLine.p2;
	baseLineDirection.normalize();

	{
		qCDebug(polydot_line) << "Base Normilized Line: " << baseLineNormilized;
		qCDebug(polydot_line).noquote() << std::format(
		    "\n       x - {}      y - {}     z - {} "
		    "\nLine: -------- = -------- = -------- "
		    "\n       {:.2}       {:.2}      {:.2}  ",
		    baseLine.p1.x(),
		    baseLine.p1.y(),
		    baseLine.p1.z(),
		    baseLineDirection.x(),
		    baseLineDirection.y(),
		    baseLineDirection.z() //
		);
	}

	for (const auto &[originalBasis, resultBasis] : std::views::zip(origBasises, resBasises)) {
		const auto &origBasis = originalBasis.value<QVector3D>();
		const auto &resBasis = resultBasis.value<QVector3D>();

		double betta = baseLineNormilized.signDistanceToPoint(origBasis);
		double betta2 = signDistanceToLine(origBasis, baseLine.p1, baseLineDirection);
		double betta3 = signDistanceToLine(origBasis, baseLine.p2, baseLineDirection);

		if (qFuzzyIsNull(betta)) {
			// If distance from origBasis to Line is 0 => we cannot solve system!
			return {};
		}

		qCDebug(polydot_line) << "Original Basis" << origBasis;
		qCDebug(polydot_line) << "Result   Basis" << resBasis;
		qCDebug(polydot_line)
		    << "Betta" << std::format("{:.10} {:.10} {:.10}", betta, betta2, betta3);
		qCDebug(polydot_line) << "-----------------------------------------";

		// assert(fuzzyCompare(betta, betta2));
		// assert(fuzzyCompare(betta, betta3));

		double bettaSqure = betta * betta;

		double x = resBasis.x();
		double y = resBasis.y();

		a1 += x * x / bettaSqure;
		b1 += x * y / bettaSqure;
		c1 += x / bettaSqure;
		r1 += x / betta;

		a2 += y * x / bettaSqure;
		b2 += y * y / bettaSqure;
		c2 += y / bettaSqure;
		r2 += y / betta;

		a3 += x / bettaSqure;
		b3 += y / bettaSqure;
		c3 += 1. / bettaSqure;
		r3 += 1. / betta;
	}

	const solve::Matrix polidotEquestion = {
	    {a1, b1, c1, r1},
	    {a2, b2, c2, r2},
	    {a3, b3, c3, r3},
	};

	try {
		const auto polidotSolutionArr = solve::SolveSystem(polidotEquestion);
		assert(polidotSolutionArr.size() == 3);

		const auto resLine = StreightLine::FromArray(polidotSolutionArr);
		qCDebug(polydot_line) << "Result line" << resLine;

		return resLine;
	} catch (const std::runtime_error &e) {
		qCWarning(polydot_line) << "System cannot be solved: " << e.what();
	}

	return {};
}

// Apply Polidot Transformations for each line (Result is StreightLine).
// Find intersecations between StreightLines.
// Compose new lines from points, received from p2
Mesh MathUtils::getPolydotTransformedMesh(
    Mesh inMesh, const QVariantList &origBasises, const QVariantList &resBasises)
{
	assert(!inMesh.empty());
	// assert(inMesh.size() % 2 == 0);

	//	Simple Mesh of 4 lines:
	//
	//	            line2
	//         p1            p2
	//           ____________
	//	        |           |
	//	        |           |
	//    line1 |           |  line3
	//	        |           |
	//	        |___________|
	//        p4             p3
	//              line4

	// To find last intersection point, copy first line to the end
	// Now: line1, line2, line3, line4, line1
	inMesh.emplace_back(inMesh.front());

	std::vector<bool> linesWithSwappedBeginEnd(inMesh.size(), false);

	std::vector<QVector3D> linesIntersections;
	StreightLine leftTransformedLine = MathUtils::getPolydotTransformedLine(
	    inMesh.front(), origBasises, resBasises); // line1

	for (int i = 1; auto &line : inMesh | v::drop(1)) { // Skip line1
		StreightLine rightTransformedLine = MathUtils::getPolydotTransformedLine(
		    line, origBasises, resBasises);

		try {
			QVector3D intersectionPoint = leftTransformedLine.intersect(rightTransformedLine);
			linesIntersections.emplace_back(intersectionPoint);
		} catch (...) {
			qCWarning(polydot_mesh)
			    << "Failed to find intersection of" << leftTransformedLine << rightTransformedLine
			    << std::uncaught_exceptions();
			// try {
			//	rightTransformedLine *= 10;
			//	QVector3D intersectionPoint = leftTransformedLine.intersect(rightTransformedLine);
			//	linesIntersections.emplace_back(intersectionPoint);
			// } catch (...) {
			//	qCCritical(polydot_mesh)
			//	    << "Failed to find intersection of" << leftTransformedLine
			//	    << rightTransformedLine;
			// }
		}
		leftTransformedLine = rightTransformedLine;

		if (inMesh[i - 1].p2 == inMesh[i].p2) {
			linesWithSwappedBeginEnd[i] = true;
		}
		i++;
	}

	if (linesIntersections.empty()) {
		qCCritical(polydot_mesh) << "Failed to apply Polidot transformations for the mesh!";
		return {};
	}

	Mesh outMesh;
	QVector3D leftPoint = linesIntersections.back(); // p4
	for (const auto &[rightPoint, inLine, shouldSwap] :
	     v::zip(linesIntersections, inMesh, linesWithSwappedBeginEnd) //
	) {
		// Create and add new line
		if (shouldSwap) {
			outMesh.emplace_back(inLine.id, rightPoint, leftPoint);
		} else {
			outMesh.emplace_back(inLine.id, leftPoint, rightPoint);
			leftPoint = rightPoint;
		}
	}

	// assert((inMesh.size() - 1) == outMesh.size());
	if ((inMesh.size() - 1) != outMesh.size()) {
		outMesh = {inMesh.begin(), inMesh.end() - 1};
	}

	return outMesh;
}

Mesh MathUtils::getPolydotTransformedStreightLineMesh(
    Mesh inMesh, const QVariantList &origBasises, const QVariantList &resBasises)
{
	Mesh outMesh;

	for (const auto &inLine : inMesh) {
		const auto outStreightLine = MathUtils::getPolydotTransformedLine(
		    inLine, origBasises, resBasises);

		auto outLine = Line::FromStreightLine(outStreightLine);
		if (!outLine.isNull()) {
			outMesh.emplace_back(std::move(outLine));
		}
	}

	// assert((inMesh.size()) == outMesh.size());
	return outMesh;
}

QVector3D MathUtils::mouseEventToSpace(
    const Qt3DInput::QMouseEvent *mouseEvent, const Qt3DRender::QCamera *camera, QSize surfaceSize)
{
	const int areaWidth = surfaceSize.width();
	const int areaHeight = surfaceSize.height();
	const QPointF glCorrectSurfacePosition = {
	    static_cast<float>(mouseEvent->x()), areaHeight - static_cast<float>(mouseEvent->y())};

	const QMatrix4x4 viewMatrix = {camera->viewMatrix()};
	const QMatrix4x4 projectionMatrix = {camera->lens()->projectionMatrix()};

	const auto relativeViewport = QRectF(0.0f, 0.0f, 1.0f, 1.0f);
	const auto viewport = QRectF(
	    relativeViewport.x() * areaWidth,
	    (1.0 - relativeViewport.y() - relativeViewport.height()) * areaHeight,
	    relativeViewport.width() * areaWidth,
	    relativeViewport.height() * areaHeight);

	const auto nearPos =
	    QVector3D{
	        static_cast<float>(glCorrectSurfacePosition.x()),
	        static_cast<float>(glCorrectSurfacePosition.y()),
	        0.0f}
	        .unproject(viewMatrix, projectionMatrix, viewport.toRect());
	return nearPos;
}
