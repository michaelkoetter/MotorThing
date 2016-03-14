from SCons.Script import *

env = DefaultEnvironment()

# Builds the webapp using Bower and Gulp

env.Append(BUILDERS = {
    'GulpCompress': Builder(action = 'gulp compress'),
    'NpmInstall': Builder(action = 'npm install'),
    'BowerInstall': Builder(action = 'bower install')
})

node_modules = Dir('node_modules')
bower_components = Dir('bower_components')

env.NpmInstall(node_modules, [])
env.BowerInstall(bower_components, node_modules)
gulp_compress = env.GulpCompress(Dir('data/web'), [Dir('src/web'), bower_components])

env.AlwaysBuild(node_modules)
env.AlwaysBuild(bower_components)
