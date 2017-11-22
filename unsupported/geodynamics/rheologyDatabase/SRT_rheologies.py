from ..scaling import u
import underworld.function as fn


def powerLaw(A, Q, n, Va, R, strainRateFn, pressureFn, temperatureFn,
             DefaultSRInvariant, solutionExist=None, f=1.0):
    if solutionExist.evaluate():
        I = fn.tensor.second_invariant(strainRateFn)
    else:
        I = DefaultSRInvariant
    P = pressureFn
    T = temperatureFn

    return (f * A**(-1.0 / n) * I**((1.0-n)/n) *
            fn.math.exp((Q + P * Va)/(R*T*n)))


SRT_WetQ_Patterson1990 = {"n": 3.1, 
                          "A": 6.60693448e-8 * u.megapascal**(-3.1) / u.second,
                          "Q": 135 * u.kilojoule / u.mole,
                          "Va": 0.0 * u.meter**3 / u.mole,
                          "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_WetQ_Goetze1978    = {"n": 3.0, 
                          "A": 5e-6 * u.megapascal**(-3.0) / u.second,
                          "Q": 190 * u.kilojoule / u.mole,
                          "Va":0.0 * u.meter**3 / u.mole,
                          "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_DryMG_Wang2012     = {"n": 3.2, 
                          "A": 10e-2 * u.megapascal**(-3.2) / u.second,
                          "Q": 244 * u.kilojoule / u.mole,
                          "Va":0.0 * u.meter**3 / u.mole,
                          "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_WOl_Hirth2003      = {"n": 3.5, 
                          "A": 1600 * u.megapascal**(-3.5) / u.second,
                          "Q": 520 * u.kilojoule / u.mole,
                          "Va":23e-6 * u.meter**3 / u.mole,
                          "DefaultSRInvariant": 1.0e-15 / u.second,
                          "WaterFugacityExponent": 1.2,
                          "GrainSizeExponent": 0.0,
                          "WaterFugacity": 1000,
                          "GrainSize": 10000 * u.micrometer
                          }

SRT_DryQtz_Koch1983    = {"n": 3.0, 
                          "A": 5e-25 * u.pascal**(-3.0) / u.second,
                          "Q": 149 * u.kilojoule / u.mole,
                          "Va": 0.0 * u.meter**3 / u.mole,
                          "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_WetQtz_Kronenberg1984  = {"n": 2.4, 
                              "A": 1.3e-20 * u.pascal**(-2.4) / u.second,
                              "Q": 134 * u.kilojoule / u.mole,
                              "Va": 0.0 * u.meter**3 / u.mole,
                              "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_DryDiabase_Mackwell1998 = {"n": 4.7, 
                               "A": 8.0 * u.megapascal**(-4.7) / u.second,
                               "Q": 485 * u.kilojoule / u.mole,
                               "Va": 0.0 * u.meter**3 / u.mole,
                               "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_WetAnorthite_Ribacki2000 = {"n": 3.0, 
                                "A": 3.9810717e-16 * u.pascal**(-3.0)/u.second,
                                "Q": 356 * u.kilojoule/u.mole,
                                "Va": 0.0 * u.meter**3/u.mole,
                                "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_WetDunite_Brace1980 = {"n": 3.0, 
                           "A": 7.0e4*u.megapascal**(-3.0) / u.second,
                           "Q": 520*u.kilojoule/u.mole,
                           "Va": 0.0*u.meter**3/u.mole,
                           "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_DryOlivine_Korenaga2008 = {"n": 4.94, 
                               "A": 10e6 * u.megapascal**(-4.94) / u.second,
                               "Q": 610 * u.kilojoule / u.mole,
                               "Va": 0.0 * u.meter**3 / u.mole,
                               "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_Olivine_Kirby1987 = {"n": 3.0, 
                         "A": 1e-15 * u.pascal**(-3.0) / u.second,
                         "Q": 500 * u.kilojoule / u.mole,
                         "Va": 0.0 * u.meter**3 / u.mole,
                         "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_WetQ_GleasonTullis1995 = {"n": 4.0, 
                              "A": 1.10e-28 * u.pascal**(-4.0) / u.second,
                              "Q": 223 * u.kilojoule / u.mole,
                              "Va": 0.0 * u.meter**3 / u.mole,
                              "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_DryOl_KaratoWu1990 = {"n": 3.5, 
                          "A": 2.4168e-15 * u.pascal**(-3.5) / u.second,
                          "Q": 540 * u.kilojoule / u.mole,
                          "Va": 25.0e-6 * u.meter**3 / u.mole,
                          "DefaultSRInvariant": 1.0e-15 / u.second}

SRT_WetQ_Tullis2002 = {"n": 4.0, 
                          "A": 1.10e-28 * u.pascal**(-4.0) / u.second,
                          "Q": 223 * u.kilojoule / u.mole,
                          "Va": 0.0 * u.meter**3 / u.mole,
                          "DefaultSRInvariant": 1.0e-15 / u.second}

_SRT = "Strain Rate and Temperature Dependent Rheology"
_WQ = "Wet Quartz"
_WOl = "Wet Olivine"
_DOl = "Dry Olivine"
_DQ = "Dry Quartz"
_DD = "Dry Diabase"
_WA = "Wet Anorthite"
_WD = "Wet Dunite"

Functions = {_SRT: powerLaw}

ViscousLaws = {
"Patterson et al., 1990"   : {"Type" : _SRT, "Mineral" : _WQ,  "coefficients"       : SRT_WetQ_Patterson1990},
"Goetze et al., 1978"      : {"Type" : _SRT, "Mineral" : _WQ,  "coefficients"       : SRT_WetQ_Goetze1978},
"Wang et al., 2012"        : {"Type" : _SRT, "Mineral" : _WQ,  "coefficients"       : SRT_DryMG_Wang2012},
"Hirth et al., 2003"       : {"Type" : _SRT, "Mineral" : _WOl,  "coefficients"      : SRT_WOl_Hirth2003},
"Koch et al., 1983"        : {"Type" : _SRT, "Mineral" : _DQ,  "coefficients"       : SRT_DryQtz_Koch1983},
"Kronenberg et al., 1984"  : {"Type" : _SRT, "Mineral" : _WQ,  "coefficients"       : SRT_WetQtz_Kronenberg1984},
"Mackwell et al., 1998"    : {"Type" : _SRT, "Mineral" : _DD,  "coefficients"       : SRT_DryDiabase_Mackwell1998},
"Ribacki et al., 2000"     : {"Type" : _SRT, "Mineral" : _WA,  "coefficients"       : SRT_WetAnorthite_Ribacki2000},
"Brace et al., 1980"       : {"Type" : _SRT, "Mineral" : _WD,  "coefficients"       : SRT_WetDunite_Brace1980},
"Korenaga et al., 2008"    : {"Type" : _SRT, "Mineral" : _DOl,  "coefficients"      : SRT_DryOlivine_Korenaga2008},
"Kirby et al., 1987"       : {"Type" : _SRT, "Mineral" : "Olivine",  "coefficients" : SRT_Olivine_Kirby1987},
"Gleason and Tullis, 1995" : {"Type" : _SRT, "Mineral" : _WQ,  "coefficients"       : SRT_WetQ_GleasonTullis1995},
"Karato and Wu, 1990"      : {"Type" : _SRT, "Mineral" : _DOl,  "coefficients"      : SRT_DryOl_KaratoWu1990},
"Tullis et al, 2002"       : {"Type" : _SRT, "Mineral" : _WQ,  "coefficients"       : SRT_WetQ_Tullis2002},
}
