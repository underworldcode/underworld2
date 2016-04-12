from config import Package

class CGL(Package):

    def gen_envs(self, loc):
        env = self.env.Clone()
        self.headers = ['OpenGL/OpenGL.h', 'OpenGL/gl.h']
        env.AppendUnique(FRAMEWORKS=['OpenGL', 'Cocoa', 'Quartz'])
        env.AppendUnique(CPPPATH=['/System/Library/Frameworks/OpenGL.framework/Headers'])
        yield env
