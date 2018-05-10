from distutils.core import setup, Extension

module1 = Extension('uilib',
                    sources = ['src/wrapper.c'],
                    include_dirs = ['../library/src'],
                    libraries = ['mylib'],
                    library_dirs = ['../bin/library'])

setup (name = 'uilib',
       version = '1.0',
       description = 'Crude low-res UI based on SDL and tslib',
       ext_modules = [module1])
