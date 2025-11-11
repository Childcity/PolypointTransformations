#pragma once

#include <QQmlEngine>
#include <QSphereMesh>
#include <QUuid>
#include <QVector3D>

struct StreightLine;
struct Line;

using Mesh = std::vector<Line>;
using MeshList = std::vector<Mesh>;
using BasisList = std::vector<QVector3D>;

using LineId = QUuid;
using LineIdList = std::vector<LineId>;

template<typename T, typename... Rest>
void hash_combine(std::size_t &seed, const T &v, const Rest &...rest)
{
	seed ^= std::hash<T>{}(v) + 0x9e37'79b9 + (seed << 6) + (seed >> 2);
	(hash_combine(seed, rest), ...);
}

template<>
struct std::hash<QVector3D>
{
	std::size_t operator()(const QVector3D &v) const noexcept
	{
		std::size_t h = 0;
		hash_combine(h, v.x(), v.y(), v.z());
		return h;
	}
};

struct Line
{
	QUuid id;
	QVector3D p1;
	QVector3D p2;

	static Line FromStreightLine(StreightLine line);

	StreightLine toStraightLine() const;

	// size_t id() const;
	bool isNull() const;
};

template<>
struct std::hash<Line>
{
	std::size_t operator()(const Line &l) const noexcept
	{
		std::size_t h = 0;
		hash_combine(h, l.p1);
		return h;
	}
};

struct StreightLine
{
	double A{}; // Ax
	double B{}; // By
	double C{}; // C

	template<typename Container>
	    requires (std::ranges::input_range<Container>)
	static StreightLine FromArray(Container arr)
	{
		assert(arr.size() == 3);
		auto it = arr.begin();
		return {*it++, *it++, *it};
	}

	static StreightLine FromLine(Line line);

	StreightLine &operator*=(double cnst)
	{
		A *= cnst;
		B *= cnst;
		C *= cnst;
		return *this;
	}

	StreightLine &operator+=(double cnst)
	{
		A += cnst;
		B += cnst;
		C += cnst;
		return *this;
	}

	// clang-format off

	auto toTuple() const { return std::tuple{A, B, C}; }

	// clang-format on

	bool isNull() const;
	double length() const;

	bool isNormilized() const;
	StreightLine normilized() const;

	double signDistanceToPoint(QVector3D p) const;

	QVector3D intersect(StreightLine other);
};

class LineGeometry : public QObject
{
	Q_OBJECT
	QML_NAMED_ELEMENT(LineGeometry)
	Q_DISABLE_COPY(LineGeometry)

	Q_PROPERTY(QVector3D p1 READ p1 WRITE setP1 NOTIFY p1Changed FINAL)
	Q_PROPERTY(QVector3D p2 READ p2 WRITE setP2 NOTIFY p2Changed FINAL)

public:
	explicit LineGeometry() = default;
	explicit LineGeometry(StreightLine line);
	explicit LineGeometry(Line line);
	~LineGeometry() override = default;

	LineId id() const;

	QVector3D p1() const;
	void setP1(QVector3D newP1);

	QVector3D p2() const;
	void setP2(QVector3D newP2);

	bool selected() const;
	void setSelected(bool selected);

	StreightLine toStraightLine() const;
	Line toLine() const;

signals:
	void p1Changed();
	void p2Changed();

private:
	Line m_line;
	bool m_isSelected = false;
};

QDebug operator<<(QDebug, const Line &);
QDebug operator<<(QDebug, const StreightLine &);
QDebug operator<<(QDebug, const LineGeometry &);
