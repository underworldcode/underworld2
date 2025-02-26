#!/bin/sh
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
##
## Copyright (C), 2003-2006, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street,
##	Melbourne, 3053, Australia.
##
## Primary Contributing Organisations:
##	Victorian Partnership for Advanced Computing Ltd, Computational Software Development - http://csd.vpac.org
##	Australian Computational Earth Systems Simulator - http://www.access.edu.au
##	Monash Cluster Computing - http://www.mcc.monash.edu.au
##	Computational Infrastructure for Geodynamics - http://www.geodynamics.org
##
## Contributors:
##	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
##	Robert Turnbull, Research Assistant, Monash University. (robert.turnbull@sci.monash.edu.au)
##	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
##	David May, PhD Student, Monash University (david.may@sci.monash.edu.au)
##	Louis Moresi, Associate Professor, Monash University. (louis.moresi@sci.monash.edu.au)
##	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
##	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
##	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
##	Julian Giordani, Research Assistant, Monash University. (julian.giordani@sci.monash.edu.au)
##	Vincent Lemiale, Postdoctoral Fellow, Monash University. (vincent.lemiale@sci.monash.edu.au)
##
##  This library is free software; you can redistribute it and/or
##  modify it under the terms of the GNU Lesser General Public
##  License as published by the Free Software Foundation; either
##  version 2.1 of the License, or (at your option) any later version.
##
##  This library is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##  Lesser General Public License for more details.
##
##  You should have received a copy of the GNU Lesser General Public
##  License along with this library; if not, write to the Free Software
##  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
##
## $Id: testLibStgFEM.0of1.sh 656 2006-10-18 06:45:50Z SteveQuenette $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TEST_SCRIPT=./VMake/executableTester.sh
until test -r ${TEST_SCRIPT} ; do
        TEST_SCRIPT=../${TEST_SCRIPT}
done
. ${TEST_SCRIPT}

runAndHandleSystemTestStdLocations "testLibStgFEM " "$0" "$@"
