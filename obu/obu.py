
from obu.obu_sy import ObuSy

_obu_list = [ObuSy]

obuAll = {}

for obu in _obu_list:
    obuAll[obu.__name__.lower()] = obu
