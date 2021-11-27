# ---
# jupyter:
#   jupytext:
#     text_representation:
#       extension: .py
#       format_name: light
#       format_version: '1.5'
#       jupytext_version: 1.10.3
#   kernelspec:
#     display_name: Python 3
#     language: python
#     name: python3
# ---

# Stokes Solver
# =======
#
# We want to solve the following Stokes block system.
#
# $$
# \begin{bmatrix}
#  K & G \\
#  G^T & C
# \end{bmatrix}
# \begin{bmatrix}
#   u\\
#   p
# \end{bmatrix}
# =
# \begin{bmatrix}
#  f\\ 
#  h
# \end{bmatrix}.
# $$
#
# If we apply Gaussian elimination to the above as a 2x2 block matrix system
# we can write this as:
#
# $$
# \begin{bmatrix}
#   K & G\\
#   0 & S
# \end{bmatrix}
# \begin{bmatrix}
#   u\\
#   p
# \end{bmatrix}
# =
# \begin{bmatrix}
#  f\\ 
#  \hat{h}
# \end{bmatrix},
# $$
#
# where $S=G^{T}K^{-1}G-C$ is the Schur complement and $\hat{h}=G^{T}K^{-1}f -h$.
#
# This system is now solved first for the pressure using the Schur complement matrix, $S$. Then a backsolve for the
# velocity gives the complete solution.
#
# Note that wherever $K^{-1}$ appears, the inverse is never explicitly calculated but is achieved via
# a [PETSc](http://www.mcs.anl.gov/petsc/) solve method. While solving for the pressure, there are necessarily solves using $K$ inside of the matrix $S$.
# We often refer to these as 'inner' solves.
#
# Basic usage of the Stokes solver class involves being able to easily set up the inner solves in a few different ways
# (Setting up the pressure solve is more advanced).
#
# To illustrate some basic usage let's set up a simple problem to solve.

# +
import underworld as uw
from underworld import function as fn

res=128
mesh = uw.mesh.FeMesh_Cartesian(elementRes=(res,res))
velocityField = mesh.add_variable(2)
pressureField = mesh.subMesh.add_variable(1)
velocityField.data[:] = (0.,0.)
pressureField.data[:] = 0.

# We are going to make use of one of the existing analytic solutions so that we may easily
# obtain functions for a viscosity profile and forcing terms.
# Exact solution solCx with defaults
sol = fn.analytic.SolCx()
stokesSystem = uw.systems.Stokes(velocityField,pressureField,sol.fn_viscosity,sol.fn_bodyforce,conditions=sol.get_bcs(velocityField))

# Now we create a solver.
solver=uw.systems.Solver(stokesSystem)

# The Stokes solver will use multigrid as a preconditioner along with
# PETSc's 'fgmres' Krylov method by default for the 'inner' solve.
solver.solve()

# Now let's set up the inner solve to do a direct solve.
# Note that the `lu` direct solver will not work in parallel.
solver.set_inner_method("lu")
solver.solve()
# -

# Let's run underworld's help function on the solver.configure function.

help(solver.set_inner_method)

# We can see all the of the options that are configured in the solver using the `list()` functions for each component of the solver. These are the most useful ones.

print("System Level Options:")
solver.options.main.list()
print("")
print("Schur Complement Solve Options:")
solver.options.scr.list()   # Specifics for the 
print("")
print("Inner (velocity) Solve Options:")
solver.options.A11.list()   # Specifics for the inner (velocity) solve
print("")
print("Multigrid (where enabled) Options:")
solver.options.mg.list()    # Options relevant to multigrid (if chosen)

# Further information about options is available via the `help()` Python function:
#
# ```
# help(solver.options.A11)
# ```
#
# The options generally follow `PETSc` naming conventions. 
#
# A useful trick is to be able to imitate the classic _"penalty method"_ which can be very efficient with modest-sized (2D) problems.  
#
# In the penalty method, we add a term to the weak form of the Stokes equation which penalises $\lambda | \nabla \cdot \mathbf{u}| > 0$ and where $\lambda$ is a sufficiently large constant to enforce the constraint. Typically $10^7$ is considered sufficient. 
#
# The problem with this method is that the condition number of the system is severely compromised by adding the penalty term and standard iterative methods do not work well. 
#
# Our solvers have been configured with the penalty term and, for sufficiently robust choices of the inner solver, this can help solve problems faster (by improving pressure convergence). 
#
# An indestructible solver like `lu` or `mumps` (Mumps is a direct solve that will work in parallel) can use very large penalties. Hence we can recreate the penalty method as follows (though it still follows the pattern of the Schur complement solver, while the classical method takes some shortcuts).

try:
    solver.set_inner_method("mumps")
    solver.options.scr.ksp_type="cg"
    solver.set_penalty(1.0e7)
    solver.solve()
    solver.print_stats()
except RuntimeError:
    # If the above fails, "mumps" probably isn't installed
    pass

# Now let's go back to using multigrid. We can use a penalty here too, but the gigantic numbers won't work.

solver.set_inner_method("mg")
solver.set_penalty(1.0)
solver.solve()
solver.print_stats()
