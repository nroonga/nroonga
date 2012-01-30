import os
import sys
from os.path import exists
from shutil import copy2 as copy, rmtree
import multiprocessing

import Options
import Utils
import urllib

TARGET = 'nroonga_bindings'
TARGET_FILE = '%s.node' % TARGET
built = 'build/Release/%s' % TARGET_FILE
dest = 'lib/%s' % TARGET_FILE

INTERNAL_GROONGA_VERSION = '1.3.0'
INTERNAL_GROONGA = 'groonga-%s' % INTERNAL_GROONGA_VERSION
GROONGA_SOURCE_TREE = 'deps/%s' % INTERNAL_GROONGA

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure_internal_groonga(conf):
  tar_gz = INTERNAL_GROONGA + '.tar.gz'
  groonga_url = 'http://packages.groonga.org/source/groonga/' + tar_gz

  if not os.path.exists('deps'):
    os.mkdir('deps')
  os.chdir('deps')

  if not os.path.exists(tar_gz):
    Utils.pprint('GREEN', 'Fetching %s' % groonga_url)
    f = open(tar_gz, 'w')
    remote = urllib.urlopen(groonga_url)
    block_size = 8192
    while True:
      buffer = remote.read(block_size)
      if not buffer:
        break
      f.write(buffer)
    remote.close()
    f.close()
  else:
    Utils.pprint('GREEN', 'Using existing %s' % tar_gz)

  if not os.path.exists(INTERNAL_GROONGA):
    Utils.pprint('GREEN', 'Extracting %s' % tar_gz)
    os.system('tar xzf %s' % tar_gz)
  else:
    Utils.pprint('GREEN', 'Using existing %s source tree' % INTERNAL_GROONGA)

  os.chdir(INTERNAL_GROONGA)
  groonga_target = os.path.join(conf.blddir, conf.envname, 'groonga')
  if not os.path.exists('config.status'):
    Utils.pprint('GREEN', 'Configuring groonga')
    os.system('./configure --prefix=%s --disable-document' % groonga_target)
  else:
    Utils.pprint('GREEN', 'Groonga already configured')

  conf.env.append_value("CPPPATH_GROONGA", ['../deps/%s' % INTERNAL_GROONGA])
  conf.env.append_value("GROONGA_TARGET", groonga_target)

  os.chdir('../../')

def build_internal_groonga(bld):
  if Options.commands['clean'] and bld.env['GROONGA_TARGET']:
    groonga_target = bld.env['GROONGA_TARGET'][0]
    if os.path.exists(groonga_target):
      rmtree(groonga_target)
  elif '../deps' in bld.env['CPPPATH_GROONGA'][0]:
    if not os.path.exists(GROONGA_SOURCE_TREE):
      Utils.pprint('RED','Please re-run node-waf configure')
      sys.exit()

    os.chdir(GROONGA_SOURCE_TREE)
    Utils.pprint('GREEN', 'Building groonga')
    os.system('make -j %d' % multiprocessing.cpu_count())
    os.system('make install')
    os.chdir('../../')

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")

  Utils.pprint('GREEN', 'Trying to use system groonga ...')
  if conf.check_cfg(package='groonga', args='--cflags --libs'):
    Utils.pprint('GREEN', 'Groonga is installed. Using it ...')
  else:
    Utils.pprint('GREEN', 'Groonga not installed. Trying to build groonga internally ...')
    configure_internal_groonga(conf)
    os.environ['PKG_CONFIG_PATH'] = 'build/Release/groonga/lib/pkgconfig'
    Utils.pprint('RED', conf.check_cfg(package='groonga', args='--cflags --libs'))

def build(bld):
  build_internal_groonga(bld)

  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.cxxflags = ["-g", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"]
  obj.target = "nroonga_bindings"
  obj.source = "src/nroonga.cc"
  obj.uselib = ["GROONGA"]

def shutdown():
  if Options.commands['clean']:
    if exists(TARGET_FILE):
      os.unlink(TARGET_FILE)
    if exists(dest):
      os.unlink(dest)
  else:
    if exists(built):
      copy(built, dest)
