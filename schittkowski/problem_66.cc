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
    namespace problem66
    {
      struct ExpectedResult
      {
	static const double f0;
	static const double x[];
	static const double fx;
      };
      const double ExpectedResult::f0 = 0.58;
      const double ExpectedResult::x[] = {0.1841264879, 1.202167873,
                                          3.327322322};
      const double ExpectedResult::fx = 0.5181632741;

      template <typename T>
      class F : public GenericDifferentiableFunction<T>
      {
      public:
	ROBOPTIM_DIFFERENTIABLE_FUNCTION_FWD_TYPEDEFS_
	(GenericDifferentiableFunction<T>);

	explicit F ();
	void
	impl_compute (result_t& result, const argument_t& x) const;
	void
	impl_gradient (gradient_t& grad, const argument_t& x, size_type)
	  const;
      };

      template <typename T>
      F<T>::F ()
	: GenericDifferentiableFunction<T>
	  (3, 1, "0.2x₂ - 0.8x₀")
      {}

      template <typename T>
      void
      F<T>::impl_compute (result_t& result, const argument_t& x)
	const
      {
	result[0] = 0.2 * x[2] - 0.8 * x[0];
      }

      template <>
      void
      F<EigenMatrixSparse>::impl_gradient
      (gradient_t& grad, const argument_t&, size_type)
	const
      {
	grad.insert (0) = -0.8;
	grad.insert (2) = 0.2;
      }

      template <typename T>
      void
      F<T>::impl_gradient (gradient_t& grad, const argument_t&, size_type)
	const
      {
	grad.setZero ();
	grad (0) = -0.8;
	grad (2) = 0.2;
      }

      template <typename T>
      class G : public GenericDifferentiableFunction<T>
      {
      public:
	ROBOPTIM_DIFFERENTIABLE_FUNCTION_FWD_TYPEDEFS_
	(GenericDifferentiableFunction<T>);

	explicit G ();
	void
	impl_compute (result_t& result, const argument_t& x) const;
	void
	impl_gradient (gradient_t&, const argument_t&, size_type)
	  const {}
	void
	impl_jacobian (jacobian_t& jac, const argument_t& x)
	  const;
      };

      template <typename T>
      G<T>::G ()
	: GenericDifferentiableFunction<T>
	  (3, 2, "x₁ - exp(x₀), x₂ - exp(x₁)")
      {}

      template <typename T>
      void
      G<T>::impl_compute (result_t& result, const argument_t& x)
	const
      {
	result[0] = x[1] - std::exp (x[0]);
	result[1] = x[2] - std::exp (x[1]);
      }

      template <>
      void
      G<EigenMatrixSparse>::impl_jacobian
      (jacobian_t& jac, const argument_t& x) const
      {
	jac.insert (0,0) = -std::exp (x[0]);
	jac.insert (0,1) = 1;

	jac.insert (1,1) = -std::exp (x[1]);
	jac.insert (1,2) = 1;
      }

      template <typename T>
      void
      G<T>::impl_jacobian
      (jacobian_t& jac, const argument_t& x) const
      {
	jac.setZero ();

	jac (0,0) = -std::exp (x[0]);
	jac (0,1) = 1;

	jac (1,1) = -std::exp (x[1]);
	jac (1,2) = 1;
      }
    } // end of namespace problem66.
  } // end of namespace schittkowski.
} // end of namespace roboptim.

BOOST_FIXTURE_TEST_SUITE (schittkowski, TestSuiteConfiguration)

BOOST_AUTO_TEST_CASE (schittkowski_problem66)
{
  using namespace roboptim;
  using namespace roboptim::schittkowski::problem66;

  // Tolerances for Boost checks.
  double f0_tol = 1e-4;
  double x_tol = 1e-4;
  double f_tol = 1e-4;

  // Build problem.
  F<functionType_t> f;
  solver_t::problem_t problem (f);

  problem.argumentBounds ()[0] = F<functionType_t>::makeInterval (0., 100.);
  problem.argumentBounds ()[1] = F<functionType_t>::makeInterval (0., 100.);
  problem.argumentBounds ()[2] = F<functionType_t>::makeInterval (0., 10.);

  boost::shared_ptr<G<functionType_t> > g =
    boost::make_shared<G<functionType_t> > ();

  solver_t::problem_t::intervals_t intervals;
  intervals.push_back (G<functionType_t>::makeLowerInterval (0.));
  intervals.push_back (G<functionType_t>::makeLowerInterval (0.));
  solver_t::problem_t::scales_t scales
    (static_cast<std::size_t> (g->outputSize ()), 1.);

  problem.addConstraint (g, intervals, scales);

  F<functionType_t>::argument_t x (f.inputSize ());
  x << 0, 1.05, 2.9;
  problem.startingPoint () = x;

  BOOST_CHECK_SMALL_OR_CLOSE (f (x)[0], ExpectedResult::f0, f0_tol);

  std::cout << f.inputSize () << std::endl;
  std::cout << problem.function ().inputSize () << std::endl;

  // Initialize solver.
  SolverFactory<solver_t> factory (SOLVER_NAME, problem);
  solver_t& solver = factory ();
  // Set optimization logger
  SET_OPTIMIZATION_LOGGER (solver, "schittkowski/problem-66");

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
