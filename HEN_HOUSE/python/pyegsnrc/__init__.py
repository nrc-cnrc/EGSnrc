import importlib

from ._egsnrc import *

print("Hello World!")

THIS = importlib.import_module(__name__)
IMPORTABLES = dir(THIS)

print("Everything that is accessible from Python:")
print(IMPORTABLES)


# TODO: Need to make functions like ausgab be user definable from within
# python.
