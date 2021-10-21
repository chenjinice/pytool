
from asn.asn2020 import *


__all__     = ['asnAll']

_asn_list   = [Asn2020]

asnAll      = {}

for asn in _asn_list:
    asnAll[asn.__name__.lower()]  = asn
