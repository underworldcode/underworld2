function trim(str, chars) {
   if (str == null) return "";
	return ltrim(rtrim(str, chars), chars);
}
 
function ltrim(str, chars) {
	chars = chars || "\\s";
	return str.replace(new RegExp("^[" + chars + "]+", "g"), "");
}
 
function rtrim(str, chars) {
	chars = chars || "\\s";
	return str.replace(new RegExp("[" + chars + "]+$", "g"), "");
}

function printNode(container, node) {
   var writeVal = true;

   if (node.nodeName == "#comment")
   {
      addSection(container, "div", "#" + node.nodeValue + " ", "font-weight: normal; font-style: italic; color: #aaaaaa");
      return;
   }

   if (node.tagName != null)
   {
      if (node.tagName == "include")
      {
         parseXMLfile(container, node.childNodes[0].nodeValue);
         writeVal = false;
      }
      else if (node.tagName == "param")
      {
         container = addSection(container, "div", null, "font-weight: normal; margin-left: 10px;");
      }
      else if (node.tagName == "struct")
      {
         container = addSection(container, "div", null, "font-weight: bold; margin-left: 10px;");
      }
      else if (node.tagName == "list")
      {
         container = addSection(container, "div", null, "font-weight: bold; margin-left: 10px;");
      }
      else
      {
         container = addSection(container, "div", "[" + node.tagName + "]", "font-weight: bold; margin-left: 10px;");
      }
   }
   
   if (writeVal && node.nodeValue && trim(node.nodeValue))
   {
      field = addSection(container, "input", null, "font-weight: normal; width: 400px;");
      field.value = node.nodeValue;
      field.type = "text"; //Type of field - can be any valid input type like text,file,checkbox etc.
   }

   if (node.getAttribute)
   {
      //Parse attribs
      var name = node.getAttribute("name")
      var merge = node.getAttribute("mergeType")
      if (name)
      {
         addSection(container, "span", name, "width: 300px; display: inline-block;");
         container = addSection(container, "span", null, null);
         //addSection(container, "span", name, "width: 400px; font-weight: normal");
      }
      if (merge)
         addSection(container, "span", " (mergeType: " + merge + ")", null);
   }

   if (!writeVal) return;

   for (var i = 0; i < node.childNodes.length; i++)
     printNode(container, node.childNodes[i]);
}

var id = 0;

function parseXMLfile(container, filename)
{
   xmlhttp=new XMLHttpRequest();

   xmlhttp.open("GET", filename, false);
   xmlhttp.send();

   parseXMLdoc(container, xmlhttp.responseXML, filename);
}


function parseXMLstring(str, filename, container) {
  var dp = new DOMParser();
  var doc = dp.parseFromString(str, "text/xml");
  if (!container) container = document.getElementById("items");

  parseXMLdoc(container, doc, filename);
}

function parseXMLdoc(container, doc, filename)
{
   id++; 
   var listid = "list" + id;
   container = addSection(container, "div", null, "border: 1px solid #bbb; border-radius: 4px; margin: 3px; font-weight: normal");

   link = addSection(container, "p", "[ " + filename + " ]", "font-weight: normal");
   link.listid = listid;

   container = addSection(container, "div", null, "font-weight: normal;");
   container.id = listid;

   printNode(container, doc);
}

//Add more fields dynamically.
function addField(area, labeltext, value) {
 var field_area = document.getElementById(area);
 
 if(document.createElement) { //W3C Dom method.
  var label = document.createElement("div");
  label.appendChild(label.ownerDocument.createTextNode(labeltext));
  var input = document.createElement("input");
  //input.id = field+count;
  //input.name = field+count;
  input.value = value;
  input.type = "text"; //Type of field - can be any valid input type like text,file,checkbox etc.
  label.appendChild(input);
  field_area.appendChild(label);
 }
 return input;
}

function addSection(area, eltype, text, style) {
  //var field_area = document.getElementById(area);
 
  if(document.createElement) { //W3C Dom method.
    var el = document.createElement(eltype);  //Type "span" or "div" etc

    if (style)
      el.style.cssText = style;

    if (text)
      el.appendChild(el.ownerDocument.createTextNode(text));

    //el.appendChild(input);

    area.appendChild(el);
  }
  return el;
}
