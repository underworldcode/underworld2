Notes on new gLucifer data export features:
===========================================

As well as exporting image and video files, gLucifer can now write a database file containing the 3d model objects created during visualisation. This is particularly useful for 3D model runs when a 2D image of the output is too limiting .

To create this output file, an object of the lucDatabase type is needed:

	<struct name="db">
		<param name="Type">lucDatabase</param>
	</struct>

Which is provided to any window object(s) whose contents will be written to the database in the "database" parameter:

	<struct name="window">
   ...
		<param name="database">db</param>
   ...

Models that include the default "window.xml" file will have this feature turned on by default.
To turn off the database, set the parameter "nodb" on the window to "true". eg:

	<struct name="window">
   ...
		<param name="nodb">True</param>
   ...

To turn off image composition and output in gLucifer, a "noimage" parameter has also been added. This is to allow creation of database output only without rendering any images or movies:

	<struct name="window">
   ...
		<param name="noimage">True</param>
   ...

There are a few other optional parameters that can be set on the database object:

	<struct name="db">
		<param name="Type">lucDatabase</param>
      <param name="writeEvery">1</param>
      <param name="filename">gLucifer</param>
      <param name="singleFile">True</param>
	</struct>

The default database output filename is "gLucifer.gldb" in your output directory, set the "filename" parameter to change this. "writeEvery" refers to the frequency of database output, the default is to write data every timestep (1), increase it to N to reduce the output frequency to every Nth timestep. By default a single database file is created, to create a separate file for each timestep output, set the "singleFile" parameter tp False.

The gLucifer Viewer:
===============================
Once you have created database files from a simulation run, you can view them using the gLucifer Viewer, which is built with Underworld by default if the required visualisation dependencies are available.
From your underworld install directory it is found at: build/bin/gLucifer

To use the viewer, pass the path to a database file, eg:

build/bin/gLucifer output/gLucifer.gldb

If everything is ok with your database file and the viewer is able to load you will be presented with a window showing the first window that was output at the first timestep.

The mouse can and keyboard can be used to interact with the model, currently there is no proper graphical user interface to set view parameters and provide online help and hints but if you press the "F1" key a list of all the available interaction commands and what they do is printed to the terminal, as follows:
(Note: on Mac OS X, holding the Apple key with the left mouse button can be used if your mouse has no right button)

--- gLucifer Viewer version 0.2 ---
Hold the Left mouse button and drag to Rotate about the X & Y axes
Hold the Right mouse button and drag to Pan (left/right/up/down)
Hold the Middle mouse button and drag to Rotate about the Z axis
Hold [shift] and use the scroll wheel to move the clip plane in and out.

[F1]         Print help
[Up]         Load previous model in list at current time-step if data available
[Down]       Load next model in list at current time-step if data available
[Left]       Previous time-step
[Right]      Next time-step
[Page Up]    Select the previous viewport if available
[Page Down]  Select the next viewport if available
[Home]       View All mode ON/OFF, shows all objects in a single viewport
[End]        ViewPort mode ON/OFF, shows all viewports in window together

[a]          Hide/show axis
[b]          Background colour switch WHITE/BLACK
[B]          Background colour grey
[c]          Camera info: XML output of current camera parameters
[d]          Draw quad surfaces as triangle strips ON/OFF
[f]          Frame box mode ON/FILLED/OFF
[g]          Colour map log scales override DEFAULT/ON/OFF
[j]          Experimental: localise colour scales, minimum and maximum calibrated to each object drawn
[J]          Restore original colour scale min & max
[k]          Lock colour scale calibrations to current values ON/OFF
[l]          Lighting ON/OFF
[m]          Model bounding box update - resizes based on min & max vertex coords read
[n]          Recalculate surface normals
[o]          Print list of object names with id numbers.
[r]          Reset camera viewpoint
[s]          Take screen-shot and save as png/ppm image file
[q] or [ESC] Quit program
[u]          Backface Culling ON/OFF
[w]          Wireframe ON/OFF
[`]          Full screen ON/OFF
[*]          Auto zoom to fit ON/OFF
[/]          Stereo ON/OFF
[\]          Switch coordinate system Right-handed/Left-handed
[|]          Switch rulers ON/OFF
[@]          Zero camera - set to coord (0,0,0)
[;]          Flat tracer rendering ON/OFF
[:]          Tracer scaling by time-step ON/OFF
[,]          Switch to next particle rendering texture
[+]          More particles (reduce sub-sampling)
[=]          Less particles (increase sub-sampling)

[v]          Increase vector size scaling
[V]          Reduce vector size scaling
[t]          Increase tracer size scaling
[T]          Reduce tracer size scaling
[p]          Increase particle size scaling
[P]          Reduce particle size scaling
[h]          Increase shape size scaling
[H]          Reduce shape size scaling

[Alt] + [p]  hide/show all particle swarms
[Alt] + [v]  hide/show all vector arrow fields
[Alt] + [t]  hide/show all tracer trajectories
[Alt] + [s]  hide/show all quad surfaces (scalar fields, cross sections etc.)
[Alt] + [u]  hide/show all triangle surfaces (isosurfaces)
[Alt] + [h]  hide/show all shapes
[Alt] + [i]  hide/show all lines

Type a number and then press:
[Enter]      skip to time-step entered
[Page Up]    skip back entered timesteps relative to current timestep
[Page Down]  skip forward entered timesteps relative to current timestep
[o]          hide/show objects by global id number.
[p]          hide/show particle swarms by id number.
[v]          hide/show vector arrow fields by id
[t]          hide/show tracer trajectories by id
[s]          hide/show quad surfaces by id (scalar fields etc.)
[u]          hide/show triangle surfaces by id (isosurfaces)
[h]          hide/show shapes by id
[i]          hide/show lines by id
[e]          override tracer trajectory steps with entered number
[E]          clears the tracer step override.
[r]          filter by processor rank, e.g.: 0r will show only geometry rendered on first processor.
[R]          clears rank filter and show all processors

