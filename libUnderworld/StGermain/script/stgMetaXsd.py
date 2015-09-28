from xml.dom.minidom import Document

def createXML( metaDict ):
	doc = Document()

	XMLNS_NS = u'stgermainmeta.xsd'
	XSI_NS = u'stgermainmeta.xsd'

	eMeta = doc.createElement( u'meta' )
	eMeta.setAttribute( u'xmlns', 'urn:stgermainmeta-schema' )
	eMeta.setAttributeNS( XMLNS_NS, u'xmlns:dc', u'http://purl.org/dc/elements/1.1/' )
	eMeta.setAttributeNS( XMLNS_NS, u'xmlns:xsi', u'http://www.w3.org/2001/XMLSchema-instance' )
	eMeta.setAttributeNS( XMLNS_NS, u'xmlns:xsd', u'http://www.w3.org/2001/XMLSchema' )
	eMeta.setAttributeNS( XSI_NS, u'xsi:schemaLocation', u'http://purl.org/dc/elements/1.1/ dc.xsd http://www.w3.org/2001/XMLSchema XMLSchema.xsd urn:stgermainmeta-schema stgermainmeta.xsd' )
	doc.appendChild( eMeta )

	eInfo = doc.createElement( u'info' )
	eTitle = doc.createElement( u'dc:title' )
	eCreator = doc.createElement( u'dc:creator' )
	ePublisher = doc.createElement( u'dc:publisher' )
	eRights = doc.createElement( u'dc:rights' )
	eSource = doc.createElement( u'dc:source' )
	eSubject = doc.createElement( u'dc:subject' )
	eDescription = doc.createElement( u'dc:description' )
	eInfo.appendChild( eTitle )
	eInfo.appendChild( eCreator )
	eInfo.appendChild( ePublisher )
	eInfo.appendChild( eRights )
	eInfo.appendChild( eSource )
	eInfo.appendChild( eSubject )
	eInfo.appendChild( eDescription )
	eMeta.appendChild( eInfo )

	eCode = doc.createElement( u'code' )
	eExAnn = doc.createElement( u'xsd:annotation' )
	eExDoc = doc.createElement( u'xsd:documentation' )
	eExApp = doc.createElement( u'xsd:appinfo' )
	eInherits = doc.createElement( u'inherits' )
	eCode.appendChild( eExAnn )
	eExAnn.appendChild( eExDoc )
	eExAnn.appendChild( eExApp )
	eCode.appendChild( eInherits )
	eMeta.appendChild( eCode )

	eImplements = doc.createElement( u'implements' )
	eReference = doc.createElement( u'reference' )
	eEquation = doc.createElement( u'equation' )
	eImplements.appendChild( eReference )
	eImplements.appendChild( eEquation )
	eMeta.appendChild( eImplements )

	eParameters = doc.createElement( u'parameters' )
	eMeta.appendChild( eParameters )

	eAssociations = doc.createElement( u'associations' )
	eMeta.appendChild( eAssociations )


	# Content...
	# Info (all required i.e. let is except if dictionary entry not there) ...
	eTitleTxt = doc.createTextNode( metaDict["info"]["title"] )
	eTitle.appendChild( eTitleTxt )
	eCreatorTxt = doc.createTextNode( metaDict["info"]["creator"] )
	eCreator.appendChild( eCreatorTxt )
	ePublisherTxt = doc.createTextNode( metaDict["info"]["publisher"] )
	ePublisher.appendChild( ePublisherTxt )
	eRightsTxt = doc.createTextNode( metaDict["info"]["rights"] )
	eRights.appendChild( eRightsTxt )
	eSourceTxt = doc.createTextNode( metaDict["info"]["source"] )
	eSource.appendChild( eSourceTxt )
	eSubjectTxt = doc.createTextNode( metaDict["info"]["subject"] )
	eSubject.appendChild( eSubjectTxt )
	eDescriptionTxt = doc.createCDATASection( metaDict["info"]["description"] )
	eDescription.appendChild( eDescriptionTxt )

	# Code (Not all required i.e. catch except if dictionary entry not there) ...
	try:
		eExDocTxt = doc.createCDATASection( metaDict["code"]["example-documentation"] )
		eExDoc.appendChild( eExDocTxt )
	except KeyError:
		pass
	try:
		eExAppTxt = doc.createCDATASection( metaDict["code"]["example-code"] )
		eExApp.appendChild( eExAppTxt )
	except KeyError:
		pass
	try:
		eInheritsTxt = doc.createTextNode( metaDict["code"]["inherits"] )
		eInherits.appendChild( eInheritsTxt )
	except KeyError:
		pass

	# Implements (Not all required i.e. catch except if dictionary entry not there) ...
	try:
		eReferenceTxt = doc.createTextNode( metaDict["implements"]["reference"] )
		eReference.appendChild( eReferenceTxt )
	except KeyError:
		pass
	try:
		eEquationTxt = doc.createCDATASection( metaDict["implements"]["equation"] )
		eEquation.appendChild( eEquationTxt )
	except KeyError:
		pass

	# Parameters (as this is an XML XSD type, the rules on requirment are determined by it) ...
	for param in metaDict["parameters"]:
		eParam = doc.createElement( u'xsd:element' )
		eParam.setAttribute( u'name', param["name"] )
		eParam.setAttribute( u'type', param["type"] )
		try:
			eParam.setAttribute( u'default', param["default"] )
		except KeyError:
			pass
		try:
			eParamAnn = doc.createElement( u'xsd:annotation' )
			eParamDoc = doc.createElement( u'xsd:documentation' )
			eParamDocTxt = doc.createCDATASection( param["documentation"] )
			eParamDoc.appendChild( eParamDocTxt )
			eParamAnn.appendChild( eParamDoc )
			eParam.appendChild( eParamAnn )
		except KeyError:
			pass
		eParameters.appendChild( eParam )

	# Associations (as this is an XML XSD type, the rules on requirment are determined by it)...
	for assoc in metaDict["associations"]:
		eAssoc = doc.createElement( u'xsd:element' )
		eAssoc.setAttribute( u'name', assoc["name"] )
		eAssoc.setAttribute( u'type', assoc["type"] )
		try:
			eAssoc.setAttribute( u'nillable', assoc["nillable"] )
		except KeyError:
			pass
		try:
			eAssocAnn = doc.createElement( u'xsd:annotation' )
			eAssocDoc = doc.createElement( u'xsd:documentation' )
			eAssocDocTxt = doc.createCDATASection( assoc["documentation"] )
			eAssocDoc.appendChild( eAssocDocTxt )
			eAssocAnn.appendChild( eAssocDoc )
			eAssoc.appendChild( eAssocAnn )
		except KeyError:
			pass
		eAssociations.appendChild( eAssoc )

	return doc

