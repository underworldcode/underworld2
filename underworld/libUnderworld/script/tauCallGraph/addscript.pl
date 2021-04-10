#!/usr/bin/perl

$script=  q|<script type="text/ecmascript">
  <![CDATA[
    var typeInitialized = false;
    var typeInitializedN = false;
    var target;

    function changeEdgeThickness(evt,width) {
	evt.target.setAttributeNS(null,"stroke-width",width);
    }
    function changeEdge2(evt, flag) {
	var node=evt.target.parentNode;
	var tip = node.getElementsByTagName('title')[0].firstChild.textContent;
	var cwid = evt.target.getAttribute("stroke-width");
	evt.target.setAttribute("title",tip);
	if(cwid==2 && flag ==1){
	    evt.target.setAttributeNS(null,"stroke-width",12);
	}
	if(cwid ==12 && flag ==2){
	    evt.target.setAttributeNS(null,"stroke-width",2);
	}
    }
    function changeEdge(evt,width) {
      if(evt.detail == 1){ 
          evt.target.setAttributeNS(null,"stroke-width",width); 
      }
      else {
          evt.target.setAttributeNS(null,"stroke-width",2);
      }
    }
function xkeypress(evt){
    var svgnode=evt.target;
    var width = evt.target.getAttribute("width");
    var height = evt.target.getAttribute("height");
    var viewBox = evt.target.getAttribute("viewBox");
    var graph = evt.target.firstElementChild;
    var transform = graph.getAttribute("transform");
    var fac = 0.5;
    var width_int = parseInt(width);
    var ht_int = parseInt(height);
    
    //var parent = node.parentNode;
    //var svgnode = parent.parentNode;
    if (evt.type == "keypress") {
	//some browsers support evt.charCode, some only evt.keyCode
	if (evt.charCode) {
	    var charCode = evt.charCode;
	}
	else {
	    var charCode = evt.keyCode;
	}
	//	if(charCode == "1"){
	// 43 = +
	// 45 = -
	//alert(charCode);
	if(charCode == "45"){
	    width_int = fac*width_int;
	    ht_int = fac*ht_int;
	    width_int=parseInt(width_int);
	    ht_int = parseInt(ht_int);
	    width=width_int+'pt';
	    height=ht_int+'pt';
	    evt.target.setAttribute("width",width);
	    evt.target.setAttribute("height",height);
	}
	if(charCode == "43"){
	    width_int = 2*width_int;
	    ht_int = 2*ht_int;
	    width_int=parseInt(width_int);
	    ht_int = parseInt(ht_int);
	    width=width_int+'pt';
	    height=ht_int+'pt';
	    evt.target.setAttribute("width",width);
	    evt.target.setAttribute("height",height);
	}
	if(charCode == "47"){//*=42
	    width_int = 0.2*width_int;
	    ht_int = 0.2*ht_int;
	    width_int=parseInt(width_int);
	    ht_int = parseInt(ht_int);
	    width=width_int+'pt';
	    height=ht_int+'pt';
	    evt.target.setAttribute("width",width);
	    evt.target.setAttribute("height",height);
	}
	if(charCode == "42"){//*=/
	    width_int = 5*width_int;
	    ht_int = 5*ht_int;
	    width_int=parseInt(width_int);
	    ht_int = parseInt(ht_int);
	    width=width_int+'pt';
	    height=ht_int+'pt';
	    evt.target.setAttribute("width",width);
	    evt.target.setAttribute("height",height);
	}
	//alert("Pressed " + charCode);
	//}
    }

}//xkeypress
            function initTyping(evt) {
                if (!typeInitialized) {
                    document.documentElement.addEventListener("keypress",xkeypress,false);
                    document.documentElement.addEventListener("click",stopTyping,false);
		    typeInitialized = true;
                }
                //we don't want the click event on the document level to
                //immediately stop the typing mode
                evt.stopPropagation();
            }
            function stopTyping(evt) {
                    document.documentElement.removeEventListener("keypress",xkeypress,false);
                    document.documentElement.removeEventListener("click",stopTyping,false);
                    typeInitialized = false;
	    }
            function initTypingN(evt) {
                if (!typeInitializedN) {
		    //evt.target.addEventListener("keypress",doedges,false);
                    document.documentElement.addEventListener("keypress",doedges,false);
                    document.documentElement.addEventListener("click",stopTypingN,false);
		    //evt.target.addEventListener("click",stopTypingN,false);
		    typeInitializedN = true;
		    target=evt.target;
                }
                //we don't want the click event on the document level to
                //immediately stop the typing mode
                evt.stopPropagation();
            }
            function stopTypingN(evt) {
                    document.documentElement.removeEventListener("keypress",doedges,false);
                    document.documentElement.removeEventListener("click",stopTypingN,false);
                    typeInitializedN = false;
	    }
function doedges(evt){
      var node=target.parentNode;
      var str;
      var nname;
      var nvalue;
      var parent;
      var childs;
      var i;
      var loc;
      var pnodename;
      if (evt.type == "keypress") {
	  //some browsers support evt.charCode, some only evt.keyCode
	  if (evt.charCode) {
	      var charCode = evt.charCode;
	  }
	  else {
	      var charCode = evt.keyCode;
	  }
	  if(charCode == "49"){//"49" = 1
	  parent = node.parentNode;
	  var pnodename=node.getElementsByTagName('title')[0].firstChild.textContent;
	  if(parent.hasChildNodes()){
	      var alledges=parent.getElementsByClassName('edge');
	      for(i=0; i< alledges.length; i++){
		  var currentedge = alledges[i];
		  var etitle = currentedge.getElementsByTagName('title')[0];
		  loc = etitle.firstChild.textContent.indexOf("->");
		  var ename = etitle.firstChild.textContent.substr(0,loc);
		  if(ename == pnodename){
		      str="Found a match";
		      //so currentedge is what we are looking for
		      var cpath = currentedge.getElementsByTagName('path')[0];
		      var width=cpath.getAttribute("stroke-width");
		      if(width == 9){
			  cpath.setAttribute("stroke-width",2);
		      }
		      if(width == 2){
			  cpath.setAttribute("stroke-width",9);
		      }
		      if(width == 12){
			  cpath.setAttribute("stroke-width",9);
		      }
		  }//if(ename
	      }//for(i=
	  }//if(parent
	  }
      }//if (evt.type
}
    function getparent(evt) {
      var node=evt.target.parentNode;
      var str;
      var nname;
      var nvalue;
      var parent;
      var childs;
      var i;
      var loc;
      var pnodename;
      parent = node.parentNode;
      //initTyping(evt);
      //var newtitle = evt.target.title;
      //var childs = parent ? parent.getChildNodes() : null;

      //for (var i=0; childs && i<childs.getLength(); i++)
      //if (childs.item(i).getNodeType() == 3) // text node ..
       //  str= childs.item(i).getNodeValue();
      str= " xx ";
      var curedgetitle=node.getElementsByTagName('title')[0].firstChild.textContent;
      loc = node.getElementsByTagName('title')[0].firstChild.textContent.indexOf("->");
      pnodename=node.getElementsByTagName('title')[0].firstChild.textContent.substr(0,loc);
      if(evt.detail == 1){
      if(parent.hasChildNodes()){
         //alert("Test "+parent.id+ " " + node.id);
         //childs = parent.childNodes;
         var tt=parent.getElementsByTagName('title')[0];
	 var alledges=parent.getElementsByClassName('edge');
	 var allnodes=parent.getElementsByClassName('node');
         var ttc=tt.firstChild.textContent;
	 for(i=0; i< alledges.length; i++){
	     var currentedge = alledges[i];
	     var etitle = currentedge.getElementsByTagName('title')[0];
	     loc = etitle.firstChild.textContent.indexOf("->");
	     var ename = etitle.firstChild.textContent.substr(0,loc);
	     if(ename == pnodename){
		 str="Found a match";
		 //so currentedge is what we are looking for
		 var cpath = currentedge.getElementsByTagName('path')[0];
		 var width=cpath.getAttribute("stroke-width");
		 if(width == 9){
		     cpath.setAttribute("stroke-width",2);
		 }
		 if(width == 2){
		     cpath.setAttribute("stroke-width",9);
		 }
		 if(width == 12){
		     cpath.setAttribute("stroke-width",9);
		 }
	     }
	 }
         //str=childs.length;
         //nname = childs.item(0).nodeName;
         //nvalue = childs.item(0).nodeValue;
	 loc = curedgetitle.indexOf("->");
         //alert("len= " + parent.getElementsByTagName('title').length + "data= " + ttc + " num nodes= " + allnodes.length + " num edges= " + alledges.length +" curedge title = " + curedgetitle +" loc= " + loc +" pnodename= " + pnodename + " str= " + str);
      }
      else{
          alert("No child nodes");
      }
      }
      else{
	  var width = evt.target.getAttribute("stroke-width");
	  if(width > 2){
	      evt.target.setAttribute("stroke-width",2);
	  }
	  else{
	      evt.target.setAttribute("stroke-width",9);
	  }
      }
      //stopTyping(evt);
    }

  ]]>
    </script>|;

##$events=q|onclick="changeEdge(evt,7)" onclick="getparent(evt)" stroke-width="2" |;
$events=q|onclick="getparent(evt)" stroke-width="2" onmouseover="changeEdge2(evt,1)" onmouseout="changeEdge2(evt,2)" onkeypress="xkeypress(evt)"|;
$graphevent=q| onclick="initTyping(evt)"|;
$hilitenode=q| onmouseover="initTypingN(evt)" onmouseout="stopTypingN(evt)"|;
while(<STDIN>){
    if(/<path /){
	#$str=q|style="fill:none;stroke:#ff2e00;stroke-width:3;"|
	s/(<path .*)\/>/$1 $events \/>/;
	s/(style=\"fill:none;stroke:.*;)stroke-width:\d;\"/$1\"/;
    }
    if(/<\/svg>/){
	s/(<\/svg>)/$script\n$1/;
    }
    if(/<g id\=\"graph0\" .*>/){
	s/(<g id\=\"graph0\" .*)>/$1 $graphevent >/;
    }
    if(/<ellipse /){
	s/(<ellipse .*)\/>/$1 $hilitenode\/>/;
    }
    print $_;
    
}
