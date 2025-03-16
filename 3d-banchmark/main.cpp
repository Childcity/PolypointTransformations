#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <tuple>

using namespace std;
namespace fs = std::filesystem;

namespace geom {

double eps = 1e-9;

bool areEqual(double a, double b)
{
	return std::abs(a - b) < eps;
}

struct Point
{
	double x, y, z;

	bool operator==(const Point &other) const
	{
		return std::abs(x - other.x) < 1e-9 && std::abs(y - other.y) < 1e-9
		       && std::abs(z - other.z) < 1e-9;
	}
};

struct Plane
{
	double A, B, C, D;

	Plane(double a, double b, double c, double d)
	    : A(a)
	    , B(b)
	    , C(c)
	    , D(d)
	{
	}
};

} // namespace geom

struct ElapsedTimer
{
	chrono::high_resolution_clock::time_point start;

	ElapsedTimer()
	    : start(chrono::high_resolution_clock::now())
	{
	}

	auto elapsedSec()
	{
		chrono::duration<double> dur = chrono::high_resolution_clock::now() - start;
		return dur.count();
	}
};

auto readPlanes(auto filePath)
{
	vector<geom::Plane> inPlanes;
	inPlanes.reserve(1'000'000);

	ifstream file{filePath};
	for (double a, b, c, d; file >> a >> b >> c >> d;) {
		inPlanes.emplace_back(a, b, c, d);
	}

	return inPlanes;
}

int main()
{
	auto currPath = fs::current_path();
	auto planesFile = currPath.append("in_planes.txt");
	cout << format("Planes file: {}\n", planesFile.string());

	auto timer = ElapsedTimer{};
	auto inPlanes = readPlanes(planesFile);

	// clang-format off
	cout << format("Read {} planes in {} seconds\n", inPlanes.size(), timer.elapsedSec());
	cout << format("Last plane: {} {} {} {}\n", inPlanes.back().A, inPlanes.back().B, inPlanes.back().C, inPlanes.back().D);
	// clang-format on

	return 0;
}
