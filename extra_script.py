from SCons.Script import *

env = DefaultEnvironment()

# Builds the webapp using Bower and Gulp

env.Append(BUILDERS = {
    'GulpCompress': Builder(action = 'gulp compress'),
    'NpmInstall': Builder(action = 'npm --production=false install'),
})

node_modules = Dir('node_modules')

env.NpmInstall(node_modules, [])
gulp_compress = env.GulpCompress(Dir('data/web'), [Dir('src/web'), node_modules])

env.AlwaysBuild(node_modules)
