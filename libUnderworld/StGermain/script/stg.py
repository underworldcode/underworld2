import os, sys
from SCons.Script import *

def generate(env, **kw):

    sys.path.insert(0, os.path.dirname(__file__))

    import stgDtd
    import stgMetaXsd
    import convert

    def loadXML( filename ):
        xml_file = file( filename )
        xml_lines = xml_file.readlines()
        xml_text = ""
        for l in xml_lines:
            xml_text += str(l)

        try:
            dtdDict = stgDtd.readXML( xml_text )
        except:
            print 'Failed to parse as a StGermain DTD'
            raise
        try:
            return convert.dtdDict2metaXsdDict( dtdDict )
        except:
            print 'Failed to convert information from a StGermain Meta DTD to a StGermain Meta XSD'
            raise

    def stgMeta(target, source, env):
        dir = os.path.dirname(target[0].abspath)
        if not os.path.exists(dir):
            os.makedirs(dir)

        meta = file(target[0].abspath, 'w')
        xsdDict = loadXML(source[0].abspath)
        # TODO: Add here things like real location (rep & path), test results & sv/hg diff status

        meta.write( convert.metaXsdDict2stgCodeHeader() + '\n' )
        meta.write( convert.metaXsdDict2stgStrings( xsdDict ) + '\n' )
        meta.write( convert.metaXsdDict2stgDictionaryCode( xsdDict ) )

        meta.close()

    def stgSharedMeta(env, target, source, **kw):
        metas = env.stgMeta(source, **kw)
        return env.SharedObject(target, metas, **kw)

    def stgStaticMeta(env, target, source, **kw):
        metas = env.stgMeta(source, **kw)
        return env.StaticObject(target, metas, **kw)

    def gen_meta_suffix(env, source):
        return "-meta.c"

    env['BUILDERS']['stgMeta'] = Builder(action=stgMeta, single_source=True,
                                         suffix=gen_meta_suffix, src_suffix='meta')
    env['BUILDERS']['stgSharedMeta'] = stgSharedMeta
    env['BUILDERS']['stgStaticMeta'] = stgStaticMeta

def exists(env):
    return True
