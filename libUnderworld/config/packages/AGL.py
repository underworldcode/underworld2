from config import Package

class AGL(Package):

    def gen_envs(self, loc):
        env = self.env.Clone()
        self.headers = ['AGL/agl.h', 'gl.h']
        env.AppendUnique(FRAMEWORKS=['AGL'])
        env.AppendUnique(FRAMEWORKS=['OpenGL'])
        env.AppendUnique(CPPPATH=['/System/Library/Frameworks/OpenGL.framework/Headers'])
        yield env
