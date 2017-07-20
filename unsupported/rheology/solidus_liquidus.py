from unsupported.scaling import *

# Solidus

Solidus_Peridodite_Hirschmann = {"A1": 1120.66061 * u.delta_degC,
                                 "A2": 132.899012 * u.delta_degC / u.gigapascal,
                                 "A3": -5.1404654 * u.delta_degC / u.gigapascal**2
                                 }

Solidus_Crustal_Rey           = {"A1": 993.0 * u.degK,
                                 "A2": -1.2e-7 * u.delta_degC / u.pascal,
                                 "A3": 1.2e-16 * u.delta_degC / u.pascal**2
                                 }

Solidus_Mantle_Rey            = {"A1": 1393.661 * u.degK,
                                 "A2": 1.32899e-7 * u.delta_degC / u.pascal,
                                 "A3": -5.104e-18 * u.delta_degC / u.pascal**2
                                 }

# Liquidus

Liquidus_Crustal_Rey           = {"A1": 1493.0 * u.degK,
                                 "A2": -1.2e-7 * u.delta_degC / u.pascal,
                                 "A3": 1.6e-16 * u.delta_degC / u.pascal**2
                                 }

Liquidus_Mantle_Rey            = {"A1": 2013. * u.degK,
                                 "A2": 6.15e-8 * u.delta_degC / u.pascal,
                                 "A3": -3.12e-18 * u.delta_degC / u.pascal**2
                                 }
