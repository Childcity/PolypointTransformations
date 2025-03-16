#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

#include <Eigen/Dense>

using namespace std;
namespace fs = std::filesystem;

namespace geom {

double eps = 1e-9;
double reg_term = 1e-6;

bool areEqual(double a, double b)
{
	return std::abs(a - b) < eps;
}

// struct Point
// {
// 	double x, y, z;

// 	bool operator==(const Point &other) const
// 	{
// 		return std::abs(x - other.x) < 1e-9 && std::abs(y - other.y) < 1e-9
// 		       && std::abs(z - other.z) < 1e-9;
// 	}
// };

struct Plane
{
	int id;
	double a, b, c, d;

	double sign_distance(const Eigen::Vector3d &point) const
	{
		return a * point.x() + b * point.y() + c * point.z() + d;
	}
};

Plane getPolypointPlane(
    const Plane &plane,
    const std::vector<Eigen::Vector3d> &origBasises,
    const std::vector<Eigen::Vector3d> &resBasises)
{
	double a1 = 0, b1 = 0, c1 = 0, d1 = 0, r1 = 0;
	double b2 = 0, c2 = 0, d2 = 0, r2 = 0;
	double c3 = 0, d3 = 0, r3 = 0;
	double d4 = 0, r4 = 0;

	for (size_t i = 0; i < origBasises.size(); ++i) {
		const Eigen::Vector3d &orig_basis_p = origBasises[i];
		const Eigen::Vector3d &res_basis_p = resBasises[i];

		double gamma = plane.sign_distance(orig_basis_p);
		double gamma_squared = gamma * gamma;

		double x = res_basis_p.x();
		double y = res_basis_p.y();
		double z = res_basis_p.z();

		a1 += x * x / gamma_squared;
		b1 += x * y / gamma_squared;
		c1 += x * z / gamma_squared;
		d1 += x / gamma_squared;

		b2 += y * y / gamma_squared;
		c2 += y * z / gamma_squared;
		d2 += y / gamma_squared;

		c3 += z * z / gamma_squared;
		d3 += z / gamma_squared;

		d4 += 1 / gamma_squared;

		r1 += x / gamma;
		r2 += y / gamma;
		r3 += z / gamma;
		r4 += 1 / gamma;
	}

	Eigen::Matrix4d A;
	A << a1 + reg_term, b1, c1, d1, //
	    b1, b2 + reg_term, c2, d2, //
	    c1, c2, c3 + reg_term, d3, //
	    d1, d2, d3, d4 + reg_term;

	Eigen::Vector4d B(r1, r2, r3, r4);
	Eigen::Vector4d X = A.lu().solve(B);

	return {plane.id, X(0), X(1), X(2), X(3)};
}

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
		inPlanes.emplace_back(0, a, b, c, d);
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
	cout << format("Last plane: {} {} {} {}\n", inPlanes.back().a, inPlanes.back().b, inPlanes.back().c, inPlanes.back().d);
	// clang-format on

	timer = ElapsedTimer{};
	for (auto plane : inPlanes) {
		getPolypointPlane(plane, {{1, 1, 1}, {2, 2, 2}}, {{2, 2, 2}, {1, 1, 1}});
	}
	cout << format("Deformation took: {}\n", timer.elapsedSec());

	return 0;
}
