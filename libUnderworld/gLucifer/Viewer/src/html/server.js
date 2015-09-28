/////////////////////////////////////////////////////////////////////////
//Server Event handling
//TODO: get rid of globals and put in a class

function keyPressCommand(event, el) {
  if (event.keyCode == 13) {
    //OK.debug("PRESS Code: " + event.keyCode + " Char: " + charc);
    var cmd = el.value.trim();
    if (cmd.length == 0) cmd = "repeat";
    sendCommand('' + cmd);
    el.value = "";
  }
}

function keyPress(event) {
  if (event.target == document.getElementById('cmdInput')) return;
  var charc = '';
  var code = 0;
  var key = 0;
  if (event.which && event.charCode != 0)
     code = event.which; //charc = String.fromCharCode(event.which);	  // All others
  else
     // ie or special key
     code = event.keyCode;

  //Ignore ESC, too easy to accidentally quit
  if (code == 27) return;

  //Special key codes
  if (code == 38) key = 17;
  else if (code == 40) key = 18;
  else if (code == 37) key = 20;
  else if (code == 39) key = 19;
  else if (code == 33) key = 24;
  else if (code == 34) key = 25;
  else if (code == 36) key = 22;
  else if (code == 35) key = 23;
  else key = code; 

  //OK.debug("PRESS Code: " + event.keyCode + " Char: " + charc);
  requestData('/key=' + key + ',modifiers=' + getModifiers(event) + ",x=" + defaultMouse.x + ",y=" + defaultMouse.y);
}

function getModifiers(event) {
  var modifiers = '';
  if (event.ctrlKey) modifiers += 'C';
  if (event.shiftKey) modifiers += 'S';
  if (event.altKey) modifiers += 'A';
  if (event.metaKey) modifiers += 'M';
  return modifiers;
}

//Mouse event handling
function serverMouseClick(event, mouse) {
  if (event.button > 0) return true;
  if (mvtimeout) clearTimeout(mvtimeout);  //Clear move timer
  if (mvcount > 0) {
    //Move queued
    var request = "/mouse=move,button=" + document.mouse.button + ",x=" + document.mouse.x + ",y=" + document.mouse.y;
    mouseCall(request);
    mvcuont = 0;
  }
  requestData('/mouse=up,button=' + (mouse.button+1) + ',modifiers=' + getModifiers(event) + ",x=" + mouse.x + ",y=" + mouse.y);
}

function serverMouseDown(event, mouse) {
  if ($('tmode').checked)
    var button = Math.abs(mouse.button-2)+1;
  else 
    var button = mouse.button+1;
  requestData('/mouse=down,button=' + button + ",x=" + mouse.x + ",y=" + mouse.y);
  return false; //Prevent drag
}

var mvtimeout = undefined;
var spintimeout = undefined;
var mvcount = 0;
var spincount = 0;

function serverMouseMove(event, mouse) {
  //Mouseover processing
  if (mouse.x >= 0 && mouse.y >= 0 && mouse.x <= mouse.element.width && mouse.y <= mouse.element.height)
  {
    //Convert mouse coords
    //...
    //document.getElementById("coords").innerHTML = "&nbsp;x: " + mouse.x + " y: " + mouse.y;
  }
  if (!mouse.isdown) return true;

  //Right & middle buttons: drag to scroll
  if (mouse.button > 0) {
    // Set the scroll position
    //window.scrollBy(-mouse.deltaX, -mouse.deltaY);
    //return true;
  }


  //Drag processing
  //requestData('/mouse=move,button=' + (mouse.button+1) + ",x=" + mouse.x + ",y=" + mouse.y);
  if (mvtimeout) clearTimeout(mvtimeout);
  if (0) { //mvcount > 20) {
    moveCall(); //Instant call when count > 20
  } else {
    document.body.style.cursor = "wait";
    var request = "/mouse=move,button=" + document.mouse.button + ",x=" + document.mouse.x + ",y=" + document.mouse.y;
    mvtimeout = setTimeout("mouseCall('" + request + "'); mvcount = 0;", 100);
    mvcount ++;
  }
  return false;
}

function serverMouseWheel(event, mouse) {
  if (spintimeout) clearTimeout(spintimeout);
  document.body.style.cursor = "wait";
  spincount += event.spin;
  var request = "/mouse=scroll,spin=" + spincount + ',modifiers=' + getModifiers(event) + ",x=" + document.mouse.x + ",y=" + document.mouse.y;
  spintimeout = setTimeout("mouseCall('" + request + "'); spincount = 0;", 100);
  //requestData('/mouse=scroll,spin=' + event.spin + ',modifiers=' + getModifiers(event) + ",x=" + mouse.x + ",y=" + mouse.y);
}

function mouseCall(request) {
  document.body.style.cursor = "default";
  requestData(request);
  //requestData('/mouse=move,button=' + document.mouse.button + ",x=" + document.mouse.x + ",y=" + document.mouse.y);
}

///////////////////////////////////////////////////////
var cmdlog = null;
function sendCommand(cmd) {
  //Shortcut to send commands (and optionally log)
  requestData('/command=' + cmd);
  if (cmdlog != null)
    cmdlog += cmd + "\n";
}

var count = 0;
function requestData(data, callback, sync) {
  var http = new XMLHttpRequest();
  // the url of the script where we send the asynchronous call
  var url = data.replace(/\n/g, ';'); //Replace newlines with semi-colon
  //console.log(url);
  //Add count to url to prevent caching
  if (data) {
    url += "&" + count;
    count++;
  }

  http.onreadystatechange = function() { 
    if(http.readyState == 4)
      if(http.status == 200) {
        if (callback)
          callback(http.responseText);
        else
          OK.debug(http.responseText);
      } else  
        OK.debug("Ajax Request Error: " + url + ", returned status code " + http.status + " " + http.statusText);
  } 

  //Add date to url to prevent caching
  //var d = new Date();
  //http.open("GET", url + "?d=" + d.getTime(), true); 
  http.open("GET", url, !sync); 
  http.send(null); 
}

function requestImage() {
  if (client_id < 0) return; //No longer connected
  var http = new XMLHttpRequest();
  //Add count to url to prevent caching
  var url = '/image=' + client_id + '&' + count; count++;

  http.onload = function() { 
    if(http.status == 200) {
      var frame = document.getElementById('frame');
      //Clean up when loaded
      frame.onload = function(e) {window.URL.revokeObjectURL(frame.src);};
      frame.src = window.URL.createObjectURL(http.response);

      //Update the object state, then request next image
      requestData('/objects', parseObjects);
      //Get next frame (after brief timeout so we don't flood the server)
      //if (imgtimer) clearTimeout(imgtimer);
      //imgtimer = setTimeout(requestImage, 100);
    } else  
      OK.debug("Ajax Request Error: " + url + ", returned status code " + http.status + " " + http.statusText);
  } 

  http.open("GET", url, true); 
  http.responseType = 'blob';
  http.send(null); 
}

//Get client_id after connect call
var client_id = 0;
function parseRequest(response) {
  client_id = parseInt(response);
  requestData('/objects', parseObjects);
  //requestImage();
}

var imgtimer;
function parseObjects(response) {
  viewer.loadFile(response);
  //Get next frame (after brief timeout so we don't flood the server)
  if (imgtimer) clearTimeout(imgtimer);
  imgtimer = setTimeout(requestImage, 100);
}

