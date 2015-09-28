//WebGL particle/surface viewer, Owen Kaluza (c) Monash University 2012-14
//TODO: 
//Replace all control creation, update and set with automated routines from list of defined property controls
// License: LGPLv3 for now
var vis = {};
var viewer;
var params, messages, properties, objectlist;
var server = false;
var types = {'triangles' : "triangle", 'points' : "particle", 'lines' : "line", "border" : "line"};
var debug_on = false;

function initPage(src, fn) {
  var urlq = decodeURI(window.location.href);
  var query;
  if (urlq.indexOf("?") > 0) {
    var parts = urlq.split("?"); //whole querystring before and after ?
    query = parts[1]; 

    //Print debugging output?
    if (query.indexOf("debug") > 0) debug_on = true;

    if (!src && query.indexOf(".json") > 0) {
      //Passed a json(p) file on URL
      if (query.indexOf(".jsonp") > 0) {
        //Load jsonp file as a script, useful when opening page as file://
        //only way to get around security issues in chrome, 
        //script calls the loadData function providing the content
        var script = document.createElement('script');
        script.id = 'script';
        script.style.display = 'none';
        script.src = query;
        document.body.appendChild(script);
      } else {
        $S('fileupload').display = "none";
        progress("Downloading model data from server...");
        ajaxReadFile(query, initPage, false, updateProgress);
      }
      return;
    }
  }

  progress();

  var canvas = $('canvas');
  viewer =  new Viewer(canvas);

  if (query && query.indexOf("server") >= 0) {
    //Switch to image frame
    setAll('', 'server');
    setAll('none', 'client');

    server = true;

    //if (!viewer.gl) {
      //img = $('frame');
      //Image canvas event handling
      //img.mouse = new Mouse(img, new MouseEventHandler(serverMouseClick, serverMouseWheel, serverMouseMove, serverMouseDown));
    //}

    //Initiate the server update
    requestData('/connect', parseRequest);
    //requestData('/objects', parseObjects);

    //Enable to forward key presses to server directly
    //document.onkeypress = keyPress;
    window.onbeforeunload = function() {if (client_id >= 0) requestData("/disconnect=" + client_id, null, true); client_id = -1;};

    //Set viewer window to match ours?
    //resizeToWindow();

  } else {
    setAll('none', 'server');
    setAll('', 'client');
    $('frame').style.display = 'none';
  }

  //Canvas event handling
  canvas.mouse = new Mouse(canvas, new MouseEventHandler(canvasMouseClick, canvasMouseWheel, canvasMouseMove, canvasMouseDown, null, null, canvasMousePinch));
  //Following two settings should probably be defaults?
  canvas.mouse.moveUpdate = true; //Continual update of deltaX/Y
  //canvas.mouse.setDefault();

  canvas.mouse.wheelTimer = true; //Accumulate wheel scroll (prevents too many events backing up)
  defaultMouse = document.mouse = canvas.mouse;

  //Create tool windows
  params =     new Toolbox("params", 20, 20);
  objectlist = new Toolbox("objectlist", 370, 20);
  messages =   new Toolbox("messages", 400, 300);
  properties = new Toolbox("properties", 720, 20);

  params.show();
  objectlist.show();

  if (src) {
    viewer.loadFile(src);
  } else {
    var source = getSourceFromElement('source');
    if (source) {
      //Preloaded data
      $S('fileupload').display = "none";
      viewer.loadFile(source);
    } else {
      //Demo objects
      //demoData();
      viewer.draw();
    }
  }
}

function loadData(data) {
  initPage(data);
}

function progress(text) {
  var el = $('progress');
  if (el.style.display == 'block' || text == undefined)
    //el.style.display = 'none';
    setTimeout("$('progress').style.display = 'none';", 150);
  else {
    $('progressmessage').innerHTML = text;
    $('progressstatus').innerHTML = "";
    $S('progressbar').width = 0;
    el.style.display = 'block';
  }
}

var rztimeout = null;
function autoResize() {
  if (rztimeout) clearTimeout(rztimeout);
  //if (server)
  //  rztimeout = setTimeout('resizeToWindow();', 500);
  //else
    rztimeout = setTimeout('viewer.draw();', 150);
}

function canvasMouseClick(event, mouse) {
  if (server) {
    if (viewer.rotating)
      sendCommand('' + viewer.getRotationString());
    else
      sendCommand('' + viewer.getTranslationString());
  }

  //if (server) serverMouseClick(event, mouse); //Pass to server handler
  if (viewer.rotating) {
    viewer.rotating = false;
    //viewer.reload = true;
    sortTimer();
  }
  if ($("immsort").checked == true) {
    //No timers
    viewer.draw();
    viewer.rotated = true; 
  }

  viewer.draw();
  return false;
}

function sortTimer(ifexists) {
  if ($("immsort").checked == true) {
    //No timers
    return;
  }
  //Set a timer to apply the sort function in 2 seconds
  if (viewer.timer) {
    clearTimeout(viewer.timer);
  } else if (ifexists) {
    //No existing timer? Don't start a new one
    return;
  }
  var element = $("sort");
  element.style.display = 'block';
  viewer.timer = setTimeout(function() {viewer.rotated = true; element.onclick.apply(element);}, 2000);
}

function canvasMouseDown(event, mouse) {
  //if (server) serverMouseDown(event, mouse); //Pass to server handler
  return false;
}

function canvasMouseMove(event, mouse) {
  //if (server) serverMouseMove(event, mouse); //Pass to server handler
  if (!mouse.isdown || !viewer) return true;
  viewer.rotating = false;

  //Switch buttons for translate/rotate
  var button = mouse.button;
  if ($('tmode').checked)
    button = Math.abs(button-2);

  //console.log(mouse.deltaX + "," + mouse.deltaY);
  switch (button)
  {
    case 0:
      viewer.rotateY(mouse.deltaX/5);
      viewer.rotateX(mouse.deltaY/5);
      viewer.rotating = true;
      sortTimer(true);  //Delay sort if queued
      break;
    case 1:
      viewer.rotateZ(Math.sqrt(mouse.deltaX*mouse.deltaX + mouse.deltaY*mouse.deltaY)/5);
      viewer.rotating = true;
      sortTimer(true);  //Delay sort if queued
      break;
    case 2:
      var adjust = viewer.modelsize / 1000;   //1/1000th of size
      viewer.translate[0] += mouse.deltaX * adjust;
      viewer.translate[1] -= mouse.deltaY * adjust;
      break;
  }

  //Always draw border while interacting in server mode?
  if (server)
    viewer.draw(true);
  //Draw border while interacting (automatically on for models > 500K vertices)
  //Hold shift to switch from default behaviour
  else if ($("interactive").checked == true)
    viewer.draw();
  else
    viewer.draw(!event.shiftKey);
  return false;
}

function canvasMouseWheel(event, mouse) {
  //if (server) serverMouseWheel(event, mouse); //Pass to server handler
  if (event.shiftKey) {
    var factor = event.spin * 0.01;
    viewer.zoomClip(factor);
  } else {
    var factor = event.spin * 0.05;
    viewer.zoom(factor);
  }
  return false; //Prevent default
}

function canvasMousePinch(event, mouse) {
  if (event.distance != 0) {
    var factor = event.distance * 0.0001;
    viewer.zoom(factor);
  }
  return false; //Prevent default
}

//File upload handling
var saved_files;
function fileSelected(files, callback) {
  saved_files = files;

  // Check for the various File API support.
  if (window.File && window.FileReader) { // && window.FileList) {
    //All required File APIs are supported.
    for (var i = 0; i < files.length; i++) {
      var file = files[i];
      //User html5 fileReader api (works offline)
      var reader = new FileReader();

      // Closure to capture the file information.
      reader.onload = (function(file) {
        return function(e) {
          //alert(e.target.result);
          viewer.loadFile(e.target.result);
        };
      })(file);

      // Read in the file (AsText/AsDataURL/AsArrayBuffer/AsBinaryString)
      reader.readAsText(file);
    }
  } else {
    alert('The File APIs are not fully supported in this browser.');
  }
}

function getImageDataURL(img) {
  var canvas = document.createElement("canvas");
  canvas.width = img.width;
  canvas.height = img.height;
  var ctx = canvas.getContext("2d");
  ctx.drawImage(img, 0, 0);
  var dataURL = canvas.toDataURL("image/png");
  return dataURL;
}

function loadColourMaps() {
  //Load colourmaps
  if (!vis.colourmaps) return;
  var list = $('colourmap-presets');
  var sel = list.value;
  var canvas = $('palette');
  list.options.length = 1; //Remove all except "None"
  for (var i=0; i<vis.colourmaps.length; i++) {
    var palette = new Palette("Background=rgba(255,255,255,0)\n");
    vis.colourmaps[i].palette = palette;

    //palette.colours = vis.colourmaps[i].colours;
    for (var j=0; j<vis.colourmaps[i].colours.length; j++)
      //palette.colours.push(new ColourPos(parseInt(vis.colourmaps[i].colours[j].colour), vis.colourmaps[i].colours[j].position));
      palette.colours.push(new ColourPos(vis.colourmaps[i].colours[j].colour, vis.colourmaps[i].colours[j].position));

    var option = new Option(vis.colourmaps[i].name || ("ColourMap " + i), i);
    list.options[list.options.length] = option;

     //Draw without UI elements
     palette.draw(canvas, false);

    //Get colour obj and store in array
    var context = canvas.getContext('2d');  
    if (!context) alert("getContext failed");
    var pixels = context.getImageData(0, 0, 512, 1).data;
    palette.cache = [];
    for (var c=0; c<512; c++) {
      var cstr = "rgba(" + pixels[c*4] + "," + pixels[c*4+1] + "," + pixels[c*4+2] + "," + pixels[c*4+3] + ")";
      OK.debug(c + " == " + cstr);
      //var colour = new Colour(cstr);
      palette.cache[c] = pixels[c*4] + (pixels[c*4+1] << 8) + (pixels[c*4+2] << 16) + (pixels[c*4+3] << 24);
    }

    //Redraw UI
    palette.draw(canvas, true);
  }
  //Restore selection
  list.value = sel;
  viewer.setColourMap(sel);
}

/*function checkPointMinMax(x, y, z) {
  if (x < vis.options.min[0]) vis.options.min[0] = x;
  if (y < vis.options.min[1]) vis.options.min[1] = y;
  if (z < vis.options.min[2]) vis.options.min[2] = z;
  if (x > vis.options.max[0]) vis.options.max[0] = x;
  if (y > vis.options.max[1]) vis.options.max[1] = y;
  if (z > vis.options.max[2]) vis.options.max[2] = z;
  //alert(min[0] + "," + min[1] + "," + min[2] + " -- " + max[0] + "," + max[1] + "," + max[2]);
  //alert(offset + " : " + x + "," + y + "," + z);
}*/

function objVertexColour(obj, values, idx) {
  return vertexColour(obj.colour, obj.opacity, obj.colourmap >= 0 ? vis.colourmaps[obj.colourmap] : null, values, idx);
}

function vertexColour(colour, opacity, colourmap, values, idx) {
  //Default to object colour property
  if (values) {
    if (colourmap) {
      //Use a colourmap
      var min = parseFloat(colourmap.min);
      var max = parseFloat(colourmap.max);
      if (values.minimum != undefined) min = values.minimum;
      if (values.maximum != undefined) max = values.maximum;
      if (min == undefined) min = 0;
      if (max == undefined) min = 1;
      //Get nearest pixel on the canvas
      var pos = 256;  //If rubbish data, return centre
      //Allows single value for entire object
      if (idx >= values.data.length) idx = values.data.length-1;
      var val = values.data[idx];
      if (val < min)
        pos = 0;
      else if (val > max)
        pos = 511;
      else if (max > min) {
        var scaled;
        if (colourmap.log) {
          val = Math.log10(val);
          min = Math.log10(min);
          max = Math.log10(max);
        }
        //Scale to range [0,1]
        scaled = (val - min) / (max - min);
        //Get colour pos [0-512)
        pos =  Math.round(511 * scaled);
      }
      colour = colourmap.palette.cache[pos];
      //if (idx % 100 == 0) console.log(" : " + val + " min " + min + " max " + max + " pos = " + pos + " colour: " + colour);
    } else if (values.type == 'integer') {
      //Integer data values, treat as colours
      colour = values.data[idx];
    }
  }

  //Apply opacity per object setting
  if (opacity) {
    var C = new Colour(colour);
    C.alpha = C.alpha * opacity;
    colour = C.toInt();
  }

  //Return final integer value
  return colour;
}

function demoData(num)
{
  var start = new Date();
  if (!num) num = 1000000;
  var min = [-1.0, -1.0, -1.0];
  var max = [1.0, 1.0, 1.0];
  var dims = [max[0] - min[0], max[1] - min[1], max[2] - min[2]];
  var modelsize = Math.sqrt(dims[0]*dims[0] + dims[1]*dims[1] + dims[2]*dims[2]);
  OK.debug("Generating demo particles...");
  var data = 
    {
      "options" : {"pointScale" : 1, "rotate" : [0,0,0], "min" : min, "max" : max},
      "colourmaps" : 
      [
        {
          "minimum" : 0,
          "maximum" : modelsize/2,
          "log" : 0,
          "colours" : 
          [
            {"position" : 0, "colour" : -16776961},
            {"position" : 0.2, "colour" : -16711681},
            {"position" : 0.4, "colour" : -16711936},
            {"position" : 0.6, "colour" : -1648467},
            {"position" : 0.8, "colour" : -65536},
            {"position" : 1, "colour" : -1040224}
          ]
        }
      ],
      "objects" : 
      [
        {
          "name" : "particles",
          "points" :
          {
            "colour" : null,
            "colourmap" : 0,
            "vertices" : 
            {
              "size" : 3,
              "data" : []
            },
            "values" : 
            {
              "size" : 1,
              "minimum" : null, 
              "maximum" : null,
              "colourmap" : 0,
              "data" : []
            }
          }
        },
        {
          "name" : "surface",
          "triangles" :
          {
            "colour" : null,
            "colourmap" : 0,
            "indices" : 
            {
              "size" : 1,
              "data" : []
            },
            "vertices" : 
            {
              "size" : 3,
              "data" : []
            },
            "normals" : 
            {
              "size" : 3,
              "data" : []
            },
            "values" : 
            {
              "size" : 1,
              "minimum" : null, 
              "maximum" : null,
              "colourmap" : 0,
              "data" : []
            }
          }
        }
      ]
    };

  var verts = data.objects[0].points.vertices.data;
  var vals = data.objects[0].points.values.data;
  for(var i=0; i < num; i++) 
  {
     var x = min[0] + (dims[0] * Math.random());
     var y = min[1] + (dims[1] * Math.random());
     var z = min[2] + (dims[2] * Math.random());
     verts.push(x);
     verts.push(y);
     verts.push(z);
     vals.push(Math.sqrt(x*x + y*y + z*z));
     //verts.push(1.0);
  }

/*
  OK.debug("Generating demo triangles...");
  verts = data.objects[1].triangles.vertices.data;
  norms = data.objects[1].triangles.normals.data;
  vals = data.objects[1].triangles.values.data;
  var heightdata = generateHeight(101, 101);
  var calcVert = function(i, j) {
    return [min[0] + (dims[0] * i/100.0), min[1] + (dims[1] * j/100.0), heightdata[j*101+i]/200.0];
  }
  for(var i=0; i < 100; i++) 
  {
    for(var j=0; j < 100; j++) 
    {
      //var z = min[2] + (0.1 * dims[2] * Math.random());
      var v1 = calcVert(i, j);
      var v2 = calcVert(i, j+1);
      var v3 = calcVert(i+1, j);
      var v4 = calcVert(i+1, j+1);

      //Vertices, normals & values
      verts.push(v1[0]); verts.push(v1[1]); verts.push(v1[2]);
      verts.push(v2[0]); verts.push(v2[1]); verts.push(v2[2]);
      verts.push(v3[0]); verts.push(v3[1]); verts.push(v3[2]);
      verts.push(v4[0]); verts.push(v4[1]); verts.push(v4[2]);
      var normal = trinormal(v1, v3, v2);
      norms.push(normal[0]); norms.push(normal[1]); norms.push(normal[2]);
      norms.push(normal[0]); norms.push(normal[1]); norms.push(normal[2]);
      norms.push(normal[0]); norms.push(normal[1]); norms.push(normal[2]);
      norms.push(normal[0]); norms.push(normal[1]); norms.push(normal[2]);
      vals.push(Math.sqrt(v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2]));
      vals.push(Math.sqrt(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]));
      vals.push(Math.sqrt(v3[0]*v3[0] + v3[1]*v3[1] + v3[2]*v3[2]));
      vals.push(Math.sqrt(v4[0]*v4[0] + v4[1]*v4[1] + v4[2]*v4[2]));

      //Triangle 1
      indices.push(
      var normal = trinormal(v1, v3, v2);
      verts.push(v1[0]); verts.push(v1[1]); verts.push(v1[2]);
      norms.push(normal[0]); norms.push(normal[1]); norms.push(normal[2]);
      vals.push(Math.sqrt(v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2]));
      //
      verts.push(v2[0]); verts.push(v2[1]); verts.push(v2[2]);
      norms.push(normal[0]); norms.push(normal[1]); norms.push(normal[2]);
      vals.push(Math.sqrt(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]));
      //
      verts.push(v3[0]); verts.push(v3[1]); verts.push(v3[2]);
      norms.push(normal[0]); norms.push(normal[1]); norms.push(normal[2]);
      vals.push(Math.sqrt(v3[0]*v3[0] + v3[1]*v3[1] + v3[2]*v3[2]));
      //Triangle 2
      var normal = trinormal(v4, v2, v3);
      verts.push(v4[0]); verts.push(v4[1]); verts.push(v4[2]);
      norms.push(normal[0]); norms.push(normal[1]); norms.push(normal[2]);
      vals.push(Math.sqrt(v4[0]*v4[0] + v4[1]*v4[1] + v4[2]*v4[2]));
      //
      verts.push(v3[0]); verts.push(v3[1]); verts.push(v3[2]);
      norms.push(normal[0]); norms.push(normal[1]); norms.push(normal[2]);
      vals.push(Math.sqrt(v3[0]*v3[0] + v3[1]*v3[1] + v3[2]*v3[2]));
      //
      verts.push(v2[0]); verts.push(v2[1]); verts.push(v2[2]);
      norms.push(normal[0]); norms.push(normal[1]); norms.push(normal[2]);
      vals.push(Math.sqrt(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]));
    }
  }
*/

  var time = (new Date() - start) / 1000.0;
  OK.debug(time + " seconds to generate random data");
  viewer.loadFile(data);
}

function str2ab(str) {
  var buf = new ArrayBuffer(str.length);
  var bufView = new Uint8Array(buf);
  for (var i=0, strLen=str.length; i<strLen; i++) {
    bufView[i] = str.charCodeAt(i);
  }
  return buf;
}

function generateHeight( width, height ) {
  var size = width * height, data = new Float32Array(size),
      perlin = new ImprovedNoise(), quality = 1, z = Math.random() * 100;

  for ( var i = 0; i < size; i ++ ) {
    data[ i ] = 0
  }

  for ( var j = 0; j < 4; j ++ ) {
    for ( var i = 0; i < size; i ++ ) {
      var x = i % width, y = ~~ ( i / width );
      data[ i ] += Math.abs( perlin.noise( x / quality, y / quality, z ) * quality * 1.75 );
    }
    quality *= 5;
  }

  return data;
}

function crossProduct(a, b) {
  // Check lengths
  if (a.length != 3 || b.length != 3) {
     return;
  }
  return [a[1]*b[2] - a[2]*b[1],
          a[2]*b[0] - a[0]*b[2],
          a[0]*b[1] - a[1]*b[0]];
} 

function normalise(v) {
  var len = 0;
  for (var i in v)
    len += v[i]*v[i];
  len = Math.sqrt(len);
  for (var i in v)
    v[i] /= len;
  return v;
} 

function trinormal(a, b, c) {
  var ab = [b[0] - a[0], b[1] - a[1], b[2] - a[2]];
  var ac = [c[0] - a[0], c[1] - a[1], c[2] - a[2]];
  return normalise(crossProduct(ab, ac));
} 

//Get eye pos vector z by multiplying vertex by modelview matrix
function eyeDistance(M2,P) {
  return -(M2[0] * P[0] + M2[1] * P[1] + M2[2] * P[2] + M2[3]);
}

function newFilledArray(length, val) {
    var array = [];
    for (var i = 0; i < length; i++) {
        array[i] = val;
    }
    return array;
}

function radix(nbyte, source, dest, N)
//void radix(char byte, char size, long N, unsigned char *src, unsigned char *dst)
{
   // Radix counting sort of 1 byte, 8 bits = 256 bins
   var count = newFilledArray(256,0);
   var index = [];
   var i;
   //unsigned char* dst = (unsigned char*)dest;

   //Create histogram, count occurences of each possible byte value 0-255
   var mask = 0xff;// << (byte*8);
   for (i=0; i<N; i++) {
     //if (byte > 0) alert(source[i].key + " : " + (source[i].key >> (byte*8) & mask) + " " + mask); 
      count[source[i].key >> (nbyte*8) & mask]++;
   }

   //Calculate number of elements less than each value (running total through array)
   //This becomes the offset index into the sorted array
   //(eg: there are 5 smaller values so place me in 6th position = 5)
   index[0]=0;
   for (i=1; i<256; i++) index[i] = index[i-1] + count[i-1];

   //Finally, re-arrange data by index positions
   for (i=0; i<N; i++ )
   {
       var val = source[i].key >> (nbyte*8) & mask;  //Get value
       //memcpy(&dest[index[val]], &source[i], size);
       dest[index[val]] = source[i];
       //memcpy(&dst[index[val]*size], &src[i*size], size);
       index[val]++; //Increment index to push next element with same value forward one
   }
}


function radix_sort(source, swap, bytes)
{
   //assert(bytes % 2 == 0);
   //OK.debug("Radix X sort: %d items %d bytes. Byte: ", N, size);
   // Sort bytes from least to most significant 
   var N = source.length;
   for (var x = 0; x < bytes; x += 2) 
   {
      radix(x, source, swap, N);
      radix(x+1, swap, source, N);
   }
}


//http://stackoverflow.com/questions/7936923/assist-with-implementing-radix-sort-in-javascript
//arr: array to be sorted
//begin: 0
//end: length of array
//bit: maximum number of bits required to represent numbers in arr
function msb_radix_sort(arr, begin, end, bit) {
  var i, j, mask, tmp;
  i = begin;
  j = end;
  mask = 1 << bit;
  while(i < j) {
    while(i < j && !(arr[i].key & mask)) ++i;
    while(i < j && (arr[j - 1].key & mask)) --j;
    if(i < j) {
      j--;
      tmp = arr[i]; //Swap
      arr[i] = arr[j];
      arr[j] = tmp;
      i++;
    }
  }
  if(bit && i > begin)
    msb_radix_sort(arr, begin, i, bit - 1);
  if(bit && i < end)
    msb_radix_sort(arr, i, end, bit - 1);
}

/**
 * @constructor
 */
function Toolbox(id, x, y) {
  //Mouse processing:
  this.el = $(id);
  this.mouse = new Mouse(this.el, this);
  this.mouse.moveUpdate = true;
  this.el.mouse = this.mouse;
  this.style = $S(id);
  if (x && y) {
    this.style.left = x + 'px';
    this.style.top = y + 'px';
  } else {
    this.style.left = ((window.innerWidth - this.el.offsetWidth) * 0.5) + 'px';
    this.style.top = ((window.innerHeight - this.el.offsetHeight) * 0.5) + 'px';
  }
  this.drag = false;
}

Toolbox.prototype.toggle = function() {
  if (this.style.visibility == 'visible')
    this.hide();
  else
    this.show();
}

Toolbox.prototype.show = function() {
  this.style.visibility = 'visible';
}

Toolbox.prototype.hide = function() {
  this.style.visibility = 'hidden';
}

//Mouse event handling
Toolbox.prototype.click = function(e, mouse) {
  this.drag = false;
  return true;
}

Toolbox.prototype.down = function(e, mouse) {
  //Process left drag only
  this.drag = false;
  if (mouse.button == 0 && e.target.className.indexOf('scroll') < 0 && ['INPUT', 'SELECT', 'OPTION', 'RADIO'].indexOf(e.target.tagName) < 0)
    this.drag = true;
  return true;
}

Toolbox.prototype.move = function(e, mouse) {
  if (!mouse.isdown) return true;
  if (!this.drag) return true;

  //Drag position
  this.el.style.left = parseInt(this.el.style.left) + mouse.deltaX + 'px';
  this.el.style.top = parseInt(this.el.style.top) + mouse.deltaY + 'px';
  return false;
}

Toolbox.prototype.wheel = function(e, mouse) {
}

//This object encapsulates a vertex buffer and shader set
function Renderer(gl, type, colour, border) {
  this.border = border;
  this.gl = gl;
  this.type = type;
  if (colour) this.colour = new Colour(colour);

  //Only two options for now, points and triangles
  if (type == "particle") {
    //Particle renderer
    this.attributes = ["aVertexPosition", "aVertexColour", "aVertexSize", "aPointType"],
    this.uniforms = ["uPointType", "uPointScale", "uAlpha", "uColour"]
    this.attribSizes = [3 * Float32Array.BYTES_PER_ELEMENT,
                        Int32Array.BYTES_PER_ELEMENT,
                        Float32Array.BYTES_PER_ELEMENT,
                        Float32Array.BYTES_PER_ELEMENT];
  } else if (type == "triangle") {
    //Triangle renderer
    this.attributes = ["aVertexPosition", "aVertexNormal", "aVertexColour", "aVertexObjectID", "aVertexTexCoord"],
    this.uniforms = ["uColour", "uCullFace", "uAlpha"]
    this.attribSizes = [3 * Float32Array.BYTES_PER_ELEMENT,
                        3 * Float32Array.BYTES_PER_ELEMENT,
                        Int32Array.BYTES_PER_ELEMENT,
                        4 * Uint8Array.BYTES_PER_ELEMENT];
  } else if (type == "line") {
    //Line renderer
    this.attributes = ["aVertexPosition", "aVertexColour"],
    this.uniforms = ["uColour"]
    this.attribSizes = [3 * Float32Array.BYTES_PER_ELEMENT,
                        Int32Array.BYTES_PER_ELEMENT];
  }

  this.elementSize = 0;
  for (var i=0; i<this.attribSizes.length; i++)
    this.elementSize += this.attribSizes[i];
}

Renderer.prototype.init = function() {
  if (this.type == "triangle" && !viewer.hasTriangles) return false;
  if (this.type == "particle" && !viewer.hasPoints) return false;
  var fs = this.type + '-fs';
  var vs = this.type + '-vs';

  //User defined shaders if provided...
  if (vis.shaders) {
    if (vis.shaders[this.type]) {
      fs = vis.shaders[this.type].fragment || fs;
      vs = vis.shaders[this.type].vertex || vs;
    }
  }
  //Compile the shaders
  this.program = new WebGLProgram(this.gl, vs, fs);
  if (this.program.errors) OK.debug(this.program.errors);
  //Setup attribs/uniforms (flag set to skip enabling attribs)
  this.program.setup(this.attributes, this.uniforms, true);

  return true;
}

function SortIdx(idx, key) {
  this.idx = idx;
  this.key = key;
}

Renderer.prototype.loadElements = function() {
  if (this.border) return;
  OK.debug("Loading " + this.type + " elements...");
  var start = new Date();
  var distances = [];
  var indices = [];
  //Only update the positions array when sorting due to update
  if (!this.positions || !viewer.rotated || this.type == 'line') {
    this.positions = [];
    //Add visible element positions
    for (var id in vis.objects) {
      var name = vis.objects[id].name;
      var skip = !$('object_' + name).checked;
      if (this.type == "particle") {
        if (vis.objects[id].points) {
          for (var e in vis.objects[id].points) {
            var dat = vis.objects[id].points[e];
            var count = dat.vertices.data.length;
            //OK.debug(name + " " + skip + " : " + count);
            for (var i=0; i<count; i += 3)
              this.positions.push(skip ? null : [dat.vertices.data[i], dat.vertices.data[i+1], dat.vertices.data[i+2]]);
          }
        }
      } else if (this.type == "triangle") {
        if (vis.objects[id].triangles) {
          for (var e in vis.objects[id].triangles) {
            var dat =  vis.objects[id].triangles[e];
            var count = dat.indices.data.length/3;

            //console.log(name + " " + skip + " : " + count + " - " + dat.centroids.length);
            for (var i=0; i<count; i++) {
              //this.positions.push(skip ? null : dat.centroids[i]);
              if (skip)
                this.positions.push(null);
              else if (dat.centroids.length == 1)
                this.positions.push(dat.centroids[0]);
              else
                this.positions.push(dat.centroids[i]);
            }
          }
        }
      } else if (this.type == "line") {
        //Write lines directly to indices buffer, no depth sort necessary
        if (skip) continue;
        if (vis.objects[id].lines) {
          for (var e in vis.objects[id].lines) {
            var dat =  vis.objects[id].lines[e];
            var count = dat.indices.data.length;
            //OK.debug(name + " " + skip + " : " + count);
            for (var i=0; i<count; i++)
              indices.push(dat.indices.data[i]);
          }
        }
      }
    }

    var time = (new Date() - start) / 1000.0;
    OK.debug(time + " seconds to update positions ");
    start = new Date();
  }

  //Depth sorting and create index buffer for objects that require it...
  if (indices.length == 0) {
    var distance;
    //Calculate min/max distances from view plane
    var minmax = minMaxDist();
    var mindist = minmax[0];
    var maxdist = minmax[1];

    //Update eye distances, clamping int distance to integer between 0 and 65535
    var multiplier = 65534.0 / (maxdist - mindist);
    var M2 = [viewer.webgl.modelView.matrix[2],
              viewer.webgl.modelView.matrix[6],
              viewer.webgl.modelView.matrix[10],
              viewer.webgl.modelView.matrix[14]];

    //Add visible element distances to sorting array
    for (var i=0; i<this.positions.length; i++) {
      if (this.positions[i]) {
        if (this.positions[i].length == 0) {
          //No position data, draw last
          distances.push(new SortIdx(i, 65535));
        } else {
          //Distance from viewing plane is -eyeZ
          distance = multiplier * (-(M2[0] * this.positions[i][0] + M2[1] * this.positions[i][1] + M2[2] * this.positions[i][2] + M2[3]) - mindist);
          //distance = (-(M2[0] * this.positions[i][0] + M2[1] * this.positions[i][1] + M2[2] * this.positions[i][2] + M2[3]) - mindist);
          //      if (i%100==0 && this.positions[i].length == 4) console.log(distance + " - " + this.positions[i][3]);
          //if (this.positions[i].length == 4) distance -= this.positions[i][3];
          //distance *= multiplier;
          //if (distance < 0) distance = 0;
          if (distance > 65535) distance = 65535;
          distances.push(new SortIdx(i, 65535 - Math.round(distance)));
        }
      }
    }

    var time = (new Date() - start) / 1000.0;
    OK.debug(time + " seconds to update distances ");

    if (distances.length > 0) {
      //Sort
      start = new Date();
      //distances.sort(function(a,b){return a.key - b.key});
      //This is about 10 times faster than above:
      msb_radix_sort(distances, 0, distances.length, 16);
      //Pretty sure msb is still fastest...
      //if (!this.swap) this.swap = [];
      //radix_sort(distances, this.swap, 2);
      time = (new Date() - start) / 1000.0;
      OK.debug(time + " seconds to sort");

      start = new Date();
      //Reload index buffer
      if (this.type == "particle") {
        //Process points
        for (var i = 0; i < distances.length; ++i)
          indices.push(distances[i].idx);
          //if (distances[i].idx > this.elements) alert("ERROR: " + i + " - " + distances[i].idx + " > " + this.elements);
      } else if (this.type == "triangle") {
        //Process triangles
        for (var i = 0; i < distances.length; ++i) {
          var i3 = distances[i].idx*3;
          indices.push(i3);
          indices.push(i3+1);
          indices.push(i3+2);
          //if (i3+2 > this.elements) alert("ERROR: " + i + " - " + (i3+2) + " > " + this.elements);
        }
      }
      time = (new Date() - start) / 1000.0;
      OK.debug(time + " seconds to load index buffers");
    }
  }

  start = new Date();
  if (indices.length > 0) {
    this.gl.bufferData(this.gl.ELEMENT_ARRAY_BUFFER, new Uint32Array(indices), this.gl.STATIC_DRAW);
    //this.gl.bufferData(this.gl.ELEMENT_ARRAY_BUFFER, new Uint32Array(indices), this.gl.DYNAMIC_DRAW);
    //this.gl.bufferData(this.gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), this.gl.DYNAMIC_DRAW);

    time = (new Date() - start) / 1000.0;
    OK.debug(time + " seconds to update index buffer object");
  }
  //Update count to visible elements...
  this.elements = indices.length;
}

var texcoords = [[[0,0], [0,0], [0,0]], [[0,0], [0,255], [255,0]], [[255,255], [0,0], [0,255]]];

function VertexBuffer(elements, size) {
  this.size = size;
  this.vertexSizeInFloats = size / Float32Array.BYTES_PER_ELEMENT;
  this.array = new ArrayBuffer(elements * size);
  // Map this buffer to a Float32Array to access the positions/normals/sizes
  this.floats = new Float32Array(this.array);
  // Map the same buffer to an Int32Array to access the color
  this.ints = new Int32Array(this.array);
  this.bytes = new Uint8Array(this.array);
  this.offset = 0;
  OK.debug(elements + " - " + size);
  OK.debug("Created vertex buffer");
}

VertexBuffer.prototype.loadParticles = function(object) {
  for (var p in object.points) {
    var dat =  object.points[p];
    for (var i=0; i<dat.vertices.data.length/3; i++) {
      var i3 = i*3;
      var vert = [dat.vertices.data[i3], dat.vertices.data[i3+1], dat.vertices.data[i3+2]];
      this.floats[this.offset] = vert[0];
      this.floats[this.offset+1] = vert[1];
      this.floats[this.offset+2] = vert[2];
      this.ints[this.offset+3] = objVertexColour(object, dat.values, i);
      this.floats[this.offset+4] = dat.sizes ? dat.sizes.data[i] * object.pointsize : object.pointsize;
      this.floats[this.offset+5] = object.pointtype > 0 ? object.pointtype-1 : -1;
      this.offset += this.vertexSizeInFloats;
    }
  }
}

VertexBuffer.prototype.loadTriangles = function(object, id) {
  //Process triangles
  if (!this.byteOffset) this.byteOffset = 7 * Float32Array.BYTES_PER_ELEMENT;
  var T = 0;
  if (object.wireframe) T = 1;
  for (var t in object.triangles) {
    var dat =  object.triangles[t];
    var calcCentroids = false;
    if (!dat.centroids) {
      calcCentroids = true;
      dat.centroids = [];
    }
    //if (dat.values)
    //  console.log(object.name + " : " + dat.values.minimum + " -> " + dat.values.maximum);
    //if (object.colourmap >= 0)
    //  console.log(object.name + " :: " + vis.colourmaps[object.colourmap].minimum + " -> " + vis.colourmaps[object.colourmap].maximum);

    for (var i=0; i<dat.indices.data.length/3; i++) {
      //Tex-coords for wireframing
      var texc = texcoords[(i%2+1)*T];

      //Indices holds references to vertices and other data
      var i3 = i * 3;
      var ids = [dat.indices.data[i3], dat.indices.data[i3+1], dat.indices.data[i3+2]];
      var ids3 = [ids[0]*3, ids[1]*3, ids[2]*3];

      for (var j=0; j<3; j++) {
        this.floats[this.offset] = dat.vertices.data[ids3[j]];
        this.floats[this.offset+1] = dat.vertices.data[ids3[j]+1];
        this.floats[this.offset+2] = dat.vertices.data[ids3[j]+2];
        if (dat.normals.data.length == 3) {
          //Single surface normal
          this.floats[this.offset+3] = dat.normals.data[0];
          this.floats[this.offset+4] = dat.normals.data[1];
          this.floats[this.offset+5] = dat.normals.data[2];
        } else {
          this.floats[this.offset+3] = dat.normals.data[ids3[j]];
          this.floats[this.offset+4] = dat.normals.data[ids3[j]+1];
          this.floats[this.offset+5] = dat.normals.data[ids3[j]+2];
        }
        this.ints[this.offset+6] = objVertexColour(object, dat.values, ids[j]);
        this.bytes[this.byteOffset] = id;
        this.bytes[this.byteOffset+1] = texc[j][0];
        this.bytes[this.byteOffset+2] = texc[j][1];
        this.offset += this.vertexSizeInFloats;
        this.byteOffset += this.size;
      }

      //Calc centroids (only required if vertices changed)
      if (calcCentroids) {
        if (dat.width) //indices.data.length == 6)
          //Cross-sections, null centroid - always drawn last
          dat.centroids.push([]);
        else {
          //(x1+x2+x3, y1+y2+y3, z1+z2+z3)
          var verts = dat.vertices.data;
          /*/Side lengths: A-B, A-C, B-C
          var AB = vec3.createFrom(verts[ids3[0]] - verts[ids3[1]], verts[ids3[0] + 1] - verts[ids3[1] + 1], verts[ids3[0] + 2] - verts[ids3[1] + 2]);
          var AC = vec3.createFrom(verts[ids3[0]] - verts[ids3[2]], verts[ids3[0] + 1] - verts[ids3[2] + 1], verts[ids3[0] + 2] - verts[ids3[2] + 2]);
          var BC = vec3.createFrom(verts[ids3[1]] - verts[ids3[2]], verts[ids3[1] + 1] - verts[ids3[2] + 1], verts[ids3[1] + 2] - verts[ids3[2] + 2]);
          var lengths = [vec3.length(AB), vec3.length(AC), vec3.length(BC)];
                //Size weighting shift
                var adj = (lengths[0] + lengths[1] + lengths[2]) / 9.0;
                //if (i%100==0) console.log(verts[ids3[0]] + "," + verts[ids3[0] + 1] + "," + verts[ids3[0] + 2] + " " + adj);*/
          dat.centroids.push([(verts[ids3[0]]     + verts[ids3[1]]     + verts[ids3[2]])     / 3,
                              (verts[ids3[0] + 1] + verts[ids3[1] + 1] + verts[ids3[2] + 1]) / 3,
                              (verts[ids3[0] + 2] + verts[ids3[1] + 2] + verts[ids3[2] + 2]) / 3]);
        }
      }
    }
  }
}

VertexBuffer.prototype.loadLines = function(object) {
  for (var l in object.lines) {
    var dat =  object.lines[l];
    for (var i=0; i<dat.vertices.data.length/3; i++) {
      var i3 = i*3;
      var vert = [dat.vertices.data[i3], dat.vertices.data[i3+1], dat.vertices.data[i3+2]];
      this.floats[this.offset] = vert[0];
      this.floats[this.offset+1] = vert[1];
      this.floats[this.offset+2] = vert[2];
      this.ints[this.offset+3] = objVertexColour(object, dat.values, i);
      this.offset += this.vertexSizeInFloats;
    }
  }
}

VertexBuffer.prototype.update = function(gl) {
  start = new Date();
  gl.bufferData(gl.ARRAY_BUFFER, this.array, gl.STATIC_DRAW);
  //gl.bufferData(gl.ARRAY_BUFFER, buffer, gl.DYNAMIC_DRAW);
  //gl.bufferData(gl.ARRAY_BUFFER, this.vertices * this.elementSize, gl.DYNAMIC_DRAW);
  //gl.bufferSubData(gl.ARRAY_BUFFER, 0, buffer);

  time = (new Date() - start) / 1000.0;
  OK.debug(time + " seconds to update vertex buffer object");
}

Renderer.prototype.updateBuffers = function() {
  if (this.border) {
    this.box(vis.options.min, vis.options.max);
    this.elements = 24;
    return;
  }

  //Count vertices
  this.elements = 0;
  for (var id in vis.objects) {
    if (this.type == "triangle" && vis.objects[id].triangles) {
      for (var t in vis.objects[id].triangles)
        this.elements += vis.objects[id].triangles[t].indices.data.length;
    } else if (this.type == "particle" && vis.objects[id].points) {
      for (var t in vis.objects[id].points)
        this.elements += vis.objects[id].points[t].vertices.data.length/3;
    } else if (this.type == "line" && vis.objects[id].lines) {
      for (var t in vis.objects[id].lines)
        this.elements += vis.objects[id].lines[t].indices.data.length;
    }
  }

  if (this.elements == 0) return;
  OK.debug("Updating " + this.type + " data... (" + this.elements + " elements)");

  //Load vertices and attributes into buffers
  var start = new Date();

  //Copy data to VBOs
  var buffer = new VertexBuffer(this.elements, this.elementSize);

  //Reload vertex buffers
  if (this.type == "particle") {
    //Process points

    /*/Auto 50% subsample when > 1M particles
    var subsample = 1;
    if ($("subsample").checked == true && newParticles > 1000000) {
      subsample = Math.round(newParticles/1000000 + 0.5);
      OK.debug("Subsampling at " + (100/subsample) + "% (" + subsample + ") to " + Math.floor(newParticles / subsample));
    }*/
    //Random subsampling
    //if (subsample > 1 && Math.random() > 1.0/subsample) continue;

    for (var id in vis.objects)
      if (vis.objects[id].points)
        buffer.loadParticles(vis.objects[id]);

  } else if (this.type == "line") {
    //Process lines
    for (var id in vis.objects)
      if (vis.objects[id].lines)
        buffer.loadLines(vis.objects[id]);

  } else if (this.type == "triangle") {
    //Process triangles
    for (var id in vis.objects)
      if (vis.objects[id].triangles)
        buffer.loadTriangles(vis.objects[id], id);
  }

  var time = (new Date() - start) / 1000.0;
  OK.debug(time + " seconds to load buffers... (elements: " + this.elements + " bytes: " + buffer.byteLength + ")");

  buffer.update(this.gl);
}

Renderer.prototype.box = function(min, max) {
  var vertices = new Float32Array(
        [
          min[0], min[1], max[2],
          min[0], max[1], max[2],
          max[0], max[1], max[2],
          max[0], min[1], max[2],
          min[0], min[1], min[2],
          min[0], max[1], min[2],
          max[0], max[1], min[2],
          max[0], min[1], min[2]
        ]);

  var indices = new Uint16Array(
        [
          0, 1, 1, 2, 2, 3, 3, 0,
          4, 5, 5, 6, 6, 7, 7, 4,
          0, 4, 3, 7, 1, 5, 2, 6
        ]
     );
  this.gl.bufferData(this.gl.ARRAY_BUFFER, vertices, this.gl.STATIC_DRAW);
  this.gl.bufferData(this.gl.ELEMENT_ARRAY_BUFFER, indices, this.gl.STATIC_DRAW);
}

Renderer.prototype.draw = function() {
  if (!vis.objects || !vis.objects.length) return;
  var start = new Date();
  //console.log(" ----- " + this.type + " --------------------------------------------------------------------");

  //Create buffer if not yet allocated
  if (this.vertexBuffer == undefined) {
    //Init shaders etc...
    //(This is done here now so we can skip triangle shaders if not required, 
    //due to really slow rendering when doing triangles and points... still to be looked into)
    if (!this.init()) return;
    OK.debug("Creating " + this.type + " buffers...");
    this.vertexBuffer = this.gl.createBuffer();
    this.indexBuffer = this.gl.createBuffer();
    //viewer.reload = true;
  }

  if (this.program.attributes["aVertexPosition"] == undefined) return; //Require vertex buffer

  viewer.webgl.use(this.program);
  viewer.webgl.setMatrices();

  //Bind buffers
  this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertexBuffer);
  this.gl.bindBuffer(this.gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);

  //Reload geometry if required
  viewer.canvas.mouse.disabled = true;
  if (this.reload) this.updateBuffers();
  if (this.sort || viewer.rotated) this.loadElements();
  this.reload = this.sort = false;
  viewer.canvas.mouse.disabled = false;

  if (this.elements == 0) return;

  //Enable attributes
  for (var key in this.program.attributes)
    this.gl.enableVertexAttribArray(this.program.attributes[key]);

  //Update palette
  //colours.update();

  //Gradient texture
  //this.gl.activeTexture(this.gl.TEXTURE0);
  //this.gl.bindTexture(this.gl.TEXTURE_2D, viewer.webgl.gradientTexture);
  //this.gl.uniform1i(this.program.uniforms["palette"], 0);

  //Options
  //var cmap = $("colourmap").checked == true ? 1 : 0;
  //this.gl.uniform1i(this.program.uniforms["colourmap"], cmap);

  /*/Image texture
  this.gl.activeTexture(this.gl.TEXTURE1);
  this.gl.bindTexture(this.gl.TEXTURE_2D, this.webgl.texture);
  this.gl.uniform1i(this.program.uniforms["texture"], 1);*/

  this.gl.uniform1f(this.program.uniforms["uAlpha"], viewer.opacity);
  if (this.colour)
    this.gl.uniform4f(this.program.uniforms["uColour"], this.colour.red/255.0, this.colour.green/255.0, this.colour.blue/255.0, this.colour.alpha);

  if (this.type == "particle") {

    this.gl.vertexAttribPointer(this.program.attributes["aVertexPosition"], 3, this.gl.FLOAT, false, this.elementSize, 0);
    this.gl.vertexAttribPointer(this.program.attributes["aVertexColour"], 4, this.gl.UNSIGNED_BYTE, true, this.elementSize, this.attribSizes[0]);
    this.gl.vertexAttribPointer(this.program.attributes["aVertexSize"], 1, this.gl.FLOAT, false, this.elementSize, this.attribSizes[0]+this.attribSizes[1]);
    this.gl.vertexAttribPointer(this.program.attributes["aPointType"], 1, this.gl.FLOAT, false, this.elementSize, this.attribSizes[0]+this.attribSizes[1]+this.attribSizes[2]);

    //Set uniforms...
    this.gl.uniform1i(this.program.uniforms["uPointType"], viewer.pointType);
    this.gl.uniform1f(this.program.uniforms["uPointScale"], viewer.pointScale*viewer.modelsize);

    //Draw points
    this.gl.drawElements(this.gl.POINTS, this.elements, this.gl.UNSIGNED_INT, 0);
    //this.gl.drawElements(this.gl.POINTS, this.positions.length, this.gl.UNSIGNED_SHORT, 0);
    //this.gl.drawArrays(this.gl.POINTS, 0, this.positions.length);
    var desc = this.elements + " points";

  } else if (this.type == "triangle") {

    this.gl.vertexAttribPointer(this.program.attributes["aVertexPosition"], 3, this.gl.FLOAT, false, this.elementSize, 0);
    this.gl.vertexAttribPointer(this.program.attributes["aVertexNormal"], 3, this.gl.FLOAT, false, this.elementSize, this.attribSizes[0]);
    this.gl.vertexAttribPointer(this.program.attributes["aVertexColour"], 4, this.gl.UNSIGNED_BYTE, true, this.elementSize, this.attribSizes[0]+this.attribSizes[1]);
    this.gl.vertexAttribPointer(this.program.attributes["aVertexObjectID"], 1, this.gl.UNSIGNED_BYTE, false, this.elementSize, this.attribSizes[0]+this.attribSizes[1]+this.attribSizes[2]);
    this.gl.vertexAttribPointer(this.program.attributes["aVertexTexCoord"], 2, this.gl.UNSIGNED_BYTE, true, this.elementSize, this.attribSizes[0]+this.attribSizes[1]+this.attribSizes[2]+1);

    //Set uniforms...
    //var colour = new Colour(parseInt(vis.colourmaps[i].colours[j].colour), vis.colourmaps[i].colours[j].position));
    //this.gl.uniform4f(this.program.uniforms["uColour"], 1.0, 1.0, 1.0, 0.0);
    //this.gl.enable(this.gl.CULL_FACE);
    //this.gl.cullFace(this.gl.BACK_FACE);
    
    //Per-object uniform arrays
    var cullfaces = [];
    for (var id in vis.objects)
      cullfaces.push(vis.objects[id].cullface ? 1 : 0);

    this.gl.uniform1iv(this.program.uniforms["uCullFace"], cullfaces);

    //Draw triangles
    this.gl.drawElements(this.gl.TRIANGLES, this.elements, this.gl.UNSIGNED_INT, 0);
    //this.gl.drawElements(this.gl.TRIANGLES, this.positions.length*3, this.gl.UNSIGNED_SHORT, 0);
    //this.gl.drawArrays(this.gl.TRIANGLES, 0, this.positions.length*3);
    var desc = (this.elements / 3) + " triangles";

  } else if (this.border) {
    this.gl.vertexAttribPointer(this.program.attributes["aVertexPosition"], 3, this.gl.FLOAT, false, 0, 0);
    this.gl.vertexAttribPointer(this.program.attributes["aVertexColour"], 4, this.gl.UNSIGNED_BYTE, true, 0, 0);
    this.gl.drawElements(this.gl.LINES, this.elements, this.gl.UNSIGNED_SHORT, 0);
    var desc = "border";

  } else if (this.type == "line") {

    this.gl.vertexAttribPointer(this.program.attributes["aVertexPosition"], 3, this.gl.FLOAT, false, this.elementSize, 0);
    this.gl.vertexAttribPointer(this.program.attributes["aVertexColour"], 4, this.gl.UNSIGNED_BYTE, true, this.elementSize, this.attribSizes[0]);

    //Draw lines
    this.gl.drawElements(this.gl.LINES, this.elements, this.gl.UNSIGNED_INT, 0);
    var desc = (this.elements / 2) + " lines";
  }

  //Disable attribs
  for (var key in this.program.attributes)
    this.gl.disableVertexAttribArray(this.program.attributes[key]);

  this.gl.bindBuffer(this.gl.ARRAY_BUFFER, null);
  this.gl.bindBuffer(this.gl.ELEMENT_ARRAY_BUFFER, null);
  this.gl.useProgram(null);

  var time = (new Date() - start) / 1000.0;
  if (time > 0.01) OK.debug(time + " seconds to draw " + desc);
}

function minMaxDist()
{
  //Calculate min/max distances from view plane
  var M2 = [viewer.webgl.modelView.matrix[0*4+2],
            viewer.webgl.modelView.matrix[1*4+2],
            viewer.webgl.modelView.matrix[2*4+2],
            viewer.webgl.modelView.matrix[3*4+2]];
  var maxdist = -Number.MAX_VALUE, mindist = Number.MAX_VALUE;
  for (i=0; i<2; i++)
  {
     var x = i==0 ? vis.options.min[0] : vis.options.max[0];
     for (var j=0; j<2; j++)
     {
        var y = j==0 ? vis.options.min[1] : vis.options.max[1];
        for (var k=0; k<2; k++)
        {
           var z = k==0 ? vis.options.min[2] : vis.options.max[2];
           var dist = eyeDistance(M2, [x, y, z]);
           if (dist < mindist) mindist = dist;
           if (dist > maxdist) maxdist = dist;
        }
     }
  }
  //alert(mindist + " --> " + maxdist);
  if (maxdist == mindist) maxdist += 0.0000001;
  return [mindist, maxdist];
}

//This object holds the viewer details and calls the renderers
function Viewer(canvas) {
  this.canvas = canvas;
  try {
    this.webgl = new WebGL(this.canvas, {antialias: true, premultipliedAlpha: false});
    this.gl = this.webgl.gl;
    this.ext = (
      this.gl.getExtension('OES_element_index_uint') ||
      this.gl.getExtension('MOZ_OES_element_index_uint') ||
      this.gl.getExtension('WEBKIT_OES_element_index_uint')
    );
  } catch(e) {
    //No WebGL
    OK.debug(e);
    if (!this.webgl) $('canvas').style.display = 'none';
  }

  //Default colour editor
  this.gradient = new GradientEditor($('palette'), paletteUpdate);

  this.width = 0; //Auto resize
  this.height = 0;

  this.rotating = false;
  this.translate = [0,0,0];
  this.rotate = quat4.create();
  quat4.identity(this.rotate);
  this.focus = [0,0,0];
  this.centre = [0,0,0];
  this.near_clip = this.far_clip = 0.0;
  this.modelsize = 1;
  this.scale = [1, 1, 1];
  this.orientation = 1.0; //1.0 for RH, -1.0 for LH
  this.background = new Colour(0xff404040);
  document.body.style.background = this.background.html();
  $("bgColour").value = '';
  this.showBorder = $("border").checked;
  this.pointScale = 1.0;
  this.pointType = 0;
  this.opacity = 1.0;

  //Create the renderers
  if (this.gl) {
    this.points = new Renderer(this.gl, 'particle');
    this.triangles = new Renderer(this.gl, 'triangle');
    this.lines = new Renderer(this.gl, 'line');
    this.border = new Renderer(this.gl, 'line', 0xffffffff, true);

    this.gl.enable(this.gl.DEPTH_TEST);
    this.gl.depthFunc(this.gl.LEQUAL);
    //this.gl.depthMask(this.gl.FALSE);
    this.gl.enable(this.gl.BLEND);
    //this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);
    //this.gl.blendFuncSeparate(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA, this.gl.ZERO, this.gl.ONE);
    this.gl.blendFuncSeparate(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA, this.gl.ONE, this.gl.ONE_MINUS_SRC_ALPHA);
  }
}

Viewer.prototype.loadFile = function(source) {
  //Skip update to rotate/translate etc if in process of updating
  if (document.mouse.isdown) return;
  var start = new Date();
  var updated = true;
  try {
    if (typeof source != 'object') {
      if (source.indexOf("{") < 0) {
        if (server) {
          //Not a json string, assume a command script
          //TODO: this doesn't seem to work in LavaVR?
          var lines = source.split("\n");
          for (var line in lines) {
            sendCommand('' + lines[line]);
          }
          return;
        } else {
          console.log(source);
          alert("Invalid data");
          return;
        }
      } else if (source.substr(8) == "loadData") {
        //jsonp, strip function call when loaded from FileReader
        source = source.substring(10, source.lastIndexOf("\n"));
      }
      source = JSON.parse(source);
    }
  } catch(e) {
    console.log(source);
    alert(e);
  }
  var time = (new Date() - start) / 1000.0;
  OK.debug(time + " seconds to parse data");

  if (source.exported) {
    if (!vis.options) {OK.debug("Exported settings require loaded model"); return;}
    var old = this.toString();
    //Copy, overwriting if exists in source
    if (source.options.rotate) vis.options.rotate = source.options.rotate;
    Merge(vis, source);
    if (!source.reload) updated = false;  //No reload necessary
  } else {
    //Clear geometry
    for (var type in types)
      if (this[type])
        this[type].elements = 0;

    //Replace
    vis = source;
  }

  //Always set a bounding box
  if (!source.options.min) source.options.min = [0, 0, 0];
  if (!source.options.max) source.options.max = [1, 1, 1];

  //Load some user options...
  loadColourMaps();
  if (vis.options) {
    this.near_clip = vis.options.near_clip || 0;
    this.far_clip = vis.options.far_clip || 0;
    this.orientation = vis.options.orientation || 1;
    this.showBorder = vis.options.border;
    this.axes = vis.options.axes;
    this.pointScale = vis.options.pointScale || 1.0;
    this.pointType = vis.options.pointType >= -1 ? vis.options.pointType : 0;
    this.opacity = vis.options.opacity || 1.0;

    this.applyBackground(vis.options.background);

    //Copy global options to controls where applicable..
    $("bgColour").value = this.background.r;
    $("pointScale-out").value = (this.pointScale || 1.0);
    $("pointScale").value = $("pointScale-out").value * 10.0;
    $("globalAlpha").value = $("globalAlpha-out").value = (this.opacity || 1.0);
    $("border").checked = this.showBorder;
    $("axes").checked = this.axes;
    $("globalPointType").value = this.pointType;

    $('brightness').value = $("brightness-out").value = vis.properties.brightness;
    $('contrast').value = $("contrast-out").value = vis.properties.contrast;
    $('saturation').value = $("saturation-out").value = vis.properties.saturation;

    $('xmin').value = $("xmin-out").value = vis.properties.xmin;
    $('xmax').value = $("xmax-out").value = vis.properties.xmax;
    $('ymin').value = $("ymin-out").value = vis.properties.ymin;
    $('ymax').value = $("ymax-out").value = vis.properties.ymax;
    $('zmin').value = $("zmin-out").value = vis.properties.zmin;
    $('zmax').value = $("zmax-out").value = vis.properties.zmax;
  }

  this.updateDims(vis.options);
  //boundingBox(vis.options.min, vis.options.max);

  //Load objects and add to form
  var objdiv = $("objects");
  removeChildren(objdiv);

  //Decode into Float buffers and replace original base64 data
  //-Colour lookups: do in shader with a texture?
  //-Sizes don't change at per-particle level here, per-object? send id and calc size in shader? (short:id + short:size = 4 bytes)

  //min = [Number.MAX_VALUE, Number.MAX_VALUE, Number.MAX_VALUE];
  //max = [-Number.MAX_VALUE, -Number.MAX_VALUE, -Number.MAX_VALUE];

  //Process object data and convert base64 to Float32Array
  if (!source.exported) this.vertexCount = 0;
  for (var id in vis.objects) {
    var name = vis.objects[id].name;
    //Process points/triangles
    if (!source.exported) {
      for (var type in vis.objects[id]) {
        if (["triangles", "points"].indexOf(type) < 0) continue;
        if (type == "triangles") this.hasTriangles = true;
        if (type == "points") this.hasPoints = true;
        //Read vertices, values, normals, sizes, etc...
        for (var idx in vis.objects[id][type]) {
          decodeBase64(id, type, idx, 'vertices');
          decodeBase64(id, type, idx, 'values');
          decodeBase64(id, type, idx, 'normals');
          decodeBase64(id, type, idx, 'sizes');
          decodeBase64(id, type, idx, 'indices', 'integer');
          OK.debug("Loaded " + vis.objects[id][type][idx].vertices.data.length/3 + " vertices from " + name);
          this.vertexCount += vis.objects[id][type][idx].vertices.data.length/3;

          //Create indices for cross-sections
          if (type == 'triangles' && !vis.objects[id][type][idx].indices) {
            //Collect indices
            var h = vis.objects[id][type][idx].height;
            var w = vis.objects[id][type][idx].width;
            var buf = new Uint32Array((w-1)*(h-1)*6);
            var i = 0;
            for (var j=0; j < h-1; j++)  //Iterate over h-1 strips
            {
              var offset0 = j * w;
              var offset1 = (j+1) * w;
              for (var k=0; k < w-1; k++)  //Iterate width of strips, w vertices
              {
                 //Tri 1
                 buf[i++] = offset0 + k;
                 buf[i++] = offset1 + k;
                 buf[i++] = offset0 + k + 1;
                 //Tri 2
                 buf[i++] = offset1 + k;
                 buf[i++] = offset0 + k + 1;
                 buf[i++] = offset1 + k + 1;
              }
            }

            vis.objects[id][type][idx].indices = {"data" : buf, "type" : "integer"};
            /*/Get center point for depth sort...
            var verts = vis.objects[id][type][idx].vertices.data;
            vis.objects[id][type][idx].centroids = [null];
            vis.objects[id][type][idx].centroids = 
              [[
                (verts[0]+verts[verts.length-3])/2,
                (verts[1]+verts[verts.length-2])/2,
                (verts[2]+verts[verts.length-1])/2
              ]];*/
          }
        }
      }
    }

    var div= document.createElement('div');
    div.className = "object-control";
    objdiv.appendChild(div);

    var check= document.createElement('input');
    //check.checked = !vis.objects[id].hidden;
    check.checked = !(vis.objects[id].visible === false);
    check.setAttribute('type', 'checkbox');
    check.setAttribute('name', 'object_' + name);
    check.setAttribute('id', 'object_' + name);
    check.setAttribute('onchange', 'viewer.action(' + id + ', false, true, this);');
    div.appendChild(check);

    var label= document.createElement('label');
    label.appendChild(label.ownerDocument.createTextNode(name));
    label.setAttribute("for", 'object_' + name);
    div.appendChild(label);

    var props = document.createElement('input');
    props.type = "button";
    props.value = "...";
    props.id = "props_" + name;
    props.setAttribute('onclick', 'viewer.properties(' + id + ');');
    div.appendChild(props);
  }
  var time = (new Date() - start) / 1000.0;
  OK.debug(time + " seconds to import data");

  //Default to interactive render if vertex count < 0.5 M
  $("interactive").checked = (this.vertexCount <= 500000);

  //TODO: loaded custom shader is not replaced by default when new model loaded...
  for (var type in types) {
    if (this[type]) {
      //Custom shader load
      if (vis.shaders && vis.shaders[types[type]])
        this[type].init();
      //Set update flags
      this[type].sort = this[type].reload = updated;
    }
  }

  this.draw();
}

function Merge(obj1, obj2) {
  for (var p in obj2) {
    try {
      //alert(p + " ==> " + obj2[p].constructor);
      // Property in destination object set; update its value.
      if (obj2[p].constructor == Object || obj2[p].constructor == Array) {
        obj1[p] = Merge(obj1[p], obj2[p]);
      } else {
        obj1[p] = obj2[p];
      }
    } catch(e) {
      // Property in destination object not set; create it and set its value.
      obj1[p] = obj2[p];
    }
  }
  return obj1;
}

Viewer.prototype.toString = function() {
  var exp = {};

  //Copy camera settings
  vis.options.rotate = this.getRotation();
  vis.options.focus = this.focus;
  vis.options.translate = this.translate;
  vis.options.scale = this.scale;
  vis.options.pointScale = this.pointScale;
  vis.options.pointType = this.pointType;
  vis.options.border = this.showBorder;
  vis.options.axes = this.axes;
  vis.options.opacity = this.opacity;

  this.applyBackground(vis.options.background);

  exp.options = vis.options;
  exp.shaders = vis.shaders;
  exp.properties = vis.properties;
  exp.objects = [];
  exp.colourmaps = [];
  exp.options.background = this.background.toString();
  exp.exported = true;
  exp.reload = true;

  for (var id in vis.objects) {
    exp.objects[id] = {};
    //Skip points/triangles
    for (var type in vis.objects[id]) {
      if (type != "triangles" && type != "points") {
        exp.objects[id][type] = vis.objects[id][type];
      }
    }
  }
  if (vis.colourmaps) {
    for (var i=0; i<vis.colourmaps.length; i++) {
      exp.colourmaps[i] = {};
      for (var type in vis.colourmaps[i]) {
        if (type != "palette") {
          exp.colourmaps[i][type] = vis.colourmaps[i][type];
        }
      }
    }
  }

  //Export with 2 space indentation
  return JSON.stringify(exp, undefined, 2);
}

Viewer.prototype.exportFile = function() {
  if (server) {
     //Dump history to script
     //sendCommand('history history.script');
     //window.open('/history');
     cmdlog = '';
     this.setProperties();
     cmdlog += '\n#Object properties...\n';
     for (var id in vis.objects) {
       this.properties(id);
       this.setObjectProperties();
       cmdlog += '\n';
     }

     window.open('data:text/plain;base64,' + window.btoa(cmdlog));
     cmdlog = null;

  } else {
    //Export using data URL
    window.open('data:text/plain;base64,' + window.btoa(this.toString()));
  }
}

Viewer.prototype.properties = function(id) {
  //Properties button clicked... Copy to controls
  properties.id = id;
  $('obj_name').innerHTML = vis.objects[id].name;
  this.setColourMap(vis.objects[id].colourmap);

  function loadProp(name, def) {$(name + '-out').value = $(name).value = vis.objects[id][name] ? vis.objects[id][name] : def;}

  loadProp('opacity', 1.0);
  $('pointSize').value = vis.objects[id].pointsize ? vis.objects[id].pointsize : 10.0;
  $('pointType').value = vis.objects[id].pointtype ? vis.objects[id].pointtype : -1;

  $('wireframe').checked = vis.objects[id].wireframe;
  $('cullface').checked = vis.objects[id].cullface;

  loadProp('density');
  loadProp('power');
  loadProp('samples');
  loadProp('isovalue');
  loadProp('isosmooth');
  loadProp('isoalpha');
  $('isowalls').checked = vis.objects[id].isowalls;
  $('isofilter').checked = vis.objects[id].isofilter;

  var c = new Colour(vis.objects[id].colour);
  $S('colour_set').backgroundColor = c.html();

  //Type specific options
  setAll(vis.objects[id].points ? 'block' : 'none', 'point-obj');
  setAll(vis.objects[id].triangles ? 'block' : 'none', 'surface-obj');
  setAll(vis.objects[id].volumes ? 'block' : 'none', 'volume-obj');
  setAll(vis.objects[id].lines ? 'block' : 'none', 'line-obj');

  properties.show();
}

//Global property set
Viewer.prototype.setProperties = function() {
  function setProp(name, fieldname) {
    if (fieldname == undefined) fieldname = name;
    vis.properties[name] = $(fieldname + '-out').value = $(fieldname).value;
  }

  viewer.pointScale = $('pointScale-out').value = $('pointScale').value / 10.0;
  viewer.opacity = $('globalAlpha-out').value = $('globalAlpha').value;
  viewer.pointType = $('globalPointType').value;
  viewer.showBorder = $("border").checked;
  viewer.axes = $("axes").checked;
  var c = $("bgColour").value;
  var cc = Math.round(255*c);
  vis.options.background = "rgba(" + cc + "," + cc + "," + cc + ",1.0)"
  setProp('brightness');
  setProp('contrast');
  setProp('saturation');
  setProp('xmin');
  setProp('xmax');
  setProp('ymin');
  setProp('ymax');
  setProp('zmin');
  setProp('zmax');

  //Set the local/server props
  if (server) {
    //Issue server commands
    sendCommand('select'); //Ensure no object selected
    sendCommand('scale points ' + viewer.pointScale);
    sendCommand('alpha ' + (viewer.opacity*255));
    sendCommand('pointtype all ' + viewer.pointType);
    sendCommand('border ' + (this.showBorder ? "on" : "off"));
    sendCommand('axis ' + (this.axes ? "on" : "off"));
    if (c != "") sendCommand('background ' + c);

    sendCommand('brightness=' + vis.properties.brightness);
    sendCommand('contrast=' + vis.properties.contrast);
    sendCommand('saturation=' + vis.properties.saturation);

    sendCommand('xmin=' + vis.properties.xmin);
    sendCommand('xmax=' + vis.properties.xmax);
    sendCommand('ymin=' + vis.properties.ymin);
    sendCommand('ymax=' + vis.properties.ymax);
    sendCommand('zmin=' + vis.properties.zmin);
    sendCommand('zmax=' + vis.properties.zmax);
  } else {
    viewer.applyBackground(vis.options.background);
    viewer.draw();
  }
}

Viewer.prototype.setTimeStep = function() {
  //TODO: To implement this accurately, need to get timestep range from server
  //For now just do a jump and reset slider to middle
  var timejump = $("timestep").value - 50.0;
  $("timestep").value = 50;
  if (server) {
    //Issue server commands
    sendCommand('jump ' + timejump);
  } else {
    //No timesteps supported in WebGL viewer yet
  }
}

Viewer.prototype.applyBackground = function(bg) {
  if (!bg) return;
  this.background = new Colour(bg);
  var hsv = this.background.HSV();
  if (this.border) this.border.colour = hsv.V > 50 ? new Colour(0xff444444) : new Colour(0xffbbbbbb);
  document.body.style.background = this.background.html();
}

Viewer.prototype.addColourMap = function() {
  if (properties.id == undefined) return;
  //New colourmap on active object
  if (server)
    sendCommand("colourmap " + vis.objects[properties.id].id + " add");

  var id = 1;
  if (vis.colourmaps.length > 0) id = vis.colourmaps[vis.colourmaps.length-1].id + 1;
  var newmap = {
    "id": id,
    "name": "ColourMap " + id,
    "minimum": 0,
    "maximum": 1,
    "log": 0,
    "colours": [{"position": 0, "colour": "rgba(0,0,0,0)"},{"position": 1,"colour": "rgba(255,255,255,1)"}
    ]
  }
  vis.colourmaps.push(newmap);
  loadColourMaps();
  var list = $('colourmap-presets');
  list.value = list.options[list.options.length-1].value;
}

Viewer.prototype.setColourMap = function(id) {
  if (properties.id == undefined) return;
  vis.objects[properties.id].colourmap = id;
  if (id === undefined) id = -1;
  //Set new colourmap on active object
  $('colourmap-presets').value = id;
  if (id < 0) {
    $('palette').style.display = 'none';
    $('log').style.display = 'none';
  } else {
    //Draw palette UI
    $('logscale').checked = vis.colourmaps[id].log;
    $('log').style.display = 'block';
    $('palette').style.display = 'block';
    viewer.gradient.palette = vis.colourmaps[id].palette;
    viewer.gradient.mapid = id; //Save the id
    viewer.gradient.update();
  }
}

Viewer.prototype.setObjectProperties = function() {
  //Copy from controls
  var id = properties.id;
  function setProp(name) {vis.objects[id][name] = $(name + '-out').value = $(name).value;}
  setProp('opacity');
  vis.objects[id].pointsize = $('pointSize-out').value = $('pointSize').value / 10.0;
  vis.objects[id].pointtype = $('pointType').value;
  vis.objects[id].wireframe = $('wireframe').checked;
  vis.objects[id].cullface = $('cullface').checked;
  setProp('density');
  setProp('power');
  setProp('samples');
  setProp('isovalue');
  setProp('isosmooth');
  setProp('isoalpha');
  vis.objects[id].isowalls = $('isowalls').checked;
  vis.objects[id].isofilter = $('isofilter').checked;
  var colour = new Colour($('colour_set').style.backgroundColor);
  vis.objects[id].colour = colour.toInt();
  if (vis.objects[id].colourmap >= 0)
    vis.colourmaps[vis.objects[id].colourmap].log = $('logscale').checked;

  for (var type in types) {
    if (vis.objects[id][type])
      this[type].sort = this[type].reload = true;
  }
  viewer.draw();

  //Server side...
  if (server) {
    sendCommand("select " + vis.objects[properties.id].id);
    sendCommand("colour " + colour.hex());
    sendCommand("opacity " + vis.objects[id].opacity);
    //sendCommand('brightness=' + vis.objects[id].brightness);
    //sendCommand('contrast=' + vis.objects[id].contrast);
    //sendCommand('saturation=' + vis.objects[id].saturation);
    if (vis.objects[id].points) {
      sendCommand("scale " + vis.objects[id].pointsize);
      sendCommand("pointtype " + vis.objects[id].pointtype);
    }
    if (vis.objects[id].triangles) {
      sendCommand("wireframe=" + (vis.objects[id].wireframe ? "1" : "0"));
      sendCommand('cullface=' + (vis.objects[id].cullface ? "1" : "0"));
    }
    if (vis.objects[id].volumes) {
      //TODO: volume settings here, create volume options in dialog
      sendCommand('density=' + vis.objects[id].density);
      sendCommand('power=' + vis.objects[id].power);
      sendCommand('samples=' + vis.objects[id].samples);
      sendCommand('isosmooth=' + vis.objects[id].isosmooth);
      sendCommand('isoalpha=' + vis.objects[id].isoalpha);
      sendCommand('isowalls=' + (vis.objects[id].isowalls ? "1" : "0"));
      sendCommand('tricubicfilter=' + (vis.objects[id].isofilter ? "1" : "0"));
      sendCommand('isovalue=' + vis.objects[id].isovalue);
    }
    /*sendCommand('xmin=' + vis.objects[id].xmin);
    sendCommand('xmax=' + vis.objects[id].xmax);
    sendCommand('ymin=' + vis.objects[id].ymin);
    sendCommand('ymax=' + vis.objects[id].ymax);
    sendCommand('zmin=' + vis.objects[id].zmin);
    sendCommand('zmax=' + vis.objects[id].zmax);*/

    //Get colourmap
    //console.log(JSON.stringify(vis.colourmaps));
    var cmid = vis.objects[properties.id].colourmap;
    if (cmid == undefined || cmid < 0) {
      sendCommand("colourmap -1");
    } else {
      sendCommand("colourmap " + vis.colourmaps[cmid].id);
      //Update full colourmap...
      sendCommand("colourmap \"\n" + vis.colourmaps[cmid].palette + "\n\"\n");
    }

    sendCommand("select");
  }
}

Viewer.prototype.action = function(id, reload, sort, el) {
  //Object checkbox clicked
  if (server) {
    var show = el.checked;
    if (show)
      sendCommand("show " + vis.objects[id].id);
    else
      sendCommand("hide " + vis.objects[id].id);
    return;
  }

  $('apply').disabled = false;

  for (var type in types) {
    if (vis.objects[id][type])
      this[type].sort = sort;
      this[type].reload = reload;
  }
}

Viewer.prototype.apply = function() {
  $('apply').disabled = true;
  this.draw();
}

function decodeBase64(id, type, idx, datatype, format) {
  if (!format) format = 'float';
  if (!vis.objects[id][type][idx][datatype]) return null;
  var buf;
  if (typeof vis.objects[id][type][idx][datatype].data == 'string') {
    //Base64 encoded string
    var decoded = atob(vis.objects[id][type][idx][datatype].data);
    var buffer = new ArrayBuffer(decoded.length);
    var bufView = new Uint8Array(buffer);
    for (var i=0, strLen=decoded.length; i<strLen; i++) {
      bufView[i] = decoded.charCodeAt(i);
    }
    if (format == 'float')
      buf = new Float32Array(buffer);
    else
      buf = new Uint32Array(buffer);
  } else {
    //Literal array
    if (format == 'float')
      buf = new Float32Array(vis.objects[id][type][idx][datatype].data);
    else
      buf = new Uint32Array(vis.objects[id][type][idx][datatype].data);
  }

  vis.objects[id][type][idx][datatype].data = buf;
  vis.objects[id][type][idx][datatype].type = format;

  if (datatype == 'values') {
    if (!vis.objects[id][type][idx].values.minimum ||
        !vis.objects[id][type][idx].values.maximum) {
      //Value field max min
      var minval = Number.MAX_VALUE, maxval = -Number.MAX_VALUE;
      for (var i=0; i<buf.length; i++) {
        if (buf[i] > maxval)
          maxval = buf[i];
        if (buf[i] < minval)
          minval = buf[i];
      }

      //Set from data where none provided...
      if (!vis.objects[id][type][idx].values.minimum)
        vis.objects[id][type][idx].values.minimum = minval;
      if (!vis.objects[id][type][idx].values.maximum)
        vis.objects[id][type][idx].values.maximum = maxval;
    }
  }
}

function removeChildren(element) {
  if (element.hasChildNodes()) {
    while (element.childNodes.length > 0 )
    element.removeChild(element.firstChild);
  }
}

paletteUpdate = function(obj) {
  //Load colourmap change
  if (!vis.colourmaps || viewer.gradient.mapid >= 0) return;
  var canvas = $('palette');
  var context = canvas.getContext('2d');  
  if (!context) alert("getContext failed");

  var cmap = vis.colourmaps[viewer.gradient.mapid];
  if (!cmap) return;

  //Get colour data and store in array
  //Redraw without UI elements
  cmap.palette.draw(canvas, false);
  //Update colour data
  cmap.colours = cmap.palette.colours;
  //Cache colour values
  var pixels = context.getImageData(0, 0, 512, 1).data;
  for (var c=0; c<512; c++)
    cmap.palette.cache[c] = pixels[c*4] + (pixels[c*4+1] << 8) + (pixels[c*4+2] << 16) + (pixels[c*4+3] << 24);

  //Redraw UI
  cmap.palette.draw(canvas, true);
}

Viewer.prototype.draw = function(borderOnly) {
  //If requested draw border only (used while interacting)
  //Draw the full model on a timer
  viewer.drawFrame(borderOnly);
  if (borderOnly && !server) {
    if (this.drawTimer)
      clearTimeout(this.drawTimer);
    this.drawTimer = setTimeout(function () {viewer.drawFrame();}, 100 );
  }
}

Viewer.prototype.drawFrame = function(borderOnly) {
  if (!this.canvas) return;

  if (server && !document.mouse.isdown) {
    //Don't draw in server mode unless interacting (border view)
    this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);
    return;
  }
  
  //Show screenshot while interacting or if using server
  //if (server || borderOnly)
  if (server) {
    $("frame").style.display = 'block';
    var frame = $('frame');
    this.width = frame.offsetWidth;
    this.height = frame.offsetHeight;
    this.canvas.style.width = this.width + "px";
    this.canvas.style.height = this.height + "px";
  }
  else
    $("frame").style.display = 'none';
  
  if (!this.gl) return;

  if (this.width == 0 || this.height == 0) {
    //Get size from window
    this.width = window.innerWidth;
    this.height = window.innerHeight;
  }

  if (this.width != this.canvas.width || this.height != this.canvas.height) {
    this.canvas.width = this.width;
    this.canvas.height = this.height;
    this.canvas.setAttribute("width", this.width);
    this.canvas.setAttribute("height", this.height);
    if (this.gl) {
      this.gl.viewportWidth = this.width;
      this.gl.viewportHeight = this.height;
      //this.webgl.viewport = new Viewport(0, 0, this.width, this.height);      
    }
  }
  this.width = this.height = 0;

  var start = new Date();

  this.gl.viewport(0, 0, this.gl.viewportWidth, this.gl.viewportHeight);
    //console.log(JSON.stringify(this.webgl.viewport));
  //this.gl.clearColor(this.background.red/255, this.background.green/255, this.background.blue/255, server || document.mouse.isdown ? 0 : 1);
  this.gl.clearColor(this.background.red/255, this.background.green/255, this.background.blue/255, server ? 0 : 1);
  //this.gl.clearColor(this.background.red/255, this.background.green/255, this.background.blue/255, 0);
  //this.gl.clearColor(1, 1, 1, 0);
  this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

  this.webgl.setPerspective(45, this.gl.viewportWidth / this.gl.viewportHeight, this.near_clip, this.far_clip);

  //Apply translation to origin, any rotation and scaling (inverse of zoom factor)
  this.webgl.modelView.identity()
  this.webgl.modelView.translate(this.translate)
  // Adjust centre of rotation, default is same as focal point so this does nothing...
  adjust = [-(this.focus[0] - this.centre[0]), -(this.focus[1] - this.centre[1]), -(this.focus[2] - this.centre[2])];
  this.webgl.modelView.translate(adjust);

  // rotate model 
  var rotmat = quat4.toMat4(this.rotate);
  this.webgl.modelView.mult(rotmat);

  // Adjust back for rotation centre
  adjust = [this.focus[0] - this.centre[0], this.focus[1] - this.centre[1], this.focus[2] - this.centre[2]];
  this.webgl.modelView.translate(adjust);

  // Translate back by centre of model to align eye with model centre
  this.webgl.modelView.translate([-this.focus[0], -this.focus[1], -this.focus[2] * this.orientation]);

  // Apply scaling factors (including orientation switch if required)
  var scaling = [this.scale[0], this.scale[1], this.scale[2] * this.orientation];
  this.webgl.modelView.scale(scaling);
  //OK.debug(JSON.stringify(this.webgl.modelView));

   // Set default polygon front faces
   if (this.orientation == 1.0)
      this.gl.frontFace(this.gl.CCW);
   else
      this.gl.frontFace(this.gl.CW);

  //Render objects
  if (!borderOnly) {
  //if (!borderOnly && !server) {
    //Draw all
    for (var type in types) {
      //if (!document.mouse.isdown && !this.showBorder && type == 'border') continue;
      if (type == 'border') continue;
      this[type].draw();
    }
  }

  if (borderOnly || this.showBorder)
    this.border.draw();

  /*/Save canvas image to background for display while interacting
  if (!borderOnly && !server) {
    if (document.mouse.isdown || !("frame").src) {
      //$("frame").src = $('canvas').toDataURL("image/png");
      //$('canvas').toBlob(function (blob) {$("frame").src =  URL.createObjectURL(blob);});
      this.border.draw();
    }
  }*/

  this.rotated = false; //Clear rotation flag
}

Viewer.prototype.rotateX = function(deg) {
  this.rotation(deg, [1,0,0]);
}

Viewer.prototype.rotateY = function(deg) {
  this.rotation(deg, [0,1,0]);
}

Viewer.prototype.rotateZ = function(deg) {
  this.rotation(deg, [0,0,1]);
}

Viewer.prototype.rotation = function(deg, axis) {
  //Quaterion rotate
  var arad = deg * Math.PI / 180.0;
  var rotation = quat4.fromAngleAxis(arad, axis);
  rotation = quat4.normalize(rotation);
  this.rotate = quat4.multiply(rotation, this.rotate);
}

Viewer.prototype.getRotation = function() {
  return [viewer.rotate[0], viewer.rotate[1], viewer.rotate[2], viewer.rotate[3]];
}

Viewer.prototype.getRotationString = function() {
  //Return current rotation quaternion as string
  var q = this.getRotation();
  return 'rotation ' + q[0] + ' ' + q[1] + ' ' + q[2] + ' ' + q[3];
}

Viewer.prototype.getTranslationString = function() {
  return 'translation ' + this.translate[0] + ' ' + this.translate[1] + ' ' + this.translate[2];
}

Viewer.prototype.reset = function() {
  if (this.gl) {
    this.updateDims(vis.options);
    this.draw();
  }

  if (server) {
    //sendCommand('' + this.getRotationString());
    //sendCommand('' + this.getTranslationString());
    sendCommand('reset');
  }
}

Viewer.prototype.zoom = function(factor) {
  if (this.gl) {
    this.translate[2] += factor * this.modelsize;
    this.draw();
  }

  if (server)
    sendCommand('' + this.getTranslationString());
    //sendCommand('zoom ' + factor);
}

Viewer.prototype.zoomClip = function(factor) {
  if (this.gl) {
     var near_clip = this.near_clip + factor * this.modelsize;
     if (near_clip >= this.modelsize * 0.001)
       this.near_clip = near_clip;
    this.draw();
  }
  if (server)
    sendCommand('zoomclip ' + factor);
}

Viewer.prototype.updateDims = function(options) {
  if (!options) return;
  var oldsize = this.modelsize;
  this.dims = [options.max[0] - options.min[0], options.max[1] - options.min[1], options.max[2] - options.min[2]];
  this.modelsize = Math.sqrt(this.dims[0]*this.dims[0] + this.dims[1]*this.dims[1] + this.dims[2]*this.dims[2]);

  this.focus = [options.min[0] + 0.5*this.dims[0], options.min[1] + 0.5*this.dims[1], options.min[2] + 0.5*this.dims[2]];
  this.centre = [this.focus[0],this.focus[1],this.focus[2]];

  this.translate = [0,0,0];
  if (this.modelsize != oldsize) this.translate[2] = -this.modelsize*1.25;

  this.near_clip = this.modelsize / 10.0;   
  this.far_clip = this.modelsize * 10.0;

  quat4.identity(this.rotate);
  this.rotated = true; 

  if (options) {
    //Initial rotation
    if (options.rotate) {
      if (options.rotate.length == 3) {
        this.rotateZ(-options.rotate[2]);
        this.rotateY(-options.rotate[1]);
        this.rotateX(-options.rotate[0]);
      } else if (options.rotate.length == 4) {
        this.rotate = quat4.create(options.rotate);
      }
    }

    //Translate
    if (options.translate) {
      this.translate[0] = options.translate[0];
      this.translate[1] = options.translate[1];
      this.translate[2] = options.translate[2];
    }

    //Scale
    if (options.scale) {
      this.scale[0] = options.scale[0];
      this.scale[1] = options.scale[1];
      this.scale[2] = options.scale[2];
    }

    //Focal point
    if (options.focus) {
      this.focus[0] = this.centre[0] = options.focus[0];
      this.focus[1] = this.centre[1] = options.focus[1];
      this.focus[2] = this.centre[2] = options.focus[2];
    }

  }

  //OK.debug("DIMS: " + min[0] + " to " + max[0] + "," + min[1] + " to " + max[1] + "," + min[2] + " to " + max[2]);
  OK.debug("New model size: " + this.modelsize + ", Focal point: " + this.focus[0] + "," + this.focus[1] + "," + this.focus[2]);
  OK.debug("Translate: " + this.translate[0] + "," + this.translate[1] + "," + this.translate[2]);
}

function resizeToWindow() {
  //var canvas = $('canvas');
  //if (canvas.width < window.innerWidth || canvas.height < window.innerHeight)
    sendCommand('resize ' + window.innerWidth + " " + window.innerHeight);
  var frame = $('frame');
  canvas.style.width = frame.style.width = "100%";
  canvas.style.height = frame.style.height = "100%";
}


