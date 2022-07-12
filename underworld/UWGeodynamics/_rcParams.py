# Configuration file
from __future__ import print_function,  absolute_import
from underworld.scaling import units as u
from ._validate import *


rcParams =  {

    "CFL": [0.5, validate_float],

    "rebuild.solver": [False, validate_bool],
    "initial.nonlinear.tolerance": [1e-2, validate_float],
    "nonlinear.tolerance": [1e-2, validate_float],
    "initial.nonlinear.min.iterations": [2, validate_int],
    "initial.nonlinear.max.iterations": [500, validate_int],
    "nonlinear.min.iterations": [2, validate_int],
    "nonlinear.max.iterations": [500, validate_int],

    "default.outputs" : [["temperature",
                          "pressureField",
                          "strainRateField",
                          "velocityField",
                          "projStressField",
                          "projTimeField",
                          "projMaterialField",
                          "projViscosityField",
                          "projStressField",
                          "projMeltField",
                          "projPlasticStrain",
                          "projDensityField"], validate_stringlist],

    "swarm.particles.per.cell.2D": [40, validate_int],
    "swarm.particles.per.cell.3D": [120, validate_int],

    "popcontrol.aggressive" : [True, validate_bool],
    "popcontrol.split.threshold" : [0.15, validate_float],
    "popcontrol.max.splits" : [10, validate_int],
    "popcontrol.particles.per.cell.2D" : [40, validate_int],
    "popcontrol.particles.per.cell.3D" : [120, validate_int],

    "time.SIunits": [u.years, validate_quantity],
    "timeField.SIunits": [u.years, validate_quantity],
    "viscosityField.SIunits" : [u.pascal * u.second, validate_quantity],
    "densityField.SIunits" : [u.kilogram / u.metre**3, validate_quantity],
    "velocityField.SIunits" : [u.centimeter / u.year, validate_quantity],
    "temperature.SIunits" : [u.degK, validate_quantity],
    "pressureField.SIunits" : [u.pascal , validate_quantity],
    "projStressTensor.SIunits" : [u.pascal , validate_quantity],
    "projStressField.SIunits" :  [u.pascal , validate_quantity],
    "strainRateField.SIunits" : [1.0 / u.second, validate_quantity],
    "projViscosityField.SIunits"  : [u.pascal * u.second, validate_quantity],
    "projDensityField.SIunits" : [u.kilogram / u.metre**3, validate_quantity],
    "projTimeField.SIunits" : [u.megayears, validate_quantity],

    "shear.heating": [False, validate_bool],
    "surface.pressure.normalization": [True, validate_bool],
    "pressure.smoothing": [True, validate_bool],
    "advection.diffusion.method": ["SUPG", validate_string],
    "rheologies.combine.method": ["Minimum", validate_string],
    "averaging.method": ["arithmetic", validate_averaging]
}

