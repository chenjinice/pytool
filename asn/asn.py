from asn.asn2020 import Asn2020


_asn_list     = [Asn2020]

asnAll      = {}


for asn in _asn_list:
    asnAll[asn.__name__.lower()]  = asn
