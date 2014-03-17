// Copyright (C) 2014 by Benjamin Chretien, CNRS.
//
// This file is part of the roboptim.
//
// roboptim is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// roboptim is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with roboptim.  If not, see <http://www.gnu.org/licenses/>.

#include "common.hh"

namespace roboptim
{
  namespace schittkowski
  {
    namespace problem48
    {
      struct ExpectedResult
      {
	static const double f0;
	static const double x[];
	static const double fx;
      };
      const double ExpectedResult::f0 = 84.;
      const double ExpectedResult::x[] = {1, 1, 1, 1, 1};
      const double ExpectedResult::fx = 0;

      template <typename T>
      class F : public GenericDifferentiableFunction<T>
      {
      public:
	ROBOPTIM_DIFFERENTIABLE_FUNCTION_FWD_TYPEDEFS_
	(GenericDifferentiableFunction<T>);

	explicit F () throw ();
	void
	impl_compute (result_t& result, const argument_t& x) const throw ();
	void
	impl_gradient (gradient_t& grad, const argument_t& x, size_type)
	  const throw ();
      };

      template <typename T>
      F<T>::F () throw ()
	: GenericDifferentiableFunction<T>
	  (5, 1, "(x₀ - 1)² + (x₁ - x₂)² + (x₃ - x₄)²")
      {}

      template <typename T>
      void
      F<T>::impl_compute (result_t& result, const argument_t& x)
	const throw ()
      {
	result[0] = std::pow (x[0] - 1, 2)
	  + std::pow (x[1] - x[2], 2)
	  + std::pow (x[3] - x[4], 2);
      }

      template <>
      void
      F<EigenMatrixSparse>::impl_gradient
      (gradient_t& grad, const argument_t& x, size_type)
	const throw ()
      {
	grad.insert (0) = 2 * (x[0] - 1);
	grad.insert (1) = 2 * (x[1] - x[2]);
	grad.insert (2) = 2 * (-x[1] + x[2]);
	grad.insert (3) = 2 * (x[3] - x[4]);
	grad.insert (4) = 2 * (-x[3] + x[4]);
      }

      template <typename T>
      void
      F<T>::impl_gradient (gradient_t& grad, const argument_t& x, size_type)
	const throw ()
      {
	grad[0] = 2 * (x[0] - 1);
	grad[1] = 2 * (x[1] - x[2]);
	grad[2] = 2 * (-x[1] + x[2]);
	grad[3] = 2 * (x[3] - x[4]);
	grad[4] = 2 * (-x[3] + x[4]);
      }

      template <typename T>
      class G : public GenericDifferentiableFunction<T>
      {
      public:
	ROBOPTIM_DIFFERENTIABLE_FUNCTION_FWD_TYPEDEFS_
	(GenericDifferentiableFunction<T>);

	explicit G () throw ();
	void
	impl_compute (result_t& result, const argument_t& x) const throw ();
	void
	impl_gradient (gradient_t&, const argument_t&, size_type)
	  const throw () {}
	void
	impl_jacobian (jacobian_t& jac, const argument_t& x)
	  const throw ();
      };

      template <typename T>
      G<T>::G () throw ()
	: GenericDifferentiableFunction<T>
	  (5, 2, "x₀ + x₁ + x₂ + x₃ + x₄, x₂ - 2(x₃ + x₄)")
      {}

      template <typename T>
      void
      G<T>::impl_compute (result_t& result, const argument_t& x)
	const throw ()
      {
	result[0] = x[0] + x[1] + x[2] + x[3] + x[4];
	result[1] = x[2] - 2 * (x[3] + x[4]);
      }

      template <>
      void
      G<EigenMatrixSparse>::impl_jacobian
      (jacobian_t& jac, const argument_t&) const throw ()
      {
	jac.insert (0,0) = 1;
	jac.insert (0,1) = 1;
	jac.insert (0,2) = 1;
	jac.insert (0,3) = 1;
	jac.insert (0,4) = 1;

	jac.insert (1,2) = 1;
	jac.insert (1,3) = -2;
	jac.insert (1,4) = -2;
      }

      template <typename T>
      void
      G<T>::impl_jacobian
      (jacobian_t& jac, const argument_t&) const throw ()
      {
	jac.setZero ();

	jac (0,0) = 1;
	jac (0,1) = 1;
	jac (0,2) = 1;
	jac (0,3) = 1;
	jac (0,4) = 1;

	jac (1,2) = 1;
	jac (1,3) = -2;
	jac (1,4) = -2;
      }
    } // end of namespace problem48.
  } // end of namespace schittkowski.
} // end of namespace roboptim.

BOOST_FIXTURE_TEST_SUITE (schittkowski, TestSuiteConfiguration)

BOOST_AUTO_TEST_CASE (schittkowski_problem48)
{
  using namespace roboptim;
  using namespace roboptim::schittkowski::problem48;

  // Tolerances for Boost checks.
  double f0_tol = 1e-4;
  double x_tol = 1e-4;
  double f_tol = 1e-4;

  // Build problem.
  F<functionType_t> f;
  solver_t::problem_t problem (f);

  boost::shared_ptr<G<functionType_t> > g =
    boost::make_shared<G<functionType_t> > ();

  solver_t::problem_t::intervals_t intervals;
  intervals.push_back (G<functionType_t>::makeInterval (5., 5.));
  intervals.push_back (G<functionType_t>::makeInterval (-3., -3.));
  solver_t::problem_t::scales_t scales (g->outputSize (), 1.);

  problem.addConstraint (g, intervals, scales);

  F<functionType_t>::argument_t x (5);
  x << 3, 5, -3, 2, -2;
  problem.startingPoint () = x;

  BOOST_CHECK_SMALL_OR_CLOSE (f (x)[0], ExpectedResult::f0, f0_tol);

  std::cout << f.inputSize () << std::endl;
  std::cout << problem.function ().inputSize () << std::endl;

  // Initialize solver.
  SolverFactory<solver_t> factory (SOLVER_NAME, problem);
  solver_t& solver = factory ();
  OptimizationLogger<solver_t> logger
    (solver,
     "/tmp/roboptim-shared-tests/" SOLVER_NAME "/schittkowski/problem-48");

  // Set optional log file for debugging
  SET_LOG_FILE(solver);

  std::cout << f.inputSize () << std::endl;
  std::cout << problem.function ().inputSize () << std::endl;

  // Compute the minimum and retrieve the result.
  solver_t::result_t res = solver.minimum ();

  std::cout << f.inputSize () << std::endl;
  std::cout << problem.function ().inputSize () << std::endl;

  // Display solver information.
  std::cout << solver << std::endl;

  // Process the result
  PROCESS_RESULT();
}

BOOST_AUTO_TEST_SUITE_END ()