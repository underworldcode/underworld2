import xml.sax.handler

def readXML( xml_text ):
	# We don't want sax to attempt to load the DTD if listed... remove the line if there.	
	start = xml_text.find( '<!DOCTYPE' )
	if start >= 0:
		end = xml_text[start+1:].find( '>' )
		if end >= 0:
			xml_text = xml_text[:start] + xml_text[start+end+2:]
		# don't bother about else... if DOCTYPE is broken, the whole XML is broken and let it fail below

        p = xml.sax.make_parser()
        h = Handler()
        p.setContentHandler( h )
	p.feed( xml_text )
	p.close()

	return h.getDict()


class Handler( xml.sax.handler.ContentHandler ):
    def __init__( self ):
        self.inStGermainData = 0
        self.stack = []
 
    def startElement( self, name, attributes ):
        if name == "StGermainData":
            if self.inStGermainData == 0:
                self.inStGermainData = 1
                self.stack.append( {} )
                self._top()["content"] = {}
                self._top()["tag"] = "StGermainData"
            else:
                raise RuntimeError( 'Invalid meta xml - parsing tag StGermainData' )
        elif name == "param":
            if self.inStGermainData:
                self.stack.append( {} )
                self._top()["content"] = ""
                self._top()["tag"] = "param"
		try:
	                self._top()["key"] = attributes["name"]
		except KeyError:
			pass
            else:
                raise RuntimeError( 'Invalid meta xml - parsing tag param' )
        elif name == "list":
            if self.inStGermainData:
                self.stack.append( {} )
                self._top()["content"] = []
                self._top()["tag"] = "list"
		try:
	                self._top()["key"] = attributes["name"]
		except KeyError:
			pass
            else:
                raise RuntimeError( 'Invalid meta xml - parsing tag list' )
        elif name == "struct":
            if self.inStGermainData:
                self.stack.append( {} )
                self._top()["content"] = {}
                self._top()["tag"] = "struct"
		try:
	                self._top()["key"] = attributes["name"]
		except KeyError:
			pass
            else:
                raise RuntimeError( 'Invalid meta xml - parsing tag struct' )

    def characters( self, data ):
        if self._top()["tag"] == "param":
            self._top()["content"] += data
 
    def endElement( self, name ):
        if name == "StGermainData":
            self.inStGermainData = 0
        elif name == "param":
            if self._top()["tag"] == "param":
                self._pop()
            else:
                raise RuntimeError( 'Invalid meta xml - parsing tag /param' )
        elif name == "list":
            if self._top()["tag"] == "list":
                self._pop()
            else:
                raise RuntimeError( 'Invalid meta xml - parsing tag /param' )
        elif name == "struct":
            if self._top()["tag"] == "struct":
               self._pop()
            else:
                raise RuntimeError( 'Invalid meta xml - parsing tag /param' )


    def getDict( self ):
        if len( self.stack ):
            return self.stack[0]["content"]


    def _top( self ):
        return self.stack[ len( self.stack ) - 1 ]


    def _peek( self ):
        return self.stack[ len( self.stack ) - 2 ]


    def _pop( self ):
        value = self._top()["content"]
        if self._peek()["tag"] == "StGermainData" or self._peek()["tag"] == "struct":
            try:
                key = self._top()["key"]
                self._peek()["content"][key] = value
            except KeyError:
                RuntimeError( 'Invalid meta xml - tag missing name attribute' )
        elif self._peek()["tag"] == "list":
            self._peek()["content"].append( value )
        else:
            raise RuntimeError( 'Invalid meta xml - parsing tag /param' )
        self.stack.pop()

