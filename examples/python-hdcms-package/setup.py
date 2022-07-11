from setuptools import setup, Extension
import numpy as np
import os

module = Extension("hdcms", # name that you `import` in python
                   include_dirs=[os.getcwd(), np.get_include()],
                   sources=["python.c"])

setup(
    name="hdcms", # this name doesn't matter, it's the "package name" which is how you interact with pip
    version="1.0.0",
    description="This is wraps the C functions in hdcms",
    author="Jason Eveleth",
    author_email="jj11@eveleth.com",
    ext_modules=[module], 
)

