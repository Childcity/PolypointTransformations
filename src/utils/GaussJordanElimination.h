#pragma once

#include <stdexcept>
#include <vector>

namespace gauss_jordan_elimination {

using Matrix = std::vector<std::vector<double>>;

// a - matrix Ab
// n - order of Matrix(n)
/*
    Example b:
    double a[,] = {{ 0, 2, 1, 4 },
                  { 1, 1, 2, 6 },
                  { 2, 1, 1, 7 }};
    b = 4, 6, 7
 */
std::vector<double> SolveSystem(const Matrix &a);

class NoSolutionError : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};

class InfiniteSolutionError : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};

} // namespace gauss_jordan_elimination
