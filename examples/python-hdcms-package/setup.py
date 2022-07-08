from setuptools import setup, Extension
# import numpy
import os
import sys

# deal with macros
macros = []
if os.getenv('REPEL'):
    macros.append(('REPEL',1))
if sys.platform == 'darwin':
    macros.append(('MAC', 1))

module = Extension("hdcms", # name that you `import` in python
                   define_macros=macros,
                   sources=["python.c"])

setup(
    name="hdcms", # this name doesn't matter, it's the "package name" which is how you interact with pip
    version="1.0.0",
    description="This is wraps the C functions in hdcms",
    author="Jason Eveleth",
    author_email="jj11@eveleth.com",
    ext_modules=[module], 
)

