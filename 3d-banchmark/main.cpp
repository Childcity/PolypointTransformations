#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
#include <ranges>
#include <thread>

#include <Eigen/Dense>

using namespace std;
namespace fs = filesystem;

namespace geom {

constexpr double reg_term = 1e-6;

struct Plane
{
	int id;
	double a, b, c, d;

	double signDistance(const Eigen::Vector3d &point) const
	{
		return a * point.x() + b * point.y() + c * point.z() + d;
	}
};

Plane getPolypointPlane(
    const Plane &plane,
    const vector<Eigen::Vector3d> &origBasises,
    const vector<Eigen::Vector3d> &resBasises)
{
	double a1 = 0, b1 = 0, c1 = 0, d1 = 0, r1 = 0;
	double b2 = 0, c2 = 0, d2 = 0, r2 = 0;
	double c3 = 0, d3 = 0, r3 = 0;
	double d4 = 0, r4 = 0;

	for (size_t i = 0; i < origBasises.size(); ++i) {
		const Eigen::Vector3d &orig_basis_p = origBasises[i];
		const Eigen::Vector3d &res_basis_p = resBasises[i];

		double gamma = plane.signDistance(orig_basis_p);
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

using PlaneList = vector<geom::Plane>;

} // namespace geom

struct ElapsedTimer
{
	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

	double elapsedSec()
	{
		chrono::duration<double> dur = chrono::high_resolution_clock::now() - start;
		return dur.count();
	}
};

auto readPlanesTxt(auto filePath)
{
	vector<geom::Plane> inPlanes;
	inPlanes.reserve(1'000'000);

	ifstream file{filePath};
	for (double a, b, c, d; file >> a >> b >> c >> d;) {
		inPlanes.emplace_back(0, a, b, c, d);
	}

	return inPlanes;
}

auto readPlanesBin(auto filePath)
{
	// Assume data stores as list of double.
	// Each 4 double are 1 Plane(a, b, c, d).

	// Open the binary file
	ifstream file{filePath, ios::binary};

	streamsize fileSize = [&file] {
		file.seekg(0, ios::end);
		auto size = file.tellg();
		file.seekg(0, ios::beg);
		return size;
	}();

	// Calculate the number of elements
	size_t num_elements = fileSize / sizeof(double);

	// Read the data into a vector
	vector<double> data(num_elements);
	file.read(reinterpret_cast<char *>(data.data()), fileSize);

	vector<geom::Plane> inPlanes;

	// Convert to Planes
	inPlanes.reserve(data.size() / 4);
	for (auto &&chunk : data | views::chunk(4)) {
		inPlanes.emplace_back(0, chunk[0], chunk[1], chunk[2], chunk[3]);
	}

	return inPlanes;
}

void increasePlanesAount(vector<geom::Plane> &planes)
{
	for (int i = 0; i < 8; i++) {
		planes.insert(planes.end(), planes.begin(), planes.end());
	}
	planes.resize(80'000'000);
}

geom::PlaneList serialApproach(auto &inPlanes, auto &basis_in, auto &basis_out)
{
	geom::PlaneList result;
	result.reserve(inPlanes.size());

	auto timer = ElapsedTimer{};
	for (auto plane : inPlanes) {
		result.push_back(getPolypointPlane(plane, basis_in, basis_out));
	}
	cout << format("Serial. Deformation took: {}\n", timer.elapsedSec());
	return result;
}

geom::PlaneList collectThreadResults(
    const geom::PlaneList &inPlanes, const vector<geom::PlaneList> &threadResults)
{
	// Collect results from all threads
	geom::PlaneList result;
	result.reserve(inPlanes.size());
	for (const auto &partialResult : threadResults) {
		result.insert(result.end(), partialResult.begin(), partialResult.end());
	}
	return result;
}

geom::PlaneList threadChunkApproach(
    const geom::PlaneList &inPlanes, const auto &basis_in, const auto &basis_out, auto threadCount)
{
	vector<jthread> threads;
	size_t chunkSize = inPlanes.size() / threadCount;

	auto threadResultsPtr = std::make_unique<vector<geom::PlaneList>>(threadCount);
	auto &threadResults = *threadResultsPtr;

	// Start parallel execution
	for (size_t i = 0; i < threadCount; ++i) {
		auto start = inPlanes.begin() + i * chunkSize;
		auto end = (i == threadCount - 1) ? inPlanes.end() : start + chunkSize;

		threads.emplace_back([&, start, end, i] {
			for (auto &&[_, inPlane] : ranges::subrange(start, end) | views::enumerate) {
				threadResults[i].push_back(getPolypointPlane(inPlane, basis_in, basis_out));
			}
		});
	}

	std::ranges::for_each(threads, &jthread::join);
	return collectThreadResults(inPlanes, threadResults);
}

int main()
{
	constexpr auto runEachExperement = 3;

	auto currPath = fs::current_path();
	auto planesFile = currPath.append("in_planes.npy");
	cout << format("Planes file: {}\n", planesFile.string());

	auto timer = ElapsedTimer{};
	auto inPlanes = readPlanesBin(planesFile);
	increasePlanesAount(inPlanes);

	// clang-format off
	cout << format("Read {} planes in {} seconds\n", inPlanes.size(), timer.elapsedSec());
	cout << format("Last plane: {} {} {} {}\n", inPlanes.back().a, inPlanes.back().b, inPlanes.back().c, inPlanes.back().d);
	cout << endl;

	auto basis_in = vector<Eigen::Vector3d>{
	    {0.0, 0.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, -0.0},
	    {1.0, 0.0, 1.0}, {1.0, 1.0, 1.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, -0.0}};
	auto basis_out = vector<Eigen::Vector3d>{
	    {0.0, 0.0, 1.0},  {0.2, 0.2,   1.0}, {0.0, 0.0,      0.0}, {0.0, 1.0, -0.0},
	    {0.2, -0.2, 1.0}, {1.18, 0.78, 1.0}, {1.0, 6.12e-17, 0.0}, {1.0, 1.0, -0.0}};
	// clang-format on

	// serialApproach(inPlanes, basis_in, basis_out);

	for (size_t threadCount = 16; threadCount <= 16;
	     /*thread::hardware_concurrency();*/ threadCount++) {
		std::vector<double> times;

		for (int i = 0; i < runEachExperement; ++i) {
			ElapsedTimer timer;
			auto result = threadChunkApproach(inPlanes, basis_in, basis_out, threadCount);
			times.push_back(timer.elapsedSec());
		}

		auto elapsedSec = std::accumulate(times.begin(), times.end(), 0.0) / runEachExperement;
		cout << format("{}; {}", threadCount, elapsedSec) << endl;
	}

	return 0;
}
